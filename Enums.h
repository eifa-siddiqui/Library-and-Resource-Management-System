#pragma once

// Defines the book's condition on return - drives fine calculation
enum class BookCondition {
    PERFECT,
    MINOR_DAMAGE,
    HEAVY_DAMAGE,
    LOST
};

// Status assigned to different members for additional perks
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
