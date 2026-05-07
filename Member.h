#pragma once
#include <vector>
#include "User.h"
#include "Enums.h"
#include "BorrowRecord.h"
#include "BorrowLimitException.h"

class LibraryResource; // forward declaration — full header in Member.cpp

// Member extends User with balance management and borrow operations.
// Member OWNS all BorrowRecord* in borrowedBooks (deletes them in destructor).
class Member : public User {
private:
    double                      balance;
    MembershipStatus            status;
    std::vector<BorrowRecord*>  borrowedBooks; // owned

public:
    Member(const std::string& id,
           const std::string& firstName,
           const std::string& lastName,
           const std::string& password,
           const std::string& email,
           const std::string& address);

    ~Member(); // deletes all BorrowRecord* in borrowedBooks

    // Overrides — required by abstract User
    void        displayDashboard() const override;
    std::string getRole()          const override; // returns "MEMBER"

    // ---------- Balance ----------
    void   depositAmount(double amount);  // rejects if amount <= 0
    double getBalance()                   const;
    void   deductBalance(double amount);  // clamps to 0 if fine > balance
    void   setBalance(double amount);     // used when loading from file
    MembershipStatus getStatus()          const;
    void             setStatus(MembershipStatus s);

    // ---------- Borrow management ----------
    void issueBook(LibraryResource* r);           // throws BorrowLimitException if limit reached
    void returnBook(const std::string& isbn);     // marks record; Admin finalises the return
    void reserveBook(LibraryResource* r);         // prints message if available; queues if not
    int  countTodaysBorrows()               const; // counts borrows issued today
    void viewBorrowHistory()               const;

    BorrowRecord* findActiveBorrow(const std::string& isbn);  // only non-returned records
    BorrowRecord* findBorrowRecord(const std::string& isbn);  // any record for this isbn

    // Used by file persistence (PR 5) to restore saved records without re-running issueBook logic
    void addBorrowRecord(BorrowRecord* r);

    const std::vector<BorrowRecord*>& getBorrowedBooks() const;
};
