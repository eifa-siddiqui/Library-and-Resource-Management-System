#pragma once

// Condition of a resource when returned — drives fine calculation
enum class BookCondition {
    PERFECT,
    MINOR_DAMAGE,
    HEAVY_DAMAGE,
    LOST
};

// Membership tier — plain enum (not enum class) so rev_mem.cpp's
// Member class can initialize with status(STANDARD) without a scope prefix.
enum MembershipStatus {
    STANDARD,
    PREMIUM,
    ELITE
};

// Broad classification of every library resource
enum class BookCategory {
    FICTION,
    NON_FICTION
};
