#pragma once
int myStrlen(const char *s);

// copies src into dst (like strcpy)
void myStrcpy(char *dst, const char *src);

// copies at most n-1 characters from src into dst, always null-terminates
void myStrncpy(char *dst, const char *src, int n);

// adds src to the end of dst (like strcat)
void myStrcat(char *dst, const char *src);

// compares two strings, returns 0 if equal (like strcmp)
int myStrcmp(const char *a, const char *b);

// compares two strings ignoring uppercase/lowercase
// returns true if they are the same word regardless of case
bool myStrEqCI(const char *a, const char *b);

// reads one comma-separated token from src into dst
// returns a pointer to the character right after the comma
const char *parseToken(const char *src, char *dst, int maxLen, char delim);

const char *parseToken(const char *src, char &dst, char delim);

// converts an integer number to a string and stores it in buf
void intToStr(int val, char *buf);

// converts a float number to a string with 2 decimal places and stores it in buf
void floatToStr(float val, char *buf);

// reads an integer number from a string (like atoi)
int strToInt(const char *s);

// reads a float number from a string (like atof)
float strToFloat(const char *s);

// returns true if every character in the string is a digit 0-9
bool isAllDigits(const char *s);

// converts a single character to lowercase manually (without using any library)
char toLowerChar(char c);