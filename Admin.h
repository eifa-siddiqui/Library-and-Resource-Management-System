#pragma once
#include <vector>
#include "User.h"
#include "Enums.h"

// Forward declarations — full headers included only in Admin.cpp
class Member;
class BorrowRecord;
class LibraryResource;

// Admin extends User with library management capabilities.
// Admin has no extra attribute  — its authority comes from its methods.
// Collection management (adding/removing users and resources) is handled by
// LibrarySystem, which owns the vectors. Admin only does what requires
// admin-level authority: processing returns, generating reports.
class Admin : public User {
public:
    Admin(const std::string& id,
          const std::string& firstName,
          const std::string& lastName,
          const std::string& password,
          const std::string& email,
          const std::string& address);

    // Overrides — required by abstract User
    void        displayDashboard() const override;
    std::string getRole()          const override; // returns "ADMIN"

    // ---------- Return processing ----------
    // Calculates fine, deducts balance, marks resource available, notifies reservers
    void processReturn(Member& member, BorrowRecord* record,
                       BookCondition condition, time_t currentDate);

    void generateFineNotice(BorrowRecord* record, double fine);

    // ---------- Reports ----------
    void generateMemberReport(const std::vector<Member*>& members,
                              const std::string& fileName);
    // members needed to cross-reference who currently holds each resource
    void generateResourceReport(const std::vector<LibraryResource*>& resources,
                                const std::vector<Member*>& members,
                                const std::string& fileName);
};
