#pragma once
#include <string>
#include <ctime>

// Forward declaration — BorrowRecord observes a LibraryResource but does not own it.
// The full definition lives in LibraryResource.h (included only in .cpp files that need it).
class LibraryResource;

// Tracks a single borrow transaction between a Member and a LibraryResource.
// ERD corrections applied: added memberID, isReturned, returnDate.
class BorrowRecord {
private:
    std::string      memberID;    // which member borrowed this resource
    LibraryResource* resource;    // observing pointer — BookDatabase owns the resource
    time_t           issueDate;
    time_t           dueDate;
    bool             isReturned;  // false until Admin processes the return
    time_t           returnDate;  // 0 if not yet returned

public:
    // Default constructor — resource starts as nullptr
    BorrowRecord();

    // Full constructor used when creating a new borrow transaction
    BorrowRecord(const std::string& memberID,
                 LibraryResource* resource,
                 time_t issueDate,
                 time_t dueDate);

    // Returns number of days past the due date.
    // Returns 0 if returned on time or not yet overdue.
    int calculateLateDays(time_t currentDate) const;

    // Getters
    std::string      getMemberID()    const;
    LibraryResource* getResource()    const;
    time_t           getIssueDate()   const;
    time_t           getDueDate()     const;
    bool             getIsReturned()  const;
    time_t           getReturnDate()  const;

    // Setters (used by Member::returnBook and file persistence)
    void setMemberID(const std::string& id);
    void setResource(LibraryResource* r);
    void setIssueDate(time_t t);
    void setDueDate(time_t t);
    void setIsReturned(bool val);
    void setReturnDate(time_t t);
};
