#pragma once
#include <exception>
#include <string>

// Thrown when a member tries to borrow more than 2 resources on the same day.
// Inherits from std::exception so it can be caught generically if needed.
class BorrowLimitException : public std::exception {
private:
    std::string message;

public:
    BorrowLimitException(const std::string& msg) : message(msg) {}

    // Override std::exception::what() to return our custom message
    const char* what() const noexcept override {
        return message.c_str();
    }
};
