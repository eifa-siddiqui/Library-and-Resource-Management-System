#include "FineUtility.h"

double FineUtility::calculateFine(int lateDays, BookCondition condition) {
    double fine = lateDays * 10.0; // Base: 10 PKR per day

    switch (condition) {
        case BookCondition::MINOR_DAMAGE: fine += 500;  break;
        case BookCondition::HEAVY_DAMAGE: fine += 2000; break;
        case BookCondition::LOST:         fine += 5000; break;
        case BookCondition::PERFECT:                    break;
    }

    return fine;
}
