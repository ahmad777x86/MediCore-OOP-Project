#include "Patient.h"
#include <iostream>

Patient::Patient() : Person(), age(0), balance(0.0f)
{
    gender[0] = '\0';
}

Patient::Patient(int id, const char *nm, int age, const char *gen,
                 const char *contact, const char *pw, float bal)
    : Person(id, nm, pw, contact), age(age), balance(bal)
{
    myStrncpy(gender, gen, 4);
}

int Patient::getAge() const { return age; }
const char *Patient::getGender() const { return gender; }
float Patient::getBalance() const { return balance; }
void Patient::setAge(int a) { age = a; }
void Patient::setGender(const char *g) { myStrncpy(gender, g, 4); }
void Patient::setBalance(float b) { balance = b; }

Patient &Patient::operator+=(float amount)
{
    balance += amount;
    return *this;
}

Patient &Patient::operator-=(float amount)
{
    balance -= amount;
    return *this;
}

bool Patient::operator==(const Patient &other) const
{
    return id == other.id;
}

std::ostream &operator<<(std::ostream &os, const Patient &p)
{
    char buf[32];
    os << "Patient [ID:" << p.id << "] " << p.name
       << " | Age:" << p.age << " | Gender:" << p.gender
       << " | Contact:" << p.contact << " | Balance:PKR ";
    floatToStr(p.balance, buf);
    os << buf;
    return os;
}

void Patient::displayMenu()
{
    char buf[32];
    floatToStr(balance, buf);
    std::cout << "\nWelcome, " << name << "\nBalance: PKR " << buf << "\n";
    std::cout << "========================\n";
    std::cout << "1. Book Appointment\n";
    std::cout << "2. Cancel Appointment\n";
    std::cout << "3. View My Appointments\n";
    std::cout << "4. View My Medical Records\n";
    std::cout << "5. View My Bills\n";
    std::cout << "6. Pay Bill\n";
    std::cout << "7. Top Up Balance\n";
    std::cout << "8. Logout\n";
    std::cout << "Choice: ";
}

void Patient::display() const
{
    std::cout << *this << "\n";
}