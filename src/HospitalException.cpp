#include "HospitalException.h"

// default constructor - starts with an empty message
HospitalException::HospitalException()
{
    message[0] = '\0';
}

// constructor that takes a message string and stores it
HospitalException::HospitalException(const char *msg)
{
    myStrncpy(message, msg, 200);
}

// returns the stored error message
const char *HospitalException::what() const
{
    return message;
}

// destructor - nothing to clean up since message is a fixed array
HospitalException::~HospitalException()
{
}