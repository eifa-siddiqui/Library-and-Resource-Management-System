#pragma once
#include <string>

// Abstract base class for all system users.
// Protected attributes are accessible to Member and Admin directly,
// which keeps displayDashboard() implementations simple in subclasses.
class User {
protected:
    std::string id;
    std::string firstName;
    std::string lastName;
    std::string password;
    std::string email;
    std::string address;

public:
    User(const std::string& id,
         const std::string& firstName,
         const std::string& lastName,
         const std::string& password,
         const std::string& email,
         const std::string& address);

    virtual ~User() {}

    // Pure virtual — forces every subclass to show its own dashboard and declare its role.
    // This makes User abstract (cannot be instantiated directly).
    virtual void        displayDashboard() const = 0;
    virtual std::string getRole()          const = 0;

    // Returns true if input matches the stored password
    bool checkPassword(const std::string& input) const;
//& is used to refer the original string otherwise extra copy will be made
    // Getters
    std::string getId()        const;
    std::string getFirstName() const;
    std::string getLastName()  const;
    std::string getPassword()  const; // needed for file persistence
    std::string getEmail()     const;
    std::string getAddress()   const;

    // Setters (used by Admin when editing member info)
    void setPassword(const std::string& p);
    void setEmail(const std::string& e);
    void setAddress(const std::string& a);
    // & so no copy will be made and const so that the paramatere would not get change 
};
