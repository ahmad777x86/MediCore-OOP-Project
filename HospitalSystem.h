#pragma once
#include "src/Storage.h"
#include "src/Patient.h"
#include "src/Doctor.h"
#include "src/Admin.h"
#include "src/Appointment.h"
#include "src/Bill.h"
#include "src/Prescription.h"
#include "src/FileHandler.h"
#include "src/Validator.h"
#include "src/HospitalException.h"
#include "src/FileNotFoundException.h"
#include "src/InsufficientFundsException.h"
#include "src/InvalidInputException.h"
#include "src/SlotUnavailableException.h"
#include <iostream>

using namespace std;

// HospitalSystem orchestrates all menus and business logic.
// Stays in .h because Storage<T> is a template — the compiler needs
// to see full template definitions at the point of use, so everything
// that depends on them must also be visible at compile time.
class HospitalSystem
{
private:
    Storage<Patient> patients;
    Storage<Doctor> doctors;
    Storage<Appointment> appointments;
    Storage<Bill> bills;
    Storage<Prescription> prescriptions;
    Admin admin;

    // ── input helpers ─────────────────────────────────────────────────────────
    // All buffers are heap-allocated — caller must delete[] the returned pointer.

    // reads one integer from stdin
    int readInt()
    {
        char *buf = new char[32];
        int i = 0;
        int c;

        while ((c = cin.get()) != '\n' && c != EOF && i < 31)
        {
            buf[i] = (char)c;
            i++;
        }
        buf[i] = '\0';

        int result = strToInt(buf);
        delete[] buf;
        return result;
    }

    // reads one line from stdin into a caller-supplied dynamic buffer
    void readLine(char *dst, int maxLen)
    {
        int i = 0;
        int c;

        while ((c = cin.get()) != '\n' && c != EOF && i < maxLen - 1)
        {
            dst[i] = (char)c;
            i++;
        }
        dst[i] = '\0';
    }

    // ── sorting helpers (bubble sort, no library) ─────────────────────────────

