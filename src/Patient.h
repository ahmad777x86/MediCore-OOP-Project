#pragma once
#include "Person.h"
#include <iostream>

class Patient : public Person
{
private:
    int age;
    char gender[4]; // M / F
    float balance;

public:
    Patient();
    Patient(int id, const char *name, int age, const char *gender,
            const char *contact, const char *password, float balance);

    int getAge() const;
    const char *getGender() const;
    float getBalance() const;
    void setAge(int a);
    void setGender(const char *g);
    void setBalance(float b);

    Patient &operator+=(float amount);           // add to balance
    Patient &operator-=(float amount);           // deduct from balance
    bool operator==(const Patient &other) const; // compare by ID
    friend std::ostream &operator<<(std::ostream &os, const Patient &p);

    void displayMenu() override;
    void display() const override;
};