#include "HospitalException.h"

// Thrown when the user types something that fails validation.
// For example: a negative fee, a contact number that is not 11 digits,
// or a date in the wrong format.
class InvalidInputException : public HospitalException
{
public:
    // takes a short description of what was wrong with the input
    InvalidInputException(const char *detail);
};