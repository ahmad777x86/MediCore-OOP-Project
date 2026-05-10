#include "StrUtils.h"

// converts a single character to lowercase without using any library
char toLowerChar(char c)
{

    if (c >= 'A' && c <= 'Z')
    {
        c = c + 32;
    }
    return c;
}

// counts how many characters are in the string before the null terminator
int myStrlen(const char *s)
{
    int count = 0;
    while (s[count] != '\0')
    {
        count++;
    }
    return count;
}

// copies every character from src into dst including the null terminator
void myStrcpy(char *dst, const char *src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// copies up to n-1 characters from src into dst and always adds a null terminator
void myStrncpy(char *dst, const char *src, int n)
{
    int i = 0;
    while (i < n - 1 && src[i] != '\0')
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// joins src onto the end of dst
void myStrcat(char *dst, const char *src)
{
    int dstLen = myStrlen(dst);
    int i = 0;
    while (src[i] != '\0')
    {
        dst[dstLen + i] = src[i];
        i++;
    }
    dst[dstLen + i] = '\0';
}

// compares two strings character by character
// returns 0 if they are identical, non-zero otherwise
int myStrcmp(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return a[i] - b[i];
        }
        i++;
    }
    // if both ended at the same time they are equal
    return a[i] - b[i];
}

// compares two strings without caring about uppercase or lowercase letters
bool myStrEqCI(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (toLowerChar(a[i]) != toLowerChar(b[i]))
        {
            return false;
        }
        i++;
    }
    // both must have ended at the same position to be equal
    return a[i] == '\0' && b[i] == '\0';
}

// reads one token (piece of text) from a CSV line into dst
// stops when it sees the delimiter, newline, or end of string
// returns a pointer to the position just after the delimiter
const char *parseToken(const char *src, char *dst, int maxLen, char delim)
{
    int i = 0;
    while (src[i] != '\0' && src[i] != delim && src[i] != '\n' && src[i] != '\r')
    {
        if (i < maxLen - 1)
        {
            dst[i] = src[i];
        }
        i++;
    }
    dst[i] = '\0';

    // skip past the delimiter so the next call starts from the right place
    if (src[i] == delim)
    {
        return src + i + 1;
    }
    return src + i;
}

// converts a whole number into its text form and stores it in buf
// for example: 42 becomes "42"
void intToStr(int val, char *buf)
{
    // special case for zero
    if (val == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    bool isNegative = false;
    if (val < 0)
    {
        isNegative = true;
        val = -val;
    }

    // build digits in reverse order
    char temp[32];
    int i = 0;
    while (val > 0)
    {
        temp[i] = '0' + (val % 10);
        val = val / 10;
        i++;
    }

    if (isNegative)
    {
        temp[i] = '-';
        i++;
    }

    // now reverse temp into buf
    int j = 0;
    int k = i - 1;
    while (k >= 0)
    {
        buf[j] = temp[k];
        j++;
        k--;
    }
    buf[j] = '\0';
}

// converts a decimal number into text with 2 decimal places
// for example: 1500.5 becomes "1500.50"
void floatToStr(float val, char *buf)
{
    int pos = 0;

    if (val < 0)
    {
        buf[pos] = '-';
        pos++;
        val = -val;
    }

    // separate the whole part and the fractional part
    int wholePart = (int)val;
    int fracPart = (int)((val - wholePart) * 100 + 0.5f);

    // write the whole part
    char temp[32];
    intToStr(wholePart, temp);
    int t = 0;
    while (temp[t] != '\0')
    {
        buf[pos] = temp[t];
        pos++;
        t++;
    }

    // write the decimal point
    buf[pos] = '.';
    pos++;

    // write fractional part with leading zero if needed
    if (fracPart < 10)
    {
        buf[pos] = '0';
        pos++;
    }
    char fracTemp[8];
    intToStr(fracPart, fracTemp);
    int f = 0;
    while (fracTemp[f] != '\0')
    {
        buf[pos] = fracTemp[f];
        pos++;
        f++;
    }

    buf[pos] = '\0';
}

// reads a whole number from a string of digits
// for example: "42" becomes 42
int strToInt(const char *s)
{
    int result = 0;
    int sign = 1;

    if (s[0] == '-')
    {
        sign = -1;
        s++;
    }

    int i = 0;
    while (s[i] >= '0' && s[i] <= '9')
    {
        result = result * 10 + (s[i] - '0');
        i++;
    }

    return result * sign;
}

// reads a decimal number from a string
// for example: "1500.75" becomes 1500.75
float strToFloat(const char *s)
{
    float result = 0;
    int sign = 1;

    if (s[0] == '-')
    {
        sign = -1;
        s++;
    }

    // read digits before the decimal point
    int i = 0;
    while (s[i] >= '0' && s[i] <= '9')
    {
        result = result * 10 + (s[i] - '0');
        i++;
    }

    // read digits after the decimal point if there is one
    if (s[i] == '.')
    {
        i++;
        float place = 0.1f;
        while (s[i] >= '0' && s[i] <= '9')
        {
            result = result + (s[i] - '0') * place;
            place = place * 0.1f;
            i++;
        }
    }

    return result * sign;
}

// checks if every character in the string is a digit between '0' and '9'
bool isAllDigits(const char *s)
{
    // an empty string is not a valid number
    if (s[0] == '\0')
    {
        return false;
    }

    int i = 0;
    while (s[i] != '\0')
    {
        if (s[i] < '0' || s[i] > '9')
        {
            return false;
        }
        i++;
    }
    return true;
}