#include "SlotUnavailableException.h"

// builds a message that tells the patient which slot was taken
SlotUnavailableException::SlotUnavailableException(const char *slot)
{
    myStrcpy(message, "Time slot ");
    myStrcat(message, slot);
    myStrcat(message, " is already taken. Please choose another slot.");
}