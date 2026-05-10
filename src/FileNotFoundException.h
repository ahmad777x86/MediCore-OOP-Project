#pragma once
#include "HospitalException.h"

// Thrown when a required .txt file cannot be opened at startup.
// For example: patients.txt or admin.txt is missing.
class FileNotFoundException : public HospitalException
{
public:
    // takes the name of the file that could not be found
    FileNotFoundException(const char *filename);
};