#include "User.h"
using namespace std;

User::User(const string& id,
           const string& firstName,
           const string& lastName,
           const string& password,
           const string& email,
           const string& address)
    : id(id), firstName(firstName), lastName(lastName),
      password(password), email(email), address(address)
{}

bool User::checkPassword(const string& input) const {
    return password == input; //if equal then true
}

string User::getId()        const { return id;        }
string User::getFirstName() const { return firstName; }
string User::getLastName()  const { return lastName;  }
string User::getPassword()  const { return password;  }
string User::getEmail()     const { return email;     }
string User::getAddress()   const { return address;   }

void User::setPassword(const string& p) { password = p; }
void User::setEmail(const string& e)    { email    = e; }
void User::setAddress(const string& a)  { address  = a; }
