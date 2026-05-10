#include "Admin.h"
#include "Member.h"
#include "BorrowRecord.h"
#include "LibraryResource.h"
#include "FineUtility.h"
#include <iostream>
#include <ctime>
using namespace std;

#define RST "\033[0m"
#define RED "\033[31m"
#define GRN "\033[32m"
#define YLW "\033[33m"
#define BLU "\033[34m"
#define CYN "\033[36m"

// Formats a time_t as YYYY-MM-DD
static string formatDate(time_t t) {
    if (t == 0) return "N/A";
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return buf;
}

// Converts MembershipStatus enum to display string
static string statusStr(MembershipStatus s) {
    if (s == PREMIUM) return "PREMIUM";
    if (s == ELITE)   return "ELITE";
    return "STANDARD";
}

// ─── Constructor ──────────────────────────────────────────────────────────────

Admin::Admin(const string& id, const string& firstName, const string& lastName,
             const string& password, const string& email, const string& address)
    : User(id, firstName, lastName, password, email, address)
{}

// ─── Overrides ────────────────────────────────────────────────────────────────

void Admin::displayDashboard() const {
    cout << "===== Admin Dashboard =====\n";
    cout << "Logged in as: " << firstName << " " << lastName << " (ID: " << id << ")\n";
}

string Admin::getRole() const { return "ADMIN"; }

// ─── processReturn ────────────────────────────────────────────────────────────

void Admin::processReturn(Member& member, BorrowRecord* record,
                          BookCondition condition, time_t currentDate) {
    int    lateDays = record->calculateLateDays(currentDate);
    double fine     = FineUtility::calculateFine(lateDays, condition);

    if (fine > 0) {
        generateFineNotice(record, fine);
        member.deductBalance(fine);
    }

    // Mark the resource available again
    record->getResource()->setAvailable(true);

    // If someone has reserved this book, lock it to them so nobody else can issue it first
    if (record->getResource()->hasReservation()) {
        Member* next = record->getResource()->nextReservation();
        record->getResource()->setHoldFor(next);
        cout << GRN "RESERVATION NOTICE: \"" RST
             << record->getResource()->getTitle()
             << GRN "\" is now on hold.\n" RST
             << CYN "Held for: " RST << next->getFirstName()
             << " " << next->getLastName()
             << " (" << next->getEmail() << ") - please notify them.\n";
    }
}

void Admin::generateFineNotice(BorrowRecord* record, double fine) {
    cout << "---------- FINE NOTICE ----------\n";
    cout << "Resource : " << record->getResource()->getTitle() << "\n";
    cout << "Member ID: " << record->getMemberID() << "\n";
    cout << "Fine     : PKR " << fine << "\n";
    cout << "---------------------------------\n";
}

// ─── Reports ─────────────────────────────────────────────────────────────────

void Admin::generateMemberReport(const vector<Member*>& members,
                                  const string& /*fileName*/) {
    time_t now = time(nullptr);
    cout << YLW "\n=== MEMBER REPORT ===\n" RST;
    cout << BLU "Generated: " RST << ctime(&now);

    if (members.empty()) { cout << YLW "No members registered.\n" RST; return; }

    for (Member* m : members) {
        cout << YLW "\n--------------------\n" RST;
        cout << CYN "Member  : " RST << m->getFirstName() << " " << m->getLastName()
             << " (" << m->getId() << ")\n";
        cout << CYN "Status  : " RST << statusStr(m->getStatus()) << "\n";
        cout << CYN "Balance : " RST << "PKR " << m->getBalance() << "\n";

        const vector<BorrowRecord*>& borrows = m->getBorrowedBooks();
        if (borrows.empty()) { cout << BLU "  No borrow history.\n" RST; continue; }

        cout << BLU "  Active Borrows:\n" RST;
        bool hasActive = false;
        for (BorrowRecord* r : borrows) {
            if (!r->getIsReturned()) {
                cout << "    - \"" << r->getResource()->getTitle() << "\""
                     << " (" << r->getResource()->getIsbn() << ")"
                     << " | Issued: " << formatDate(r->getIssueDate())
                     << " | Due: "    << formatDate(r->getDueDate()) << "\n";
                hasActive = true;
            }
        }
        if (!hasActive) cout << "    None.\n";

        cout << BLU "  Returned:\n" RST;
        bool hasReturned = false;
        for (BorrowRecord* r : borrows) {
            if (r->getIsReturned()) {
                cout << "    - \"" << r->getResource()->getTitle() << "\""
                     << " (" << r->getResource()->getIsbn() << ")"
                     << " | Returned: " << formatDate(r->getReturnDate()) << "\n";
                hasReturned = true;
            }
        }
        if (!hasReturned) cout << "    None.\n";
    }
    cout << YLW "\n====================\n" RST;
}

void Admin::generateResourceReport(const vector<LibraryResource*>& resources,
                                    const vector<Member*>& members,
                                    const string& /*fileName*/) {
    time_t now = time(nullptr);
    cout << YLW "\n=== RESOURCE REPORT ===\n" RST;
    cout << BLU "Generated: " RST << ctime(&now);

    cout << YLW "\nCurrently Issued:\n" RST;
    bool anyIssued = false;
    for (LibraryResource* r : resources) {
        if (!r->isAvailable()) {
            string borrowerID = "Unknown";
            time_t dueDate    = 0;
            for (Member* m : members) {
                BorrowRecord* rec = m->findActiveBorrow(r->getIsbn());
                if (rec) { borrowerID = m->getId(); dueDate = rec->getDueDate(); break; }
            }
            int overdueDays = (dueDate > 0 && now > dueDate)
                              ? static_cast<int>((now - dueDate) / 86400) : 0;

            cout << "  - \"" << r->getTitle() << "\" (" << r->getIsbn() << ")"
                 << " | Issued to: " << borrowerID
                 << " | Due: " << formatDate(dueDate);
            if (overdueDays > 0)
                cout << RED " | OVERDUE by " << overdueDays << " day(s)" RST;
            cout << "\n";
            anyIssued = true;
        }
    }
    if (!anyIssued) cout << "  None.\n";

    int availCount = 0;
    for (LibraryResource* r : resources) if (r->isAvailable()) availCount++;
    cout << YLW "\nAvailable Resources: " RST << availCount << "\n";
    for (LibraryResource* r : resources) {
        if (r->isAvailable())
            cout << "  - \"" << r->getTitle() << "\""
                 << " (" << r->getIsbn() << ") - " << r->getType() << "\n";
    }
    cout << YLW "\n=======================\n" RST;
}
