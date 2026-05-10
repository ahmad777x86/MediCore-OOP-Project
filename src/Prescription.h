#pragma once
#include "StrUtils.h"
#include <iostream>

class Prescription
{
private:
    int id;
    int appointmentId;
    int patientId;
    int doctorId;
    char *date;
    char *medicines;
    char *notes;

public:
    Prescription();
    Prescription(int id, int apptId, int patId, int docId,
                 const char *date, const char *medicines, const char *notes);
    Prescription(const Prescription &other);
    Prescription &operator=(const Prescription &other);
    int getId() const;
    int getAppointmentId() const;
    int getPatientId() const;
    int getDoctorId() const;
    const char *getDate() const;
    const char *getMedicines() const;
    const char *getNotes() const;

    void setId(int i);
    void setAppointmentId(int a);
    void setPatientId(int p);
    void setDoctorId(int d);
    void setDate(const char *dt);
    void setMedicines(const char *m);
    void setNotes(const char *n);

    friend std::ostream &operator<<(std::ostream &os, const Prescription &p);

    ~Prescription();
};