    void sortAppointmentsAsc(Appointment **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < n - i - 1; j++)
            {
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) > 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
            }
        }
    }

    void sortAppointmentsDesc(Appointment **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < n - i - 1; j++)
            {
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
            }
        }
    }

    void sortPrescriptionsDesc(Prescription **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < n - i - 1; j++)
            {
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                {
                    Prescription *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
            }
        }
    }

    // ── name lookup helpers ───────────────────────────────────────────────────

    const char *getDoctorName(int docId)
    {
        Doctor *d = doctors.findById(docId);
        return d ? d->getName() : "Unknown";
    }

    const char *getPatientName(int patId)
    {
        Patient *p = patients.findById(patId);
        return p ? p->getName() : "Unknown";
    }

    // ── login with 3-attempt lockout ──────────────────────────────────────────

    bool loginWithLockout(const char *role, int *outId)
    {
        int attempts = 0;

        while (attempts < 3)
        {
            char *idBuf = new char[32];
            char *pwBuf = new char[64];

            cout << "Enter ID: ";
            readLine(idBuf, 32);
            cout << "Enter Password: ";
            readLine(pwBuf, 64);

            bool found = false;

            if (myStrcmp(role, "Patient") == 0)
            {
                for (int i = 0; i < patients.size(); i++)
                {
                    Patient *p = patients.getAll()[i];
                    char *tmp = new char[16];
                    intToStr(p->getId(), tmp);

                    if (myStrcmp(tmp, idBuf) == 0 && p->checkPassword(pwBuf))
                    {
                        *outId = p->getId();
                        found = true;
                        delete[] tmp;
                        break;
                    }
                    delete[] tmp;
                }
            }
            else if (myStrcmp(role, "Doctor") == 0)
            {
                for (int i = 0; i < doctors.size(); i++)
                {
                    Doctor *d = doctors.getAll()[i];
                    char *tmp = new char[16];
                    intToStr(d->getId(), tmp);

                    if (myStrcmp(tmp, idBuf) == 0 && d->checkPassword(pwBuf))
                    {
                        *outId = d->getId();
                        found = true;
                        delete[] tmp;
                        break;
                    }
                    delete[] tmp;
                }
            }
            else if (myStrcmp(role, "Admin") == 0)
            {
                char *tmp = new char[16];
                intToStr(admin.getId(), tmp);

                if (myStrcmp(tmp, idBuf) == 0 && admin.checkPassword(pwBuf))
                {
                    *outId = admin.getId();
                    found = true;
                }
                delete[] tmp;
            }

            if (found)
            {
                FileHandler::logSecurityEvent(role, idBuf, "SUCCESS");
                delete[] idBuf;
                delete[] pwBuf;
                return true;
            }

            attempts++;
            FileHandler::logSecurityEvent(role, idBuf, "FAILED");

            char *rem = new char[8];
            intToStr(3 - attempts, rem);
            cout << "Invalid credentials. Attempts remaining: " << rem << "\n";
            delete[] rem;

            delete[] idBuf;
            delete[] pwBuf;
        }

        cout << "Account locked. Contact admin.\n";
        return false;
    }

    // ── PATIENT ACTIONS ───────────────────────────────────────────────────────

    void bookAppointment(Patient *pat)
    {
        char *spec = new char[60];
        cout << "Enter specialization to search: ";
        readLine(spec, 60);

        // collect matching doctors into a dynamic array
        int mCount = 0;
        Doctor **matched = new Doctor *[doctors.size() + 1];

        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (d && myStrEqCI(d->getSpecialization(), spec))
            {
                matched[mCount] = d;
                mCount++;
            }
        }

        delete[] spec;

        if (mCount == 0)
        {
            cout << "No doctors available for that specialization.\n";
            delete[] matched;
            return;
        }

        cout << "\nAvailable doctors:\n";
        for (int i = 0; i < mCount; i++)
        {
            char *fee = new char[32];
            floatToStr(matched[i]->getFee(), fee);
            cout << "  ID:" << matched[i]->getId()
                 << "  " << matched[i]->getName()
                 << "  Fee: PKR " << fee << "\n";
            delete[] fee;
        }

        cout << "Enter Doctor ID: ";
        int docId = readInt();
        Doctor *doc = doctors.findById(docId);

        if (!doc)
        {
            cout << "Doctor not found.\n";
            delete[] matched;
            return;
        }

        delete[] matched;

        // validate date — up to 3 attempts
        char *date = new char[12];
        int dateAttempt = 0;

        while (true)
        {
            cout << "Enter date (DD-MM-YYYY): ";
            readLine(date, 12);

            if (Validator::isValidDate(date))
                break;

            cout << "Invalid date. Use format DD-MM-YYYY.\n";
            dateAttempt++;

            if (dateAttempt >= 3)
            {
                cout << "Too many invalid attempts.\n";
                delete[] date;
                return;
            }
        }

        // these are string literals — pointers into read-only memory, not arrays
        const char *allSlots[8];
        allSlots[0] = "09:00";
        allSlots[1] = "10:00";
        allSlots[2] = "11:00";
        allSlots[3] = "12:00";
        allSlots[4] = "13:00";
        allSlots[5] = "14:00";
        allSlots[6] = "15:00";
        allSlots[7] = "16:00";

        char *slot = new char[8];

        while (true)
        {
            cout << "Available slots for Dr." << doc->getName()
                 << " on " << date << ":\n";

            for (int s = 0; s < 8; s++)
            {
                bool taken = false;
                for (int i = 0; i < appointments.size(); i++)
                {
                    Appointment *a = appointments.getAll()[i];
                    if (!a)
                        continue;

                    if (a->getDoctorId() == docId &&
                        myStrcmp(a->getDate(), date) == 0 &&
                        myStrcmp(a->getTimeSlot(), allSlots[s]) == 0 &&
                        myStrcmp(a->getStatus(), "cancelled") != 0)
                    {
                        taken = true;
                        break;
                    }
                }
                if (!taken)
                    cout << "  " << allSlots[s] << "\n";
            }

            cout << "Enter time slot (e.g. 09:00): ";
            readLine(slot, 8);

            if (!Validator::isValidTimeSlot(slot))
            {
                cout << "Invalid slot.\n";
                continue;
            }

            // check if slot is already taken
            bool slotTaken = false;
            for (int i = 0; i < appointments.size(); i++)
            {
                Appointment *a = appointments.getAll()[i];
                if (!a)
                    continue;

                if (a->getDoctorId() == docId &&
                    myStrcmp(a->getDate(), date) == 0 &&
                    myStrcmp(a->getTimeSlot(), slot) == 0 &&
                    myStrcmp(a->getStatus(), "cancelled") != 0)
                {
                    slotTaken = true;
                    break;
                }
            }

            if (slotTaken)
            {
                try
                {
                    throw SlotUnavailableException(slot);
                }
                catch (SlotUnavailableException &e)
                {
                    cout << e.what() << "\n";
                }
                continue;
            }

            // create appointment — no fee deducted here, patient pays via payBill
            int newApptId = appointments.maxId() + 1;
            char *today = new char[12];
            Validator::getTodayStr(today);

            Appointment *appt = new Appointment(newApptId, pat->getId(),
                                                docId, date, slot, "pending");
            appointments.add(appt);
            FileHandler::appendAppointment(*appt);

            int newBillId = bills.maxId() + 1;
            Bill *bill = new Bill(newBillId, pat->getId(), newApptId,
                                  doc->getFee(), "unpaid", today);
            bills.add(bill);
            FileHandler::appendBill(*bill);

            char *idStr = new char[16];
            intToStr(newApptId, idStr);
            cout << "Appointment booked successfully. Appointment ID: " << idStr << "\n";
            delete[] idStr;
            delete[] today;
            delete[] slot;
            delete[] date;
            return;
        }
    }

    void cancelAppointment(Patient *pat)
    {
        int pc = 0;
        Appointment **pending = new Appointment *[appointments.size() + 1];

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pat->getId() &&
                myStrcmp(a->getStatus(), "pending") == 0)
            {
                pending[pc] = a;
                pc++;
            }
        }

        if (pc == 0)
        {
            cout << "You have no pending appointments.\n";
            delete[] pending;
            return;
        }

        cout << "\nPending appointments:\n";
        cout << "ID     Doctor Name               Date         Slot\n";

        for (int i = 0; i < pc; i++)
        {
            cout << pending[i]->getId() << "  "
                 << getDoctorName(pending[i]->getDoctorId()) << "  "
                 << pending[i]->getDate() << "  "
                 << pending[i]->getTimeSlot() << "\n";
        }

        cout << "Enter Appointment ID to cancel: ";
        int apptId = readInt();

        Appointment *target = nullptr;
        for (int i = 0; i < pc; i++)
        {
            if (pending[i]->getId() == apptId)
            {
                target = pending[i];
                break;
            }
        }

        delete[] pending;

        if (!target)
        {
            cout << "Invalid appointment ID.\n";
            return;
        }

        target->setStatus("cancelled");
        FileHandler::saveAllAppointments(appointments);

        // cancel the corresponding unpaid bill
        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getAppointmentId() == apptId)
            {
                b->setStatus("cancelled");
                break;
            }
        }
        FileHandler::saveAllBills(bills);

        cout << "Appointment cancelled successfully.\n";
    }

    void viewMyAppointments(Patient *pat)
    {
        int n = 0;
        Appointment **arr = new Appointment *[appointments.size() + 1];

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pat->getId())
            {
                arr[n] = a;
                n++;
            }
        }

        if (n == 0)
        {
            cout << "No appointments found.\n";
            delete[] arr;
            return;
        }

        sortAppointmentsAsc(arr, n);

        cout << "\nID     Doctor Name               Specialization    Date         Slot     Status\n";

        for (int i = 0; i < n; i++)
        {
            Doctor *d = doctors.findById(arr[i]->getDoctorId());
            cout << arr[i]->getId() << "  "
                 << (d ? d->getName() : "Unknown") << "  "
                 << (d ? d->getSpecialization() : "?") << "  "
                 << arr[i]->getDate() << "  "
                 << arr[i]->getTimeSlot() << "  "
                 << arr[i]->getStatus() << "\n";
        }

        delete[] arr;
    }

    void viewMyMedicalRecords(Patient *pat)
    {
        int n = 0;
        Prescription **arr = new Prescription *[prescriptions.size() + 1];

        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getPatientId() == pat->getId())
            {
                arr[n] = rx;
                n++;
            }
        }

        if (n == 0)
        {
            cout << "No medical records found.\n";
            delete[] arr;
            return;
        }

        sortPrescriptionsDesc(arr, n);

        cout << "\nDate         Doctor Name           Medicines                                Notes\n";

        for (int i = 0; i < n; i++)
        {
            cout << arr[i]->getDate() << "  "
                 << getDoctorName(arr[i]->getDoctorId()) << "  "
                 << arr[i]->getMedicines() << "  "
                 << arr[i]->getNotes() << "\n";
        }

        delete[] arr;
    }

    void viewMyBills(Patient *pat)
    {
        float totalUnpaid = 0;
        bool any = false;

        cout << "\nBillID  ApptID  Amount      Status      Date\n";

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (!b || b->getPatientId() != pat->getId())
                continue;

            char *amt = new char[32];
            floatToStr(b->getAmount(), amt);

            cout << b->getId() << "  "
                 << b->getAppointmentId() << "  PKR "
                 << amt << "  "
                 << b->getStatus() << "  "
                 << b->getDate() << "\n";

            delete[] amt;

            if (myStrcmp(b->getStatus(), "unpaid") == 0)
                totalUnpaid += b->getAmount();

            any = true;
        }

        if (!any)
        {
            cout << "No bills found.\n";
            return;
        }

        char *tot = new char[32];
        floatToStr(totalUnpaid, tot);
        cout << "\nTotal outstanding: PKR " << tot << "\n";
        delete[] tot;
    }

    void payBill(Patient *pat)
    {
        int uc = 0;
        Bill **unpaid = new Bill *[bills.size() + 1];

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pat->getId() &&
                myStrcmp(b->getStatus(), "unpaid") == 0)
            {
                unpaid[uc] = b;
                uc++;
            }
        }

        if (uc == 0)
        {
            cout << "No unpaid bills.\n";
            delete[] unpaid;
            return;
        }

        cout << "\nUnpaid bills:\n";
        for (int i = 0; i < uc; i++)
        {
            char *amt = new char[32];
            floatToStr(unpaid[i]->getAmount(), amt);
            cout << "  Bill ID:" << unpaid[i]->getId()
                 << "  Appt:" << unpaid[i]->getAppointmentId()
                 << "  PKR " << amt
                 << "  Date:" << unpaid[i]->getDate() << "\n";
            delete[] amt;
        }

        cout << "Enter Bill ID to pay: ";
        int billId = readInt();
        Bill *target = nullptr;

        for (int i = 0; i < uc; i++)
        {
            if (unpaid[i]->getId() == billId)
            {
                target = unpaid[i];
                break;
            }
        }

        delete[] unpaid;

        if (!target)
        {
            cout << "Invalid bill ID.\n";
            return;
        }

        if (pat->getBalance() < target->getAmount())
        {
            try
            {
                throw InsufficientFundsException(target->getAmount(), pat->getBalance());
            }
            catch (InsufficientFundsException &e)
            {
                cout << e.what() << "\n";
                return;
            }
        }

        *pat -= target->getAmount();
        target->setStatus("paid");
        FileHandler::saveAllBills(bills);
        FileHandler::saveAllPatients(patients);

        char *bal = new char[32];
        floatToStr(pat->getBalance(), bal);
        cout << "Bill paid successfully. Remaining balance: PKR " << bal << "\n";
        delete[] bal;
    }

    void topUpBalance(Patient *pat)
    {
        int attempts = 0;

        while (attempts < 3)
        {
            cout << "Enter amount to add (PKR): ";
            char *buf = new char[32];
            readLine(buf, 32);

            try
            {
                if (!Validator::isPositiveFloat(buf))
                {
                    delete[] buf;
                    throw InvalidInputException("Amount must be a positive number.");
                }

                float amt = strToFloat(buf);
                delete[] buf;

                *pat += amt;
                FileHandler::saveAllPatients(patients);

                char *bal = new char[32];
                floatToStr(pat->getBalance(), bal);
                cout << "Balance updated. New balance: PKR " << bal << "\n";
                delete[] bal;
                return;
            }
            catch (InvalidInputException &e)
            {
                cout << e.what() << "\n";
                attempts++;
            }
        }

        cout << "Too many invalid attempts.\n";
    }

    // ── DOCTOR ACTIONS ────────────────────────────────────────────────────────

    void viewTodayAppointments(Doctor *doc)
    {
        char *today = new char[12];
        Validator::getTodayStr(today);

        int n = 0;
        Appointment **arr = new Appointment *[appointments.size() + 1];

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getDoctorId() == doc->getId() &&
                myStrcmp(a->getDate(), today) == 0)
            {
                arr[n] = a;
                n++;
            }
        }

        if (n == 0)
        {
            cout << "No appointments scheduled for today.\n";
            delete[] today;
            delete[] arr;
            return;
        }

        // sort by time slot ascending
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (myStrcmp(arr[j]->getTimeSlot(), arr[j + 1]->getTimeSlot()) > 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }

        cout << "\nID     Patient Name          Slot     Status\n";
        for (int i = 0; i < n; i++)
        {
            cout << arr[i]->getId() << "  "
                 << getPatientName(arr[i]->getPatientId()) << "  "
                 << arr[i]->getTimeSlot() << "  "
                 << arr[i]->getStatus() << "\n";
        }

        delete[] today;
        delete[] arr;
    }

    void markAppointmentComplete(Doctor *doc)
    {
        char *today = new char[12];
        Validator::getTodayStr(today);

        cout << "Enter Appointment ID: ";
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);

        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "pending") != 0 ||
            myStrcmp(a->getDate(), today) != 0)
        {
            cout << "Invalid appointment ID.\n";
            delete[] today;
            return;
        }

        a->setStatus("completed");
        FileHandler::saveAllAppointments(appointments);
        cout << "Appointment marked as completed.\n";

        delete[] today;
    }

    void markAppointmentNoShow(Doctor *doc)
    {
        char *today = new char[12];
        Validator::getTodayStr(today);

        cout << "Enter Appointment ID: ";
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);

        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "pending") != 0 ||
            myStrcmp(a->getDate(), today) != 0)
        {
            cout << "Invalid appointment ID.\n";
            delete[] today;
            return;
        }

        a->setStatus("noshow");
        FileHandler::saveAllAppointments(appointments);

        // cancel the bill — no refund since patient never paid
        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getAppointmentId() == apptId)
            {
                b->setStatus("cancelled");
                break;
            }
        }

        FileHandler::saveAllBills(bills);
        cout << "Appointment marked as no-show.\n";

        delete[] today;
    }

    void writePrescription(Doctor *doc)
    {
        cout << "Enter Appointment ID: ";
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);

        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "completed") != 0)
        {
            cout << "Invalid appointment.\n";
            return;
        }

        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getAppointmentId() == apptId)
            {
                cout << "Prescription already written for this appointment.\n";
                return;
            }
        }

        char *med = new char[500];
        char *notes = new char[300];

        cout << "Enter medicines (format: MedicineName Dosage;...): ";
        readLine(med, 500);
        cout << "Enter notes (max 300 chars): ";
        readLine(notes, 300);

        int newId = prescriptions.maxId() + 1;
        Prescription *rx = new Prescription(newId, apptId,
                                            a->getPatientId(), doc->getId(),
                                            a->getDate(), med, notes);
        prescriptions.add(rx);
        FileHandler::appendPrescription(*rx);

        cout << "Prescription saved.\n";

        delete[] med;
        delete[] notes;
    }

    void viewPatientHistory(Doctor *doc)
    {
        cout << "Enter Patient ID: ";
        int pid = readInt();
        Patient *pat = patients.findById(pid);

        if (!pat)
        {
            cout << "Access denied. You can only view records of your own patients.\n";
            return;
        }

        bool found = false;
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid &&
                a->getDoctorId() == doc->getId() &&
                myStrcmp(a->getStatus(), "completed") == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            cout << "Access denied. You can only view records of your own patients.\n";
            return;
        }

        int n = 0;
        Prescription **arr = new Prescription *[prescriptions.size() + 1];

        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getPatientId() == pid && rx->getDoctorId() == doc->getId())
            {
                arr[n] = rx;
                n++;
            }
        }

        if (n == 0)
        {
            cout << "No records found.\n";
            delete[] arr;
            return;
        }

        sortPrescriptionsDesc(arr, n);

        for (int i = 0; i < n; i++)
            cout << *arr[i] << "\n";

        delete[] arr;
    }

    // ── ADMIN ACTIONS ─────────────────────────────────────────────────────────

    void addDoctor()
    {
        char *nm = new char[50];
        char *spec = new char[50];
        char *ct = new char[16];
        char *pw = new char[50];
        char *fee = new char[32];

        cout << "Name: ";
        readLine(nm, 50);
        cout << "Specialization: ";
        readLine(spec, 50);

        while (true)
        {
            cout << "Contact (11 digits): ";
            readLine(ct, 16);
            if (Validator::isValidContact(ct))
                break;
            cout << "Invalid contact number.\n";
        }
        while (true)
        {
            cout << "Password (min 6 chars): ";
            readLine(pw, 50);
            if (Validator::isValidPassword(pw))
                break;
            cout << "Password too short.\n";
        }
        while (true)
        {
            cout << "Consultation fee: ";
            readLine(fee, 32);
            if (Validator::isPositiveFloat(fee))
                break;
            cout << "Invalid fee.\n";
        }

        int newId = doctors.maxId() + 1;
        Doctor *doc = new Doctor(newId, nm, spec, ct, pw, strToFloat(fee));
        doctors.add(doc);
        FileHandler::appendDoctor(*doc);

        char *idStr = new char[16];
        intToStr(newId, idStr);
        cout << "Doctor added successfully. ID: " << idStr << "\n";
        delete[] idStr;

        delete[] nm;
        delete[] spec;
        delete[] ct;
        delete[] pw;
        delete[] fee;
    }

    void removeDoctor()
    {
        cout << "\nAll Doctors:\n";
        cout << "ID   Name                      Specialization   Fee\n";

        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (!d)
                continue;

            char *fee = new char[32];
            floatToStr(d->getFee(), fee);
            cout << d->getId() << "  "
                 << d->getName() << "  "
                 << d->getSpecialization() << "  PKR "
                 << fee << "\n";
            delete[] fee;
        }

        cout << "Enter Doctor ID to remove: ";
        int docId = readInt();

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getDoctorId() == docId &&
                myStrcmp(a->getStatus(), "pending") == 0)
            {
                cout << "Cannot remove doctor with pending appointments. Cancel or reassign them first.\n";
                return;
            }
        }

        Doctor *d = doctors.findById(docId);
        if (!d)
        {
            cout << "Doctor not found.\n";
            return;
        }

        doctors.removeById(docId);
        delete d;
        FileHandler::saveAllDoctors(doctors);
        cout << "Doctor removed.\n";
    }

    void viewAllPatients()
    {
        cout << "\nID   Name                 Age  Gender  Contact         Balance    UnpaidBills\n";

        for (int i = 0; i < patients.size(); i++)
        {
            Patient *p = patients.getAll()[i];
            if (!p)
                continue;

            int unpaid = 0;
            for (int j = 0; j < bills.size(); j++)
            {
                Bill *b = bills.getAll()[j];
                if (b && b->getPatientId() == p->getId() &&
                    myStrcmp(b->getStatus(), "unpaid") == 0)
                    unpaid++;
            }

            char *bal = new char[32];
            floatToStr(p->getBalance(), bal);
            cout << p->getId() << "  "
                 << p->getName() << "  "
                 << p->getAge() << "  "
                 << p->getGender() << "  "
                 << p->getContact() << "  PKR "
                 << bal << "  "
                 << unpaid << "\n";
            delete[] bal;
        }
    }

    void viewAllDoctors()
    {
        cout << "\nID   Name                      Specialization   Contact         Fee\n";

        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (!d)
                continue;

            char *fee = new char[32];
            floatToStr(d->getFee(), fee);
            cout << d->getId() << "  "
                 << d->getName() << "  "
                 << d->getSpecialization() << "  "
                 << d->getContact() << "  PKR "
                 << fee << "\n";
            delete[] fee;
        }
    }

    void viewAllAppointments()
    {
        int n = 0;
        Appointment **arr = new Appointment *[appointments.size() + 1];

        for (int i = 0; i < appointments.size(); i++)
        {
            if (appointments.getAll()[i])
            {
                arr[n] = appointments.getAll()[i];
                n++;
            }
        }

        sortAppointmentsDesc(arr, n);

        cout << "\nID     Patient               Doctor                Date         Slot     Status\n";

        for (int i = 0; i < n; i++)
        {
            cout << arr[i]->getId() << "  "
                 << getPatientName(arr[i]->getPatientId()) << "  "
                 << getDoctorName(arr[i]->getDoctorId()) << "  "
                 << arr[i]->getDate() << "  "
                 << arr[i]->getTimeSlot() << "  "
                 << arr[i]->getStatus() << "\n";
        }

        delete[] arr;
    }

    void viewUnpaidBills()
    {
        char *today = new char[12];
        Validator::getTodayStr(today);

        cout << "\nBillID  Patient               Amount      Date\n";

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (!b || myStrcmp(b->getStatus(), "unpaid") != 0)
                continue;

            char *amt = new char[32];
            char *dateCol = new char[30];

            floatToStr(b->getAmount(), amt);
            myStrcpy(dateCol, b->getDate());

            double diff = Validator::daysBetween(b->getDate(), today);
            if (diff > 7.0)
                myStrcat(dateCol, " [OVERDUE]");

            cout << b->getId() << "  "
                 << getPatientName(b->getPatientId()) << "  PKR "
                 << amt << "  " << dateCol << "\n";

            delete[] amt;
            delete[] dateCol;
        }

        delete[] today;
    }

    void dischargePatient()
    {
        cout << "Enter Patient ID: ";
        int pid = readInt();
        Patient *p = patients.findById(pid);

        if (!p)
        {
            cout << "Patient not found.\n";
            return;
        }

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pid &&
                myStrcmp(b->getStatus(), "unpaid") == 0)
            {
                cout << "Cannot discharge patient with unpaid bills.\n";
                return;
            }
        }

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid &&
                myStrcmp(a->getStatus(), "pending") == 0)
            {
                cout << "Cannot discharge patient with pending appointments.\n";
                return;
            }
        }

        FileHandler::dischargePatient(pid, patients, appointments, bills, prescriptions);

        for (int i = bills.size() - 1; i >= 0; i--)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pid)
            {
                bills.removeById(b->getId());
                delete b;
            }
        }

        for (int i = prescriptions.size() - 1; i >= 0; i--)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getPatientId() == pid)
            {
                prescriptions.removeById(rx->getId());
                delete rx;
            }
        }

        for (int i = appointments.size() - 1; i >= 0; i--)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid)
            {
                appointments.removeById(a->getId());
                delete a;
            }
        }

        patients.removeById(pid);
        delete p;

        FileHandler::saveAllPatients(patients);
        FileHandler::saveAllAppointments(appointments);
        FileHandler::saveAllBills(bills);
        FileHandler::saveAllPrescriptions(prescriptions);
        cout << "Patient discharged and archived successfully.\n";
    }

    void generateDailyReport()
    {
        char *today = new char[12];
        Validator::getTodayStr(today);

        int total = 0, pending = 0, completed = 0, noshow = 0, cancelled = 0;
        float revenue = 0;

        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (!a || myStrcmp(a->getDate(), today) != 0)
                continue;

            total++;
            if (myStrcmp(a->getStatus(), "pending") == 0)
                pending++;
            else if (myStrcmp(a->getStatus(), "completed") == 0)
                completed++;
            else if (myStrcmp(a->getStatus(), "noshow") == 0)
                noshow++;
            else if (myStrcmp(a->getStatus(), "cancelled") == 0)
                cancelled++;
        }

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && myStrcmp(b->getDate(), today) == 0 &&
                myStrcmp(b->getStatus(), "paid") == 0)
                revenue += b->getAmount();
        }

        char *revStr = new char[32];
        floatToStr(revenue, revStr);

        cout << "\n=== Daily Report for " << today << " ===\n";
        cout << "Total appointments today: " << total
             << " (Pending:" << pending
             << " Completed:" << completed
             << " No-show:" << noshow
             << " Cancelled:" << cancelled << ")\n";
        cout << "Revenue collected today (paid bills): PKR " << revStr << "\n";

        delete[] revStr;

        cout << "\nPatients with outstanding unpaid bills:\n";
        cout << "Patient Name         Total Owed\n";

        for (int i = 0; i < patients.size(); i++)
        {
            Patient *p = patients.getAll()[i];
            if (!p)
                continue;

            float owed = 0;
            for (int j = 0; j < bills.size(); j++)
            {
                Bill *b = bills.getAll()[j];
                if (b && b->getPatientId() == p->getId() &&
                    myStrcmp(b->getStatus(), "unpaid") == 0)
                    owed += b->getAmount();
            }

            if (owed > 0)
            {
                char *owedStr = new char[32];
                floatToStr(owed, owedStr);
                cout << p->getName() << "  PKR " << owedStr << "\n";
                delete[] owedStr;
            }
        }

        cout << "\nDoctor-wise summary for today:\n";
        cout << "Doctor Name               Completed  Pending  No-show\n";

        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (!d)
                continue;

            int dc = 0, dp = 0, dn = 0;
            for (int j = 0; j < appointments.size(); j++)
            {
                Appointment *a = appointments.getAll()[j];
                if (!a || a->getDoctorId() != d->getId() ||
                    myStrcmp(a->getDate(), today) != 0)
                    continue;

                if (myStrcmp(a->getStatus(), "completed") == 0)
                    dc++;
                else if (myStrcmp(a->getStatus(), "pending") == 0)
                    dp++;
                else if (myStrcmp(a->getStatus(), "noshow") == 0)
                    dn++;
            }

            if (dc + dp + dn > 0)
                cout << d->getName() << "  " << dc << "  " << dp << "  " << dn << "\n";
        }

        delete[] today;
    }

