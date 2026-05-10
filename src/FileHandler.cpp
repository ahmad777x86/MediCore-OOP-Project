#include "FileHandler.h"
#include <fstream>
#include <iostream>

// ── helper: reads one line from an ifstream into a fixed char buffer ──────────
// we cannot use std::string so we read character by character until newline
static void readLine(std::ifstream &file, char *buf, int maxLen)
{
    int i = 0;
    char c;

    while (i < maxLen - 1 && file.get(c))
    {
        if (c == '\n')
            break;
        // skip carriage return (Windows line endings)
        if (c == '\r')
            continue;
        buf[i] = c;
        i++;
    }

    buf[i] = '\0';
}

void FileHandler::loadPatients(Storage<Patient> &store)
{
    std::ifstream file("patients.txt");

    // if the file does not exist just return quietly
    if (!file.is_open())
        return;

    char line[512];

    while (file.good())
    {
        readLine(file, line, 512);

        // skip empty lines
        if (line[0] == '\0')
            continue;

        char sid[16], nm[100], age[8], gen[4], ct[16], pw[50], bal[32];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, nm, 100, ',');
        p = parseToken(p, age, 8, ',');
        p = parseToken(p, gen, 4, ',');
        p = parseToken(p, ct, 16, ',');
        p = parseToken(p, pw, 50, ',');
        p = parseToken(p, bal, 32, ',');

        Patient *pat = new Patient(strToInt(sid), nm, strToInt(age),
                                   gen, ct, pw, strToFloat(bal));
        store.add(pat);
    }

    file.close();
}

void FileHandler::loadDoctors(Storage<Doctor> &store)
{
    std::ifstream file("doctors.txt");

    if (!file.is_open())
        return;

    char line[512];

    while (file.good())
    {
        readLine(file, line, 512);

        if (line[0] == '\0')
            continue;

        char sid[16], nm[100], spec[60], ct[16], pw[50], fee[32];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, nm, 100, ',');
        p = parseToken(p, spec, 60, ',');
        p = parseToken(p, ct, 16, ',');
        p = parseToken(p, pw, 50, ',');
        p = parseToken(p, fee, 32, ',');

        Doctor *doc = new Doctor(strToInt(sid), nm, spec, ct, pw, strToFloat(fee));
        store.add(doc);
    }

    file.close();
}

void FileHandler::loadAdmin(Admin &admin)
{
    std::ifstream file("admin.txt");

    if (!file.is_open())
    {
        throw FileNotFoundException("admin.txt");
    }

    char line[256];
    readLine(file, line, 256);

    if (line[0] != '\0')
    {
        char sid[16], nm[100], pw[50];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, nm, 100, ',');
        p = parseToken(p, pw, 50, ',');

        admin.setId(strToInt(sid));
        admin.setName(nm);
        admin.setPassword(pw);
    }

    file.close();
}

void FileHandler::loadAppointments(Storage<Appointment> &store)
{
    std::ifstream file("appointments.txt");

    if (!file.is_open())
        return;

    char line[256];

    while (file.good())
    {
        readLine(file, line, 256);

        if (line[0] == '\0')
            continue;

        char sid[16], spid[16], sdid[16], dt[12], slot[8], st[12];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, spid, 16, ',');
        p = parseToken(p, sdid, 16, ',');
        p = parseToken(p, dt, 12, ',');
        p = parseToken(p, slot, 8, ',');
        p = parseToken(p, st, 12, ',');

        Appointment *a = new Appointment(strToInt(sid), strToInt(spid),
                                         strToInt(sdid), dt, slot, st);
        store.add(a);
    }

    file.close();
}

void FileHandler::loadBills(Storage<Bill> &store)
{
    std::ifstream file("bills.txt");

    if (!file.is_open())
        return;

    char line[256];

    while (file.good())
    {
        readLine(file, line, 256);

        if (line[0] == '\0')
            continue;

        char sid[16], spid[16], said[16], amt[32], st[12], dt[12];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, spid, 16, ',');
        p = parseToken(p, said, 16, ',');
        p = parseToken(p, amt, 32, ',');
        p = parseToken(p, st, 12, ',');
        p = parseToken(p, dt, 12, ',');

        Bill *b = new Bill(strToInt(sid), strToInt(spid), strToInt(said),
                           strToFloat(amt), st, dt);
        store.add(b);
    }

    file.close();
}

