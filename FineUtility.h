#pragma once
#include "Enums.h"

// Static utility class — never instantiated.
// Centralises all fine calculation logic in one place.
class FineUtility {
public:
    // Returns total fine in PKR based on overdue days and resource condition.
    // Base rate: 10 PKR per late day.
    // Damage surcharges: MINOR +500, HEAVY +2000, LOST +5000
    static double calculateFine(int lateDays, BookCondition condition);

private:
    FineUtility() = delete; // Prevents instantiation
};
