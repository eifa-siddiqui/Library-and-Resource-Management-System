#pragma once
#include <string>
#include <ctime>

// Represents a member's review of a library resource.
// Admin can approve or reject reviews before they are visible.
class Review {
private:
    std::string reviewID;
    std::string memberID;
    std::string resourceISBN;
    int         rating;       // 1–5
    std::string comment;
    time_t      createdAt;
    bool        approved;     // false by default until admin approves

public:
    Review(const std::string& reviewID,
           const std::string& memberID,
           const std::string& resourceISBN,
           int rating,
           const std::string& comment);

    // Admin actions
    void approve();
    void reject();

    // Getters
    std::string getReviewID()      const;
    std::string getMemberID()      const;
    std::string getResourceISBN()  const;
    int         getRating()        const;
    std::string getComment()       const;
    time_t      getCreatedAt()     const;
    bool        isApproved()       const;
};
