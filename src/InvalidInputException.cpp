#include "InvalidInputException.h"

// builds the message by combining "Invalid input: " with the detail given
InvalidInputException::InvalidInputException(const char *detail)
{
    myStrcpy(message, "Invalid input: ");
    myStrcat(message, detail);
}