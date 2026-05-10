#pragma once
#include "Person.h"
#include <iostream>

class Doctor : public Person
{
private:
    char *specialization;
    float fee;

public:
    Doctor();
    Doctor(int id, const char *name, const char *spec,
           const char *contact, const char *password, float fee);
    Doctor(const Doctor &other);

    const char *getSpecialization() const;
    Doctor &operator=(const Doctor &other);

    float getFee() const;
    void setSpecialization(const char *s);
    void setFee(float f);

    bool operator==(const Doctor &other) const;
    friend std::ostream &operator<<(std::ostream &os, const Doctor &d);

    void displayMenu() override;
    void display() const override;
    ~Doctor();
};