void FileHandler::loadPrescriptions(Storage<Prescription> &store)
{
    std::ifstream file("prescriptions.txt");

    if (!file.is_open())
        return;

    char line[900];

    while (file.good())
    {
        readLine(file, line, 900);

        if (line[0] == '\0')
            continue;

        char sid[16], said[16], spid[16], sdid[16], dt[12], med[500], nt[300];
        const char *p = line;
        p = parseToken(p, sid, 16, ',');
        p = parseToken(p, said, 16, ',');
        p = parseToken(p, spid, 16, ',');
        p = parseToken(p, sdid, 16, ',');
        p = parseToken(p, dt, 12, ',');
        p = parseToken(p, med, 500, ',');
        p = parseToken(p, nt, 300, ',');

        Prescription *rx = new Prescription(strToInt(sid), strToInt(said),
                                            strToInt(spid), strToInt(sdid),
                                            dt, med, nt);
        store.add(rx);
    }

    file.close();
}

void FileHandler::saveAllPatients(Storage<Patient> &store)
{
    // opening with out and trunc clears the file before writing
    std::ofstream file("patients.txt", std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return;

    for (int i = 0; i < store.size(); i++)
    {
        Patient *p = store.getAll()[i];
        if (!p)
            continue;

        char bal[32];
        floatToStr(p->getBalance(), bal);

        file << p->getId() << ","
             << p->getName() << ","
             << p->getAge() << ","
             << p->getGender() << ","
             << p->getContact() << ","
             << p->getPassword() << ","
             << bal << "\n";
    }

    file.close();
}

void FileHandler::saveAllDoctors(Storage<Doctor> &store)
{
    std::ofstream file("doctors.txt", std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return;

    for (int i = 0; i < store.size(); i++)
    {
        Doctor *d = store.getAll()[i];
        if (!d)
            continue;

        char fee[32];
        floatToStr(d->getFee(), fee);

        file << d->getId() << ","
             << d->getName() << ","
             << d->getSpecialization() << ","
             << d->getContact() << ","
             << d->getPassword() << ","
             << fee << "\n";
    }

    file.close();
}

void FileHandler::saveAllAppointments(Storage<Appointment> &store)
{
    std::ofstream file("appointments.txt", std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return;

    for (int i = 0; i < store.size(); i++)
    {
        Appointment *a = store.getAll()[i];
        if (!a)
            continue;

        file << a->getId() << ","
             << a->getPatientId() << ","
             << a->getDoctorId() << ","
             << a->getDate() << ","
             << a->getTimeSlot() << ","
             << a->getStatus() << "\n";
    }

    file.close();
}

void FileHandler::saveAllBills(Storage<Bill> &store)
{
    std::ofstream file("bills.txt", std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return;

    for (int i = 0; i < store.size(); i++)
    {
        Bill *b = store.getAll()[i];
        if (!b)
            continue;

        char amt[32];
        floatToStr(b->getAmount(), amt);

        file << b->getId() << ","
             << b->getPatientId() << ","
             << b->getAppointmentId() << ","
             << amt << ","
             << b->getStatus() << ","
             << b->getDate() << "\n";
    }

    file.close();
}

void FileHandler::saveAllPrescriptions(Storage<Prescription> &store)
{
    std::ofstream file("prescriptions.txt", std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return;

    for (int i = 0; i < store.size(); i++)
    {
        Prescription *rx = store.getAll()[i];
        if (!rx)
            continue;

        file << rx->getId() << ","
             << rx->getAppointmentId() << ","
             << rx->getPatientId() << ","
             << rx->getDoctorId() << ","
             << rx->getDate() << ","
             << rx->getMedicines() << ","
             << rx->getNotes() << "\n";
    }

    file.close();
}

void FileHandler::appendAppointment(const Appointment &a)
{
    // opening with app means we add to the end without clearing the file
    std::ofstream file("appointments.txt", std::ios::app);

    if (!file.is_open())
        return;

    file << a.getId() << ","
         << a.getPatientId() << ","
         << a.getDoctorId() << ","
         << a.getDate() << ","
         << a.getTimeSlot() << ","
         << a.getStatus() << "\n";

    file.close();
}

void FileHandler::appendBill(const Bill &b)
{
    std::ofstream file("bills.txt", std::ios::app);

    if (!file.is_open())
        return;

    char amt[32];
    floatToStr(b.getAmount(), amt);

    file << b.getId() << ","
         << b.getPatientId() << ","
         << b.getAppointmentId() << ","
         << amt << ","
         << b.getStatus() << ","
         << b.getDate() << "\n";

    file.close();
}

void FileHandler::appendDoctor(const Doctor &d)
{
    std::ofstream file("doctors.txt", std::ios::app);

    if (!file.is_open())
        return;

    char fee[32];
    floatToStr(d.getFee(), fee);

    file << d.getId() << ","
         << d.getName() << ","
         << d.getSpecialization() << ","
         << d.getContact() << ","
         << d.getPassword() << ","
         << fee << "\n";

    file.close();
}

void FileHandler::appendPrescription(const Prescription &rx)
{
    std::ofstream file("prescriptions.txt", std::ios::app);

    if (!file.is_open())
        return;

    file << rx.getId() << ","
         << rx.getAppointmentId() << ","
         << rx.getPatientId() << ","
         << rx.getDoctorId() << ","
         << rx.getDate() << ","
         << rx.getMedicines() << ","
         << rx.getNotes() << "\n";

    file.close();
}

void FileHandler::appendPatient(const Patient &p)
{
    std::ofstream file("patients.txt", std::ios::app);

    if (!file.is_open())
        return;

    char bal[32];
    floatToStr(p.getBalance(), bal);

    file << p.getId() << ","
         << p.getName() << ","
         << p.getAge() << ","
         << p.getGender() << ","
         << p.getContact() << ","
         << p.getPassword() << ","
         << bal << "\n";

    file.close();
}

void FileHandler::logSecurityEvent(const char *role, const char *enteredId, const char *result)
{
    std::ofstream file("security_log.txt", std::ios::app);

    if (!file.is_open())
        return;

    char timestamp[24];
    Validator::getTimestampStr(timestamp);

    file << timestamp << ","
         << role << ","
         << enteredId << ","
         << result << "\n";

    file.close();
}

void FileHandler::printSecurityLog()
{
    std::ifstream file("security_log.txt");

    if (!file.is_open())
    {
        std::cout << "No security events logged." << std::endl;
        return;
    }

    char line[256];
    bool any = false;

    while (file.good())
    {
        readLine(file, line, 256);
        if (line[0] == '\0')
            continue;
        std::cout << line << "\n";
        any = true;
    }

    file.close();

    if (!any)
        std::cout << "No security events logged." << std::endl;
}

void FileHandler::readSecurityLog(char *buf, int maxLen)
{
    std::ifstream file("security_log.txt");

    if (!file.is_open())
    {
        myStrcpy(buf, "No security events logged.");
        return;
    }

    int pos = 0;
    char c;

    while (pos < maxLen - 1 && file.get(c))
    {
        buf[pos] = c;
        pos++;
    }

    buf[pos] = '\0';

    file.close();
}

void FileHandler::dischargePatient(int patientId,
                                   Storage<Patient> &patients,
                                   Storage<Appointment> &appointments,
                                   Storage<Bill> &bills,
                                   Storage<Prescription> &prescriptions)
{
    std::ofstream file("discharged.txt", std::ios::app);

    if (!file.is_open())
        return;

    // write the patient's own record
    Patient *p = patients.findById(patientId);
    if (p)
    {
        char bal[32];
        floatToStr(p->getBalance(), bal);

        file << "PATIENT:"
             << p->getId() << ","
             << p->getName() << ","
             << p->getAge() << ","
             << p->getGender() << ","
             << p->getContact() << ","
             << p->getPassword() << ","
             << bal << "\n";
    }

    // write all appointments belonging to this patient
    for (int i = 0; i < appointments.size(); i++)
    {
        Appointment *a = appointments.getAll()[i];
        if (a && a->getPatientId() == patientId)
        {
            file << "APPOINTMENT:"
                 << a->getId() << ","
                 << a->getPatientId() << ","
                 << a->getDoctorId() << ","
                 << a->getDate() << ","
                 << a->getTimeSlot() << ","
                 << a->getStatus() << "\n";
        }
    }

    // write all bills belonging to this patient
    for (int i = 0; i < bills.size(); i++)
    {
        Bill *b = bills.getAll()[i];
        if (b && b->getPatientId() == patientId)
        {
            char amt[32];
            floatToStr(b->getAmount(), amt);

            file << "BILL:"
                 << b->getId() << ","
                 << b->getPatientId() << ","
                 << b->getAppointmentId() << ","
                 << amt << ","
                 << b->getStatus() << ","
                 << b->getDate() << "\n";
        }
    }

    // write all prescriptions belonging to this patient
    for (int i = 0; i < prescriptions.size(); i++)
    {
        Prescription *rx = prescriptions.getAll()[i];
        if (rx && rx->getPatientId() == patientId)
        {
            file << "PRESCRIPTION:"
                 << rx->getId() << ","
                 << rx->getAppointmentId() << ","
                 << rx->getPatientId() << ","
                 << rx->getDoctorId() << ","
                 << rx->getDate() << ","
                 << rx->getMedicines() << ","
                 << rx->getNotes() << "\n";
        }
    }

    file.close();
}