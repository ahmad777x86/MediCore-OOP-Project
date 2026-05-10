#include "HospitalException.h"

// Thrown when a patient tries to book a time slot that is already taken
// by another appointment with the same doctor on the same date.
class SlotUnavailableException : public HospitalException
{
public:
    // takes the time slot string (e.g. "09:00") that was unavailable
    SlotUnavailableException(const char *slot);
};
