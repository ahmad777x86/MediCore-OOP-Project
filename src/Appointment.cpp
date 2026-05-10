#include "Appointment.h"
#include <iostream>

Appointment::Appointment()
    : id(0), patientId(0), doctorId(0)
{
    // Allocate exact sizes (+1 for safety if needed, matching original limits)
    date = new char[12]{'\0'};
    timeSlot = new char[8]{'\0'};
    status = new char[12]{'\0'};
}

// Parameterized Constructor
Appointment::Appointment(int id, int patId, int docId,
                         const char *dt, const char *slot, const char *st)
    : id(id), patientId(patId), doctorId(docId)
{
    date = new char[12];
    timeSlot = new char[8];
    status = new char[12];

    myStrncpy(date, dt, 12);
    myStrncpy(timeSlot, slot, 8);
    myStrncpy(status, st, 12);
}
Appointment::Appointment(const Appointment &other)
    : id(other.id), patientId(other.patientId), doctorId(other.doctorId)
{
    date = new char[12];
    timeSlot = new char[8];
    status = new char[12];

    myStrncpy(date, other.date, 12);
    myStrncpy(timeSlot, other.timeSlot, 8);
    myStrncpy(status, other.status, 12);
}

Appointment &Appointment::operator=(const Appointment &other)
{
    if (this != &other)
    {
        id = other.id;
        patientId = other.patientId;
        doctorId = other.doctorId;

        myStrncpy(date, other.date, 12);
        myStrncpy(timeSlot, other.timeSlot, 8);
        myStrncpy(status, other.status, 12);
    }
    return *this;
}

Appointment::~Appointment()
{
    delete[] date;
    delete[] timeSlot;
    delete[] status;
}

int Appointment::getId() const { return id; }
int Appointment::getPatientId() const { return patientId; }
int Appointment::getDoctorId() const { return doctorId; }
const char *Appointment::getDate() const { return date; }
const char *Appointment::getTimeSlot() const { return timeSlot; }
const char *Appointment::getStatus() const { return status; }

void Appointment::setId(int i) { id = i; }
void Appointment::setStatus(const char *s) { myStrncpy(status, s, 12); }
void Appointment::setDate(const char *d) { myStrncpy(date, d, 12); }
void Appointment::setTimeSlot(const char *t) { myStrncpy(timeSlot, t, 8); }
void Appointment::setPatientId(int pid) { patientId = pid; }
void Appointment::setDoctorId(int did) { doctorId = did; }

// conflict: same doctor, same date, same slot, NEITHER is cancelled
bool Appointment::operator==(const Appointment &other) const
{
    if (doctorId != other.doctorId)
        return false;
    if (myStrcmp(date, other.date) != 0)
        return false;
    if (myStrcmp(timeSlot, other.timeSlot) != 0)
        return false;
    if (myStrcmp(status, "cancelled") == 0)
        return false;
    if (myStrcmp(other.status, "cancelled") == 0)
        return false;
    return true;
}

std::ostream &operator<<(std::ostream &os, const Appointment &a)
{
    os << "Appt[" << a.id << "] Pat:" << a.patientId
       << " Doc:" << a.doctorId
       << " " << a.date << " " << a.timeSlot
       << " [" << a.status << "]";
    return os;
}