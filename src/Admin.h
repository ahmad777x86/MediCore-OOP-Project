#pragma once
#include "Person.h"
#include <iostream>

class Admin : public Person
{
public:
    Admin();
    Admin(int id, const char *name, const char *password);

    void displayMenu() override;
    void display() const override;
};