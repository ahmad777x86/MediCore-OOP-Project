#include "HospitalException.h"

// Thrown when a patient tries to book an appointment or pay a bill
// but does not have enough balance to cover the cost.
class InsufficientFundsException : public HospitalException
{
public:
    // use this when you don't know the exact amounts
    InsufficientFundsException();

    // use this to show the patient exactly how much is needed vs available
    InsufficientFundsException(float required, float available);
};