#pragma once
#include "StrUtils.h"
#include <iostream>

class Person
{
protected:
    int id;
    char *name;
    char *password;
    char *contact;

public:
    Person() : id(0)
    {
        name = new char[50];
        password = new char[30];
        contact = new char[15];
        name[0] = password[0] = contact[0] = '\0';
    }

    Person(int id, const char *nm, const char *pw, const char *ct) : id(id)
    {
        name = new char[50];
        password = new char[30];
        contact = new char[15];

        myStrncpy(name, nm, 50);
        myStrncpy(password, pw, 30);
        myStrncpy(contact, ct, 15);
    }

    virtual ~Person()
    {
        delete[] name;
        delete[] password;
        delete[] contact;
    }

    Person(const Person &other) : id(other.id)
    {
        name = new char[50];
        password = new char[30];
        contact = new char[15];

        myStrncpy(name, other.name, 50);
        myStrncpy(password, other.password, 30);
        myStrncpy(contact, other.contact, 15);
    }

    Person &operator=(const Person &other)
    {
        if (this != &other)
        {
            id = other.id;

            myStrncpy(name, other.name, 50);
            myStrncpy(password, other.password, 30);
            myStrncpy(contact, other.contact, 15);
        }
        return *this;
    }

    // Pure Virtual Functions
    virtual void displayMenu() = 0;
    virtual void display() const = 0;

    // Getters
    int getId() const { return id; }
    const char *getName() const { return name; }
    const char *getContact() const { return contact; }
    const char *getPassword() const { return password; }

    bool checkPassword(const char *pw) const
    {
        return myStrcmp(password, pw) == 0;
    }

    void setId(int i) { id = i; }
    void setName(const char *nm) { myStrncpy(name, nm, 50); }
    void setPassword(const char *p) { myStrncpy(password, p, 30); }
    void setContact(const char *c) { myStrncpy(contact, c, 15); }
};