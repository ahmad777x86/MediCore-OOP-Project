#pragma once
#include "StrUtils.h"
#include <iostream>

class Appointment
{
private:
    int id;
    int patientId;
    int doctorId;
    char date[12];    // DD-MM-YYYY
    char timeSlot[8]; // HH:MM
    char status[12];  // pending / completed / cancelled / noshow

public:
    Appointment();
    Appointment(int id, int patId, int docId,
                const char *date, const char *slot, const char *status);

    int getId() const;
    int getPatientId() const;
    int getDoctorId() const;
    const char *getDate() const;
    const char *getTimeSlot() const;
    const char *getStatus() const;

    void setId(int i);
    void setStatus(const char *s);
    void setDate(const char *d);
    void setTimeSlot(const char *t);
    void setPatientId(int pid);
    void setDoctorId(int did);

    // == detects scheduling conflict: same doctor, same date, same slot,
    //    neither appointment is cancelled
    bool operator==(const Appointment &other) const;
    friend std::ostream &operator<<(std::ostream &os, const Appointment &a);
};