public:
    void loadAll()
    {
        FileHandler::loadPatients(patients);
        FileHandler::loadDoctors(doctors);
        FileHandler::loadAppointments(appointments);
        FileHandler::loadBills(bills);
        FileHandler::loadPrescriptions(prescriptions);

        try
        {
            FileHandler::loadAdmin(admin);
        }
        catch (FileNotFoundException &e)
        {
            cout << "Warning: " << e.what() << "\n";
        }
    }

    void freeAll()
    {
        patients.clear();
        doctors.clear();
        appointments.clear();
        bills.clear();
        prescriptions.clear();
    }

    void runPatientMenu(Patient *pat)
    {
        int choice;
        do
        {
            pat->displayMenu();
            choice = readInt();

            switch (choice)
            {
            case 1:
                bookAppointment(pat);
                break;
            case 2:
                cancelAppointment(pat);
                break;
            case 3:
                viewMyAppointments(pat);
                break;
            case 4:
                viewMyMedicalRecords(pat);
                break;
            case 5:
                viewMyBills(pat);
                break;
            case 6:
                payBill(pat);
                break;
            case 7:
                topUpBalance(pat);
                break;
            case 8:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice.\n";
            }
        } while (choice != 8);
    }

    void runDoctorMenu(Doctor *doc)
    {
        int choice;
        do
        {
            doc->displayMenu();
            choice = readInt();

            switch (choice)
            {
            case 1:
                viewTodayAppointments(doc);
                break;
            case 2:
                markAppointmentComplete(doc);
                break;
            case 3:
                markAppointmentNoShow(doc);
                break;
            case 4:
                writePrescription(doc);
                break;
            case 5:
                viewPatientHistory(doc);
                break;
            case 6:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice.\n";
            }
        } while (choice != 6);
    }

    void runAdminMenu()
    {
        int choice;
        do
        {
            admin.displayMenu();
            choice = readInt();

            switch (choice)
            {
            case 1:
                addDoctor();
                break;
            case 2:
                removeDoctor();
                break;
            case 3:
                viewAllPatients();
                break;
            case 4:
                viewAllDoctors();
                break;
            case 5:
                viewAllAppointments();
                break;
            case 6:
                viewUnpaidBills();
                break;
            case 7:
                dischargePatient();
                break;
            case 8:
                FileHandler::printSecurityLog();
                break;
            case 9:
                generateDailyReport();
                break;
            case 10:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice.\n";
            }
        } while (choice != 10);
    }

    void run()
    {
        cout << "Welcome to MediCore Hospital Management System\n";
        cout << "===============================================\n";

        int roleChoice;
        do
        {
            cout << "\nLogin as:\n1. Patient\n2. Doctor\n3. Admin\n4. Exit\nChoice: ";
            roleChoice = readInt();

            if (roleChoice == 1)
            {
                int pid = 0;
                if (loginWithLockout("Patient", &pid))
                {
                    Patient *p = patients.findById(pid);
                    if (p)
                        runPatientMenu(p);
                }
            }
            else if (roleChoice == 2)
            {
                int did = 0;
                if (loginWithLockout("Doctor", &did))
                {
                    Doctor *d = doctors.findById(did);
                    if (d)
                        runDoctorMenu(d);
                }
            }
            else if (roleChoice == 3)
            {
                int aid = 0;
                if (loginWithLockout("Admin", &aid))
                    runAdminMenu();
            }
            else if (roleChoice != 4)
            {
                cout << "Invalid choice.\n";
            }
        } while (roleChoice != 4);

        cout << "Goodbye!\n";
    }
};
