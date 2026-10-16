#include "DateUtils.h"
#include <cctype>
#include <stdexcept>

bool DateUtils::isLeapYear(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    return (year % 4 == 0);
}

int DateUtils::getDaysInMonth(int year, int month) {
    switch (month) {
        case 1:  return 31;
        case 2:  return isLeapYear(year) ? 29 : 28;
        case 3:  return 31;
        case 4:  return 30;
        case 5:  return 31;
        case 6:  return 30;
        case 7:  return 31;
        case 8:  return 31;
        case 9:  return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: return 0;
    }
}

bool DateUtils::isValidDate(const std::string& dateStr) {
    // Must be exactly 10 characters: YYYY-MM-DD
    if (dateStr.length() != 10) return false;

    // Check positions 4 and 7 are '-'
    if (dateStr[4] != '-' || dateStr[7] != '-') return false;

    // Check all other positions are digits
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) {
            return false;
        }
    }

    try {
        int year = std::stoi(dateStr.substr(0, 4));
        int month = std::stoi(dateStr.substr(5, 2));
        int day = std::stoi(dateStr.substr(8, 2));

        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;

        int maxDays = getDaysInMonth(year, month);
        if (day < 1 || day > maxDays) return false;

        return true;
    } catch (...) {
        return false;
    }
}

int DateUtils::compareDates(const std::string& date1, const std::string& date2) {
    if (date1 < date2) return -1;
    if (date1 > date2) return 1;
    return 0;
}

bool DateUtils::isExpired(const std::string& expiryDate, const std::string& currentDate) {
    // If either date is malformed, reject as expired for safety
    if (!isValidDate(expiryDate) || !isValidDate(currentDate)) {
        return true;
    }
    // Consistent rule: a batch is expired on or after its expiry date (expiryDate <= currentDate)
    return expiryDate <= currentDate;
}
