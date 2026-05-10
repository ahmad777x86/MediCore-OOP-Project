#include "FileNotFoundException.h"

// builds the error message by combining a fixed prefix with the filename
FileNotFoundException::FileNotFoundException(const char *filename)
{
    myStrcpy(message, "File not found: ");
    myStrcat(message, filename);
}