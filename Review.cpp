#include "Review.h"

Review::Review(const std::string& reviewID,
               const std::string& memberID,
               const std::string& resourceISBN,
               int rating,
               const std::string& comment)
    : reviewID(reviewID),
      memberID(memberID),
      resourceISBN(resourceISBN),
      rating(rating),
      comment(comment),
      createdAt(time(nullptr)),
      approved(false)
{}

void Review::approve() { approved = true;  }
void Review::reject()  { approved = false; }

std::string Review::getReviewID()     const { return reviewID;     }
std::string Review::getMemberID()     const { return memberID;     }
std::string Review::getResourceISBN() const { return resourceISBN; }
int         Review::getRating()       const { return rating;       }
std::string Review::getComment()      const { return comment;      }
time_t      Review::getCreatedAt()    const { return createdAt;    }
bool        Review::isApproved()      const { return approved;     }
