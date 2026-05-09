#include "Member.h"
#include "LibraryResource.h"
#include <iostream>
#include <ctime>
#include <stdexcept>
using namespace std;

// Returns true if the given timestamp falls on today's calendar date
//it is helper funct and canbe used within this cpp file only
static bool isToday(time_t t) {
    time_t now       = time(nullptr);
    tm*    todayTm   = localtime(&now);
    tm*    checkTm   = localtime(&t);
    return todayTm->tm_year == checkTm->tm_year &&
           todayTm->tm_mon  == checkTm->tm_mon  &&
           todayTm->tm_mday == checkTm->tm_mday;
}

// ─── Constructor / Destructor ─────────────────────────────────────────────────

Member::Member(const string& id, const string& firstName, const string& lastName,
               const string& password, const string& email, const string& address)
    : User(id, firstName, lastName, password, email, address),
      balance(0.0), pending_fine(0.0), status(STANDARD)
{}

Member::~Member() {
    for (BorrowRecord* r : borrowedBooks) delete r;
}

// ─── Overrides ────────────────────────────────────────────────────────────────

// Incorporates the dashboard display pattern from rev_mem.cpp
void Member::displayDashboard() const {
    cout << "\033[33m===== Member Dashboard =====\n\033[0m";
    cout << "\033[36mUser ID : \033[0m" << id << "\n";
    cout << "\033[36mName    : \033[0m" << firstName << " " << lastName << "\n";
    cout << "\033[36mEmail   : \033[0m" << email << "\n";
    cout << "\033[36mBalance : PKR \033[0m" << balance << "\n";
    int active = 0;
    for (BorrowRecord* r : borrowedBooks)
        if (!r->getIsReturned()) active++;
    cout << "\033[35m Active borrows: \033[0m" << active << "\n";
}

string Member::getRole() const { return "MEMBER"; }

// ─── Balance ──────────────────────────────────────────────────────────────────

// depositAmount — validation logic taken directly from rev_mem.cpp
void Member::depositAmount(double amount) {
    if (amount <= 0)
        throw invalid_argument("Deposit amount must be greater than zero.");
    balance += amount;
    cout << "PKR " << amount << " deposited. New balance: PKR " << balance << endl;
}

double Member::getBalance()          const { return balance; }
MembershipStatus Member::getStatus() const { return status; }
void Member::setStatus(MembershipStatus s) { status = s; }
void Member::setBalance(double amount)     { balance = amount; }

void Member::deductBalance(double amount) {
    if (amount >= balance) {
        cout << "Fine (PKR " << amount << ") exceeds balance. Balance set to 0.\n";
        balance = 0;
    } else {
        balance -= amount;
        cout << "\033[35mPKR " << amount << "\033[35m deducted. Remaining balance: PKR \033[0m" << balance << "\n";
    }
}

// ─── Borrow management ────────────────────────────────────────────────────────

// issueBook — core logic from rev_mem.cpp, adapted:
//   • Checks daily borrow limit (throws BorrowLimitException if >= 2 today)
//   • Uses 14-day borrow period instead of 7
//   • Passes member ID to BorrowRecord (required for file persistence in PR 5)
bool Member::hasOverdueBooks() const {
    time_t now = time(nullptr);
    for (BorrowRecord* r : borrowedBooks)
        if (!r->getIsReturned() && r->getDueDate() < now)
            return true;
    return false;
}

