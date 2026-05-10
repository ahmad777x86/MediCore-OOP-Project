#pragma once
#include "StrUtils.h"

// Base class for all custom exceptions in this project.
// Every exception stores a message and can return it through what().
class HospitalException
{
protected:
    char message[200];

public:
    HospitalException();
    HospitalException(const char *msg);

    // virtual so that child classes can override it
    virtual const char *what() const;

    // virtual destructor is required when using inheritance
    virtual ~HospitalException();
};