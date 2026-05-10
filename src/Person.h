#pragma once
#include "StrUtils.h"
#include <iostream>

class Person
{
protected:
    int id;
    char name[100];
    char password[50];
    char contact[15];

public:
    Person() : id(0)
    {
        name[0] = password[0] = contact[0] = '\0';
    }
    Person(int id, const char *nm, const char *pw, const char *ct) : id(id)
    {
        myStrncpy(name, nm, 100);
        myStrncpy(password, pw, 50);
        myStrncpy(contact, ct, 15);
    }
    virtual ~Person() {}

    virtual void displayMenu() = 0;
    virtual void display() const = 0;

    int getId() const { return id; }
    const char *getName() const { return name; }
    const char *getContact() const { return contact; }

    bool checkPassword(const char *pw) const
    {
        return myStrcmp(password, pw) == 0;
    }

    void setId(int i) { id = i; }
    void setName(const char *nm) { myStrncpy(name, nm, 100); }
    void setPassword(const char *p) { myStrncpy(password, p, 50); }
    void setContact(const char *c) { myStrncpy(contact, c, 15); }
    const char *getPassword() const { return password; }
};