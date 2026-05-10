#include "Bill.h"
#include <iostream>

Bill::Bill() : id(0), patientId(0), appointmentId(0), amount(0.0f)
{
    status = new char[12];
    date = new char[12];
    status[0] = date[0] = '\0';
}

Bill::Bill(int id, int patId, int apptId, float amt,
           const char *st, const char *dt)
    : id(id), patientId(patId), appointmentId(apptId), amount(amt)
{
    status = new char[12];
    date = new char[12];
    myStrncpy(status, st, 12);
    myStrncpy(date, dt, 12);
}

Bill::Bill(const Bill &other)
    : id(other.id), patientId(other.patientId),
      appointmentId(other.appointmentId), amount(other.amount)
{
    status = new char[12];
    date = new char[12];

    myStrncpy(status, other.status, 12);
    myStrncpy(date, other.date, 12);
}

Bill &Bill::operator=(const Bill &other)
{
    if (this != &other)
    {
        id = other.id;
        patientId = other.patientId;
        appointmentId = other.appointmentId;
        amount = other.amount;

        myStrncpy(status, other.status, 12);
        myStrncpy(date, other.date, 12);
    }

    return *this;
}

int Bill::getId() const { return id; }
int Bill::getPatientId() const { return patientId; }
int Bill::getAppointmentId() const { return appointmentId; }
float Bill::getAmount() const { return amount; }
const char *Bill::getStatus() const { return status; }
const char *Bill::getDate() const { return date; }

void Bill::setId(int i) { id = i; }
void Bill::setStatus(const char *s) { myStrncpy(status, s, 12); }
void Bill::setPatientId(int pid) { patientId = pid; }
void Bill::setAppointmentId(int aid) { appointmentId = aid; }
void Bill::setAmount(float a) { amount = a; }
void Bill::setDate(const char *d) { myStrncpy(date, d, 12); }

std::ostream &operator<<(std::ostream &os, const Bill &b)
{
    char buf[32];
    floatToStr(b.amount, buf);
    os << "Bill[" << b.id << "] Pat:" << b.patientId
       << " Appt:" << b.appointmentId
       << " PKR:" << buf
       << " [" << b.status << "] " << b.date;
    return os;
}

Bill::~Bill()
{
    delete[] status;
    delete[] date;
}