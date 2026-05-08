#include "User.h"

User::User(const std::string& id,
           const std::string& firstName,
           const std::string& lastName,
           const std::string& password,
           const std::string& email,
           const std::string& address)
    : id(id), firstName(firstName), lastName(lastName),
      password(password), email(email), address(address)
{}

bool User::checkPassword(const std::string& input) const {
    return password == input; //if equal then true 
}

std::string User::getId()        const { return id;        }
std::string User::getFirstName() const { return firstName; }
std::string User::getLastName()  const { return lastName;  }
std::string User::getPassword()  const { return password;  }
std::string User::getEmail()     const { return email;     }
std::string User::getAddress()   const { return address;   }

void User::setPassword(const std::string& p) { password = p; }
void User::setEmail(const std::string& e)    { email    = e; }
void User::setAddress(const std::string& a)  { address  = a; }
