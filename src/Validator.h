#pragma once
#include "StrUtils.h"

// Validator is the ONLY class allowed to contain input validation logic.
// All functions are static so you can call them without creating an object.
// Example: Validator::isValidDate("10-05-2026")
class Validator
{
public:
    // checks if a date string is in DD-MM-YYYY format
    // day must be 01-31, month must be 01-12, year must be current year or later
    static bool isValidDate(const char *date);

    // checks if a time slot is one of the 8 allowed slots
    // allowed: 09:00 10:00 11:00 12:00 13:00 14:00 15:00 16:00
    static bool isValidTimeSlot(const char *slot);

    // checks if a contact number is exactly 11 digits and all numeric
    static bool isValidContact(const char *contact);

    // checks if a password is at least 6 characters long
    static bool isValidPassword(const char *pw);

    // checks if a string represents a positive number greater than 0
    // accepts decimals like "1500.50"
    static bool isPositiveFloat(const char *s);

    // checks if a string represents a positive whole number greater than 0
    static bool isPositiveInt(const char *s);

    // checks if a menu choice number falls within the allowed range [lo, hi]
    static bool isValidMenuChoice(int choice, int lo, int hi);

    // checks if a string is a valid positive ID number
    static bool isValidId(const char *s);

    // fills buf with today's date in DD-MM-YYYY format
    // buf must be at least 11 characters long
    static void getTodayStr(char *buf);

    // fills buf with the current date and time as a timestamp
    // format: DD-MM-YYYY HH:MM:SS
    // buf must be at least 20 characters long
    static void getTimestampStr(char *buf);

    // compares two dates in DD-MM-YYYY format
    // returns -1 if a is earlier, 0 if equal, 1 if a is later
    static int compareDates(const char *a, const char *b);

    // calculates how many days are between two DD-MM-YYYY dates
    // result is (b - a), so positive means b is after a
    static double daysBetween(const char *a, const char *b);
};
