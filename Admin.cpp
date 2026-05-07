#include "Admin.h"
#include "Member.h"
#include "BorrowRecord.h"
#include "LibraryResource.h"
#include "FineUtility.h"
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

// Formats a time_t as YYYY-MM-DD
static string formatDate(time_t t) {
    if (t == 0) return "N/A";
    char buf[16];
    struct tm* tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_info);
    return string(buf);
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

    // Notify the next person in the reservation queue
    if (record->getResource()->hasReservation()) {
        Member* next = record->getResource()->nextReservation();
        cout << "RESERVATION NOTICE: \""
             << record->getResource()->getTitle()
             << "\" is now available.\n"
             << "Next in queue: " << next->getFirstName()
             << " " << next->getLastName() << " — please notify them.\n";
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
                                  const string& fileName) {
    ofstream f(fileName);
    if (!f.is_open()) { cout << "Cannot open " << fileName << "\n"; return; }

    time_t now = time(nullptr);
    f << "=== MEMBER REPORT ===\n";
    f << "Generated: " << ctime(&now);   // ctime adds its own newline
    f << "\n";

    for (Member* m : members) {
        f << "Member: " << m->getFirstName() << " " << m->getLastName()
          << " (" << m->getId() << ")"
          << " | Status: " << statusStr(m->getStatus())
          << " | Balance: PKR " << m->getBalance() << "\n";

        const vector<BorrowRecord*>& borrows = m->getBorrowedBooks();
        if (borrows.empty()) {
            f << "  No borrow history.\n\n";
            continue;
        }

        f << "  Active Borrows:\n";
        bool hasActive = false;
        for (BorrowRecord* r : borrows) {
            if (!r->getIsReturned()) {
                f << "    - \"" << r->getResource()->getTitle() << "\""
                  << " (" << r->getResource()->getIsbn() << ")"
                  << " | Issued: " << formatDate(r->getIssueDate())
                  << " | Due: "    << formatDate(r->getDueDate()) << "\n";
                hasActive = true;
            }
        }
        if (!hasActive) f << "    None.\n";

        f << "  Returned:\n";
        bool hasReturned = false;
        for (BorrowRecord* r : borrows) {
            if (r->getIsReturned()) {
                f << "    - \"" << r->getResource()->getTitle() << "\""
                  << " (" << r->getResource()->getIsbn() << ")"
                  << " | Returned: " << formatDate(r->getReturnDate()) << "\n";
                hasReturned = true;
            }
        }
        if (!hasReturned) f << "    None.\n";
        f << "\n";
    }

    cout << "Member report written to " << fileName << "\n";
}

void Admin::generateResourceReport(const vector<LibraryResource*>& resources,
                                    const vector<Member*>& members,
                                    const string& fileName) {
    ofstream f(fileName);
    if (!f.is_open()) { cout << "Cannot open " << fileName << "\n"; return; }

    time_t now = time(nullptr);
    f << "=== RESOURCE REPORT ===\n";
    f << "Generated: " << ctime(&now);
    f << "\n";

    // Currently issued section — cross-reference members to find borrower
    f << "Currently Issued:\n";
    bool anyIssued = false;
    for (LibraryResource* r : resources) {
        if (!r->isAvailable()) {
            // Find which member has this resource
            string borrowerID = "Unknown";
            time_t dueDate    = 0;
            for (Member* m : members) {
                BorrowRecord* rec = m->findActiveBorrow(r->getIsbn());
                if (rec) { borrowerID = m->getId(); dueDate = rec->getDueDate(); break; }
            }
            int overdueDays = (dueDate > 0 && now > dueDate)
                              ? static_cast<int>((now - dueDate) / 86400) : 0;

            f << "  - \"" << r->getTitle() << "\" (" << r->getIsbn() << ")"
              << " | Issued to: " << borrowerID
              << " | Due: " << formatDate(dueDate);
            if (overdueDays > 0)
                f << " | OVERDUE by " << overdueDays << " day(s)";
            f << "\n";
            anyIssued = true;
        }
    }
    if (!anyIssued) f << "  None.\n";
    f << "\n";

    // Available section
    int availCount = 0;
    for (LibraryResource* r : resources) if (r->isAvailable()) availCount++;
    f << "Available Resources: " << availCount << "\n";
    for (LibraryResource* r : resources) {
        if (r->isAvailable())
            f << "  - \"" << r->getTitle() << "\""
              << " (" << r->getIsbn() << ") — " << r->getType() << "\n";
    }

    cout << "Resource report written to " << fileName << "\n";
}
