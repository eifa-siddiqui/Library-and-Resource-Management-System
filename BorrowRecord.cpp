#include "BorrowRecord.h"

BorrowRecord::BorrowRecord()
    : memberID(""), resource(nullptr),
      issueDate(0), dueDate(0),
      isReturned(false), returnDate(0)
{}

BorrowRecord::BorrowRecord(const std::string& memberID,
                           LibraryResource* resource,
                           time_t issueDate,
                           time_t dueDate)
    : memberID(memberID), resource(resource),
      issueDate(issueDate), dueDate(dueDate),
      isReturned(false), returnDate(0)
{}

int BorrowRecord::calculateLateDays(time_t currentDate) const {
    // If returned on time, no late days
    if (isReturned && returnDate <= dueDate) return 0;

    // Use returnDate for returned records, currentDate for active borrows
    time_t checkDate = isReturned ? returnDate : currentDate;

    if (checkDate <= dueDate) return 0;

    return static_cast<int>((checkDate - dueDate) / 86400);
}

// --- Getters ---
std::string      BorrowRecord::getMemberID()   const { return memberID;   }
LibraryResource* BorrowRecord::getResource()   const { return resource;   }
time_t           BorrowRecord::getIssueDate()  const { return issueDate;  }
time_t           BorrowRecord::getDueDate()    const { return dueDate;    }
bool             BorrowRecord::getIsReturned() const { return isReturned; }
time_t           BorrowRecord::getReturnDate() const { return returnDate; }

// --- Setters ---
void BorrowRecord::setMemberID(const std::string& id) { memberID   = id; }
void BorrowRecord::setResource(LibraryResource* r)    { resource   = r;  }
void BorrowRecord::setIssueDate(time_t t)             { issueDate  = t;  }
void BorrowRecord::setDueDate(time_t t)               { dueDate    = t;  }
void BorrowRecord::setIsReturned(bool val)            { isReturned = val;}
void BorrowRecord::setReturnDate(time_t t)            { returnDate = t;  }
