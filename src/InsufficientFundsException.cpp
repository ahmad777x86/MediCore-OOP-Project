#include "InsufficientFundsException.h"

// simple version - just says balance is not enough
InsufficientFundsException::InsufficientFundsException()
{
    myStrcpy(message, "Insufficient balance. Please top up your account.");
}

// detailed version - shows how much was needed and how much the patient has
InsufficientFundsException::InsufficientFundsException(float required, float available)
{
    char requiredStr[32];
    char availableStr[32];

    floatToStr(required, requiredStr);
    floatToStr(available, availableStr);

    myStrcpy(message, "Insufficient funds. Required: PKR ");
    myStrcat(message, requiredStr);
    myStrcat(message, ", Available: PKR ");
    myStrcat(message, availableStr);
}