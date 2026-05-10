#include "Review.h"
using namespace std;

Review::Review(const string& reviewID,
               const string& memberID,
               const string& resourceISBN,
               int rating,
               const string& comment)
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

string Review::getReviewID()     const { return reviewID;     }
string Review::getMemberID()     const { return memberID;     }
string Review::getResourceISBN() const { return resourceISBN; }
int         Review::getRating()       const { return rating;       }
string Review::getComment()      const { return comment;      }
time_t      Review::getCreatedAt()    const { return createdAt;    }
bool        Review::isApproved()      const { return approved;     }
