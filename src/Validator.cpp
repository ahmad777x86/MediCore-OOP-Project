#include "Validator.h"
#include <iostream>

// checks if a date string follows the DD-MM-YYYY format and has valid values
bool Validator::isValidDate(const char *date)
{
    // date must be exactly 10 characters: DD-MM-YYYY
    if (myStrlen(date) != 10)
    {
        return false;
    }

    // the dashes must be in position 2 and 5
    if (date[2] != '-' || date[5] != '-')
    {
        return false;
    }

    // extract day, month, and year into separate small arrays
    char dd[3];
    dd[0] = date[0];
    dd[1] = date[1];
    dd[2] = '\0';

    char mm[3];
    mm[0] = date[3];
    mm[1] = date[4];
    mm[2] = '\0';

    char yyyy[5];
    yyyy[0] = date[6];
    yyyy[1] = date[7];
    yyyy[2] = date[8];
    yyyy[3] = date[9];
    yyyy[4] = '\0';

    // all parts must contain digits only
    if (!isAllDigits(dd) || !isAllDigits(mm) || !isAllDigits(yyyy))
    {
        return false;
    }

    int day = strToInt(dd);
    int month = strToInt(mm);
    int year = strToInt(yyyy);

    // check day and month are in valid ranges
    if (day < 1 || day > 31)
    {
        return false;
    }
    if (month < 1 || month > 12)
    {
        return false;
    }

    // year must be 2026 or later (current year for Spring 2026 semester)
    int currentYear = 2026;

    if (year < currentYear)
    {
        return false;
    }

    return true;
}

// checks if the given slot matches one of the 8 allowed time slots
bool Validator::isValidTimeSlot(const char *slot)
{
    // these are the only valid time slots for any doctor
    const char *validSlots[8];
    validSlots[0] = "09:00";
    validSlots[1] = "10:00";
    validSlots[2] = "11:00";
    validSlots[3] = "12:00";
    validSlots[4] = "13:00";
    validSlots[5] = "14:00";
    validSlots[6] = "15:00";
    validSlots[7] = "16:00";

    for (int i = 0; i < 8; i++)
    {
        if (myStrcmp(slot, validSlots[i]) == 0)
        {
            return true;
        }
    }

    return false;
}

// contact number must be exactly 11 characters and all digits
bool Validator::isValidContact(const char *contact)
{
    if (myStrlen(contact) != 11)
    {
        return false;
    }

    return isAllDigits(contact);
}

// password must be at least 6 characters
bool Validator::isValidPassword(const char *pw)
{
    if (myStrlen(pw) >= 6)
    {
        return true;
    }
    return false;
}

// checks if a string is a valid positive decimal number (greater than 0)
bool Validator::isPositiveFloat(const char *s)
{
    // empty string is invalid
    if (s == NULL || s[0] == '\0')
    {
        return false;
    }

    bool hasDot = false;
    int digitCount = 0;
    int i = 0;

    while (s[i] != '\0')
    {
        if (s[i] == '.')
        {
            // only one decimal point is allowed
            if (hasDot == true)
            {
                return false;
            }
            hasDot = true;
        }
        else if (s[i] >= '0' && s[i] <= '9')
        {
            digitCount++;
        }
        else
        {
            // any other character is invalid
            return false;
        }
        i++;
    }

    // must have at least one digit
    if (digitCount == 0)
    {
        return false;
    }

    // the number itself must be greater than zero
    if (strToFloat(s) <= 0.0f)
    {
        return false;
    }

    return true;
}

// checks if a string is a valid positive whole number (greater than 0)
bool Validator::isPositiveInt(const char *s)
{
    if (!isAllDigits(s))
    {
        return false;
    }

    if (strToInt(s) > 0)
    {
        return true;
    }

    return false;
}

// checks if a menu choice is within the allowed range
bool Validator::isValidMenuChoice(int choice, int lo, int hi)
{
    if (choice >= lo && choice <= hi)
    {
        return true;
    }
    return false;
}

// checks if a string is a valid ID (positive whole number)
bool Validator::isValidId(const char *s)
{
    if (!isAllDigits(s))
    {
        return false;
    }

    if (strToInt(s) > 0)
    {
        return true;
    }

    return false;
}

// fills buf with today's date in DD-MM-YYYY format
void Validator::getTodayStr(char *buf)
{
    time_t currentTime = time(NULL);
    struct tm *currentTm = localtime(&currentTime);

    int day = currentTm->tm_mday;
    int month = currentTm->tm_mon + 1;    // tm_mon is 0-based so we add 1
    int year = currentTm->tm_year + 1900; // tm_year is years since 1900

    // write DD
    buf[0] = '0' + (day / 10);
    buf[1] = '0' + (day % 10);
    buf[2] = '-';

    // write MM
    buf[3] = '0' + (month / 10);
    buf[4] = '0' + (month % 10);
    buf[5] = '-';

    // write YYYY
    buf[6] = '0' + (year / 1000);
    buf[7] = '0' + (year / 100) % 10;
    buf[8] = '0' + (year / 10) % 10;
    buf[9] = '0' + (year % 10);

    buf[10] = '\0';
}

