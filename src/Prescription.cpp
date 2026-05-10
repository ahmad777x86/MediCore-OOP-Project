#include "Prescription.h"
#include <iostream>

Prescription::Prescription()
    : id(0), appointmentId(0), patientId(0), doctorId(0)
{
    date = new char[12];
    medicines = new char[500];
    notes = new char[300];
    date[0] = medicines[0] = notes[0] = '\0';
}

Prescription::Prescription(int id, int apptId, int patId, int docId,
                           const char *dt, const char *med, const char *nt)
    : id(id), appointmentId(apptId), patientId(patId), doctorId(docId)
{
    date = new char[12];
    medicines = new char[500];
    notes = new char[300];
    myStrncpy(date, dt, 12);
    myStrncpy(medicines, med, 500);
    myStrncpy(notes, nt, 300);
}

Prescription::Prescription(const Prescription &other)
    : id(other.id), appointmentId(other.appointmentId),
      patientId(other.patientId), doctorId(other.doctorId)
{
    date = new char[12];
    medicines = new char[500];
    notes = new char[300];

    myStrncpy(date, other.date, 12);
    myStrncpy(medicines, other.medicines, 500);
    myStrncpy(notes, other.notes, 300);
}

Prescription &Prescription::operator=(const Prescription &other)
{
    if (this != &other)
    {
        id = other.id;
        appointmentId = other.appointmentId;
        patientId = other.patientId;
        doctorId = other.doctorId;

        myStrncpy(date, other.date, 12);
        myStrncpy(medicines, other.medicines, 500);
        myStrncpy(notes, other.notes, 300);
    }

    return *this;
}

int Prescription::getId() const { return id; }
int Prescription::getAppointmentId() const { return appointmentId; }
int Prescription::getPatientId() const { return patientId; }
int Prescription::getDoctorId() const { return doctorId; }
const char *Prescription::getDate() const { return date; }
const char *Prescription::getMedicines() const { return medicines; }
const char *Prescription::getNotes() const { return notes; }

void Prescription::setId(int i) { id = i; }
void Prescription::setAppointmentId(int a) { appointmentId = a; }
void Prescription::setPatientId(int p) { patientId = p; }
void Prescription::setDoctorId(int d) { doctorId = d; }
void Prescription::setDate(const char *dt) { myStrncpy(date, dt, 12); }
void Prescription::setMedicines(const char *m) { myStrncpy(medicines, m, 500); }
void Prescription::setNotes(const char *n) { myStrncpy(notes, n, 300); }

std::ostream &operator<<(std::ostream &os, const Prescription &p)
{
    os << "Rx[" << p.id << "] Appt:" << p.appointmentId
       << " " << p.date
       << "\n  Medicines: " << p.medicines
       << "\n  Notes: " << p.notes;
    return os;
}

Prescription::~Prescription()
{
    delete[] date;
    delete[] medicines;
    delete[] notes;
}