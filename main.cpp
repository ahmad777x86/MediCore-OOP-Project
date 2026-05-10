#include "HospitalSystem.h"

// main() is a sequence of function calls only (as per spec)
int main()
{
    HospitalSystem hospital;
    hospital.loadAll();
    hospital.run();
    hospital.freeAll();
    return 0;
}