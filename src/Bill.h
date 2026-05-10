#pragma once
#include "StrUtils.h"
#include <iostream>

class Bill
{
private:
    int id;
    int patientId;
    int appointmentId;
    float amount;
    char *status; // unpaid / paid / cancelled
    char *date;   // DD-MM-YYYY

public:
    Bill();
    Bill(int id, int patId, int apptId, float amount,
         const char *status, const char *date);
    Bill(const Bill &other);
    Bill &operator=(const Bill &other);
    ~Bill();
    int getId() const;
    int getPatientId() const;
    int getAppointmentId() const;
    float getAmount() const;
    const char *getStatus() const;
    const char *getDate() const;

    void setId(int i);
    void setStatus(const char *s);
    void setPatientId(int pid);
    void setAppointmentId(int aid);
    void setAmount(float a);
    void setDate(const char *d);

    friend std::ostream &operator<<(std::ostream &os, const Bill &b);
};