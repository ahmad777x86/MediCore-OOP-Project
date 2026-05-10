#include "Admin.h"
#include <iostream>

Admin::Admin() : Person() {}
Admin::Admin(int id, const char *nm, const char *pw)
    : Person(id, nm, pw, "") {}

void Admin::displayMenu()
{
    std::cout << "\nAdmin Panel - MediCore\n";
    std::cout << "======================\n";
    std::cout << "1.  Add Doctor\n";
    std::cout << "2.  Remove Doctor\n";
    std::cout << "3.  View All Patients\n";
    std::cout << "4.  View All Doctors\n";
    std::cout << "5.  View All Appointments\n";
    std::cout << "6.  View Unpaid Bills\n";
    std::cout << "7.  Discharge Patient\n";
    std::cout << "8.  View Security Log\n";
    std::cout << "9.  Generate Daily Report\n";
    std::cout << "10. Logout\n";
    std::cout << "Choice: ";
}

void Admin::display() const
{
    std::cout << "Admin [ID:" << id << "] " << name << "\n";
}