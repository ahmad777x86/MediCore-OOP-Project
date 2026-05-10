#include "Doctor.h"
#include <iostream>

Doctor::Doctor() : Person(), fee(0.0f)
{
    specialization = new char[60];
    specialization[0] = '\0';
}

Doctor::Doctor(int id, const char *nm, const char *spec,
               const char *ct, const char *pw, float f)
    : Person(id, nm, pw, ct), fee(f)
{
    specialization = new char[60];
    myStrncpy(specialization, spec, 60);
}

Doctor::Doctor(const Doctor &other)
    : Person(other), fee(other.fee)
{
    specialization = new char[60];
    myStrncpy(specialization, other.specialization, 60);
}

Doctor &Doctor::operator=(const Doctor &other)
{
    if (this != &other)
    {
        Person::operator=(other);

        fee = other.fee;

        myStrncpy(specialization, other.specialization, 60);
    }
    return *this;
}

const char *Doctor::getSpecialization() const { return specialization; }
float Doctor::getFee() const { return fee; }
void Doctor::setSpecialization(const char *s) { myStrncpy(specialization, s, 60); }
void Doctor::setFee(float f) { fee = f; }

bool Doctor::operator==(const Doctor &other) const
{
    return id == other.id;
}

std::ostream &operator<<(std::ostream &os, const Doctor &d)
{
    char buf[32];
    floatToStr(d.fee, buf);
    os << "Doctor [ID:" << d.id << "] Dr." << d.name
       << " | Spec:" << d.specialization
       << " | Contact:" << d.contact
       << " | Fee:PKR " << buf;
    return os;
}

void Doctor::displayMenu()
{
    std::cout << "\nWelcome, Dr. " << name
              << " | Specialization: " << specialization << "\n";
    std::cout << "===============================================\n";
    std::cout << "1. View Today's Appointments\n";
    std::cout << "2. Mark Appointment Complete\n";
    std::cout << "3. Mark Appointment No-Show\n";
    std::cout << "4. Write Prescription\n";
    std::cout << "5. View Patient Medical History\n";
    std::cout << "6. Logout\n";
    std::cout << "Choice: ";
}

void Doctor::display() const { std::cout << *this << "\n"; }

Doctor::~Doctor()
{
    delete[] specialization;
}