// fills buf with a full timestamp: DD-MM-YYYY HH:MM:SS
void Validator::getTimestampStr(char *buf)
{
    time_t currentTime = time(NULL);
    struct tm *currentTm = localtime(&currentTime);

    // first write the date part into buf
    getTodayStr(buf);

    // then add a space and the time
    buf[10] = ' ';

    int hour = currentTm->tm_hour;
    int minute = currentTm->tm_min;
    int second = currentTm->tm_sec;

    buf[11] = '0' + (hour / 10);
    buf[12] = '0' + (hour % 10);
    buf[13] = ':';

    buf[14] = '0' + (minute / 10);
    buf[15] = '0' + (minute % 10);
    buf[16] = ':';

    buf[17] = '0' + (second / 10);
    buf[18] = '0' + (second % 10);

    buf[19] = '\0';
}

// compares two dates in DD-MM-YYYY format
// returns -1 if a comes before b, 0 if equal, 1 if a comes after b
int Validator::compareDates(const char *a, const char *b)
{
    // pull out year, month, day from each date string
    char yearA[5];
    yearA[0] = a[6];
    yearA[1] = a[7];
    yearA[2] = a[8];
    yearA[3] = a[9];
    yearA[4] = '\0';

    char monthA[3];
    monthA[0] = a[3];
    monthA[1] = a[4];
    monthA[2] = '\0';

    char dayA[3];
    dayA[0] = a[0];
    dayA[1] = a[1];
    dayA[2] = '\0';

    char yearB[5];
    yearB[0] = b[6];
    yearB[1] = b[7];
    yearB[2] = b[8];
    yearB[3] = b[9];
    yearB[4] = '\0';

    char monthB[3];
    monthB[0] = b[3];
    monthB[1] = b[4];
    monthB[2] = '\0';

    char dayB[3];
    dayB[0] = b[0];
    dayB[1] = b[1];
    dayB[2] = '\0';

    int ay = strToInt(yearA);
    int am = strToInt(monthA);
    int ad = strToInt(dayA);

    int by = strToInt(yearB);
    int bm = strToInt(monthB);
    int bd = strToInt(dayB);

    // compare year first, then month, then day
    if (ay != by)
    {
        if (ay < by)
            return -1;
        else
            return 1;
    }

    if (am != bm)
    {
        if (am < bm)
            return -1;
        else
            return 1;
    }

    if (ad != bd)
    {
        if (ad < bd)
            return -1;
        else
            return 1;
    }

    return 0;
}

// calculates the number of days between two DD-MM-YYYY dates
// a positive result means b is after a
double Validator::daysBetween(const char *a, const char *b)
{
    // extract parts of date a
    char yearA[5];
    yearA[0] = a[6];
    yearA[1] = a[7];
    yearA[2] = a[8];
    yearA[3] = a[9];
    yearA[4] = '\0';

    char monthA[3];
    monthA[0] = a[3];
    monthA[1] = a[4];
    monthA[2] = '\0';

    char dayA[3];
    dayA[0] = a[0];
    dayA[1] = a[1];
    dayA[2] = '\0';

    // extract parts of date b
    char yearB[5];
    yearB[0] = b[6];
    yearB[1] = b[7];
    yearB[2] = b[8];
    yearB[3] = b[9];
    yearB[4] = '\0';

    char monthB[3];
    monthB[0] = b[3];
    monthB[1] = b[4];
    monthB[2] = '\0';

    char dayB[3];
    dayB[0] = b[0];
    dayB[1] = b[1];
    dayB[2] = '\0';

    // fill in two tm structs so we can use mktime to convert to timestamps
    struct tm tmA;
    tmA.tm_year = strToInt(yearA) - 1900;
    tmA.tm_mon = strToInt(monthA) - 1;
    tmA.tm_mday = strToInt(dayA);
    tmA.tm_hour = 0;
    tmA.tm_min = 0;
    tmA.tm_sec = 0;
    tmA.tm_isdst = -1;

    struct tm tmB;
    tmB.tm_year = strToInt(yearB) - 1900;
    tmB.tm_mon = strToInt(monthB) - 1;
    tmB.tm_mday = strToInt(dayB);
    tmB.tm_hour = 0;
    tmB.tm_min = 0;
    tmB.tm_sec = 0;
    tmB.tm_isdst = -1;

    time_t timeA = mktime(&tmA);
    time_t timeB = mktime(&tmB);

    // difftime gives the difference in seconds, divide by 86400 to get days
    double differenceInSeconds = difftime(timeB, timeA);
    double differenceInDays = differenceInSeconds / 86400.0;

    return differenceInDays;
}