void Member::issueBook(LibraryResource* r) {
    if (hasOverdueBooks())
        throw runtime_error("\033[31mYou have overdue book(s). Return them before borrowing more.\033[0m");

    if (countTodaysBorrows() >= 2)
        throw BorrowLimitException("\033[31mDaily borrow limit of 2 reached.\033[0m");

    if (!r->isAvailable())
        throw runtime_error("\033[31m\"\033[0m" + r->getTitle() + "\033[31m\" is not available.\033[0m");

    time_t now     = time(nullptr);
    time_t dueDate = now + 14 * 24 * 60 * 60; // 14-day borrow period

    BorrowRecord* record = new BorrowRecord(id, r, now, dueDate);
    borrowedBooks.push_back(record);
    r->setAvailable(false);
    cout << "\033[32m\"" << r->getTitle() << "\" issued successfully. Due in 14 days.\n\033[0m";

    // Auto-upgrade status based on total borrows:
    // 5+ borrows -> PREMIUM, 15+ borrows -> ELITE
    int total = (int)borrowedBooks.size();
    MembershipStatus newStatus = status;
    if      (total >= 15) newStatus = ELITE;
    else if (total >= 5)  newStatus = PREMIUM;

    if (newStatus != status) {
        status = newStatus;
        string label = (status == ELITE) ? "ELITE" : "PREMIUM";
        cout << "\033[33mCongratulations! Your membership has been upgraded to " << label << ".\n\033[0m";
    }
}

// returnBook — marks the record only; Admin::processReturn sets isAvailable = true
// (Admin checks condition first before making the book available again)
void Member::returnBook(const string& isbn) {
    BorrowRecord* rec = findActiveBorrow(isbn);
    if (rec == nullptr) {
        cout << "\033[31mNo active borrow found for ISBN \033[0m" << isbn << ".\n";
        return;
    }
    rec->setIsReturned(true);
    rec->setReturnDate(time(nullptr));
    cout << "\033[32mReturn recorded for \"\033[0m" << rec->getResource()->getTitle()
         << "\".\033[34m Admin will complete the process.\n\033[0m";
}

// reserveBook — availability check from rev_mem.cpp; adapted to use addToReservation
void Member::reserveBook(LibraryResource* r) {
    if (hasOverdueBooks()) {
        cout << "\033[31mYou have overdue book(s). Return them before making reservations.\033[0m\n";
        return;
    }
    if (r->isAvailable()) {
        cout << "\033[32m\"" << r->getTitle() << "\033[32m\" is available — just issue it.\n\033[0m";
    } else {
        r->addToReservation(this);
        cout << "\033[32m\"" << r->getTitle() << "\033[32m\" is currently unavailable.\n\033[0m";
        cout << "\033[32mYou have been added to the reservation queue\033[0m"
             << "\033[33m (position: " << r->getReservationCount() << ").\n\033[0m";
        cout << "\033[33mYou will be notified when it becomes available.\n\033[0m";
    }
}

int Member::countTodaysBorrows() const {
    int count = 0;
    for (BorrowRecord* r : borrowedBooks)
        if (isToday(r->getIssueDate())) count++;
    return count;
}

void Member::viewBorrowHistory() const {
    if (borrowedBooks.empty()) {
        cout << "\033[35mNo borrow history.\n\033[0m";
        return;
    }
    for (BorrowRecord* r : borrowedBooks) {
        string status = r->getIsReturned() ? "[Returned]" : "[Active]  ";
        cout << "  " << status
             << " \"" << r->getResource()->getTitle() << "\""
             << " (ISBN: " << r->getResource()->getIsbn() << ")\n";
    }
}

BorrowRecord* Member::findActiveBorrow(const string& isbn) {
    for (BorrowRecord* r : borrowedBooks)
        if (!r->getIsReturned() && r->getResource()->getIsbn() == isbn)
            return r;
    return nullptr;
}

// Used by admin processReturn — finds any record for an isbn, returned or not
BorrowRecord* Member::findBorrowRecord(const string& isbn) {
    for (BorrowRecord* r : borrowedBooks)
        if (r->getResource()->getIsbn() == isbn) return r;
    return nullptr;
}

void Member::addBorrowRecord(BorrowRecord* r) {
    borrowedBooks.push_back(r);
}

const vector<BorrowRecord*>& Member::getBorrowedBooks() const {
    return borrowedBooks;
}
