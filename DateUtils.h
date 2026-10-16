#ifndef DATE_UTILS_H
#define DATE_UTILS_H

#include <string>

class DateUtils {
public:
    // Checks if year is a leap year according to Gregorian calendar rules
    static bool isLeapYear(int year);

    // Returns the number of days in a given month of a given year (1-12)
    static int getDaysInMonth(int year, int month);

    // Validates that the date is in exact YYYY-MM-DD format and represents a real calendar date
    static bool isValidDate(const std::string& dateStr);

    // Compares two valid YYYY-MM-DD dates lexicographically
    // Returns:
    //  -1 if date1 < date2
    //   0 if date1 == date2
    //   1 if date1 > date2
    static int compareDates(const std::string& date1, const std::string& date2);

    // Documented Business Rule:
    // A batch expires at 00:00:00 on its expiry date.
    // Therefore, if expiryDate <= currentDate, the batch is EXPIRED and cannot be sold.
    static bool isExpired(const std::string& expiryDate, const std::string& currentDate);
};

#endif // DATE_UTILS_H
