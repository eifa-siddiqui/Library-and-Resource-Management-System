#include "Member.h"
#include "LibraryResource.h"
#include <iostream>
#include <ctime>
#include <stdexcept>
using namespace std;

// Returns true if the given timestamp falls on today's calendar date
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
      balance(0.0), status(STANDARD)
{}

Member::~Member() {
    for (BorrowRecord* r : borrowedBooks) delete r;
}

// ─── Overrides ────────────────────────────────────────────────────────────────

// Incorporates the dashboard display pattern from baap_code.cpp
void Member::displayDashboard() const {
    cout << "===== Member Dashboard =====\n";
    cout << "User ID : " << id << "\n";
    cout << "Name    : " << firstName << " " << lastName << "\n";
    cout << "Email   : " << email << "\n";
    cout << "Balance : PKR " << balance << "\n";
    int active = 0;
    for (BorrowRecord* r : borrowedBooks)
        if (!r->getIsReturned()) active++;
    cout << "Active borrows: " << active << "\n";
}

string Member::getRole() const { return "MEMBER"; }

// ─── Balance ──────────────────────────────────────────────────────────────────

// depositAmount — validation logic taken directly from baap_code.cpp
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
        cout << "PKR " << amount << " deducted. Remaining balance: PKR " << balance << "\n";
    }
}

// ─── Borrow management ────────────────────────────────────────────────────────

// issueBook — core logic from baap_code.cpp, adapted:
//   • Checks daily borrow limit (throws BorrowLimitException if >= 2 today)
//   • Uses 14-day borrow period instead of 7
//   • Passes member ID to BorrowRecord (required for file persistence in PR 5)
void Member::issueBook(LibraryResource* r) {
    if (countTodaysBorrows() >= 2)
        throw BorrowLimitException("Daily borrow limit of 2 reached.");

    if (!r->isAvailable())
        throw runtime_error("\"" + r->getTitle() + "\" is not available.");

    time_t now     = time(nullptr);
    time_t dueDate = now + 14 * 24 * 60 * 60; // 14-day borrow period

    BorrowRecord* record = new BorrowRecord(id, r, now, dueDate);
    borrowedBooks.push_back(record);
    r->setAvailable(false);
    cout << "\"" << r->getTitle() << "\" issued successfully. Due in 14 days.\n";
}

// returnBook — marks the record only; Admin::processReturn sets isAvailable = true
// (Admin checks condition first before making the book available again)
void Member::returnBook(const string& isbn) {
    BorrowRecord* rec = findActiveBorrow(isbn);
    if (rec == nullptr) {
        cout << "No active borrow found for ISBN " << isbn << ".\n";
        return;
    }
    rec->setIsReturned(true);
    rec->setReturnDate(time(nullptr));
    cout << "Return recorded for \"" << rec->getResource()->getTitle()
         << "\". Admin will complete the process.\n";
}

// reserveBook — availability check from baap_code.cpp; adapted to use addToReservation
void Member::reserveBook(LibraryResource* r) {
    if (r->isAvailable()) {
        cout << "\"" << r->getTitle() << "\" is available — just issue it.\n";
    } else {
        r->addToReservation(this);
        cout << "\"" << r->getTitle() << "\" is currently unavailable.\n";
        cout << "You have been added to the reservation queue"
             << " (position: " << r->getReservationCount() << ").\n";
        cout << "You will be notified when it becomes available.\n";
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
        cout << "No borrow history.\n";
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
