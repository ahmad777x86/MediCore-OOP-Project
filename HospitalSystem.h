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
#include <cstdio>
#include <iostream>

// ── HospitalSystem ────────────────────────────────────────────────────────────
// Orchestrates all menus and business logic.
class HospitalSystem
{
private:
    Storage<Patient> patients;
    Storage<Doctor> doctors;
    Storage<Appointment> appointments;
    Storage<Bill> bills;
    Storage<Prescription> prescriptions;
    Admin admin;

    // ── helpers ───────────────────────────────────────────────────────────────
    void clearInput()
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    int readInt()
    {
        char buf[32];
        int i = 0, c;
        while ((c = getchar()) != '\n' && c != EOF && i < 31)
            buf[i++] = (char)c;
        buf[i] = '\0';
        return strToInt(buf);
    }

    float readFloat()
    {
        char buf[32];
        int i = 0, c;
        while ((c = getchar()) != '\n' && c != EOF && i < 31)
            buf[i++] = (char)c;
        buf[i] = '\0';
        return strToFloat(buf);
    }

    void readLine(char *dst, int maxLen)
    {
        int i = 0, c;
        while ((c = getchar()) != '\n' && c != EOF && i < maxLen - 1)
            dst[i++] = (char)c;
        dst[i] = '\0';
    }

    // sort appointments by date ascending (bubble sort — no library)
    void sortAppointmentsAsc(Appointment **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) > 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
    }

    // sort appointments by date descending
    void sortAppointmentsDesc(Appointment **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
    }

    // sort prescriptions by date descending
    void sortPrescriptionsDesc(Prescription **arr, int n)
    {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                {
                    Prescription *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
    }

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

    // login with lockout
    bool loginWithLockout(const char *role, int *outId)
    {
        int attempts = 0;
        while (attempts < 3)
        {
            char idBuf[32], pwBuf[64];
            printf("Enter ID: ");
            readLine(idBuf, 32);
            printf("Enter Password: ");
            readLine(pwBuf, 64);

            bool found = false;
            if (myStrcmp(role, "Patient") == 0)
            {
                for (int i = 0; i < patients.size(); i++)
                {
                    Patient *p = patients.getAll()[i];
                    char tmp[16];
                    intToStr(p->getId(), tmp);
                    if (myStrcmp(tmp, idBuf) == 0 && p->checkPassword(pwBuf))
                    {
                        *outId = p->getId();
                        found = true;
                        break;
                    }
                }
            }
            else if (myStrcmp(role, "Doctor") == 0)
            {
                for (int i = 0; i < doctors.size(); i++)
                {
                    Doctor *d = doctors.getAll()[i];
                    char tmp[16];
                    intToStr(d->getId(), tmp);
                    if (myStrcmp(tmp, idBuf) == 0 && d->checkPassword(pwBuf))
                    {
                        *outId = d->getId();
                        found = true;
                        break;
                    }
                }
            }
            else if (myStrcmp(role, "Admin") == 0)
            {
                char tmp[16];
                intToStr(admin.getId(), tmp);
                if (myStrcmp(tmp, idBuf) == 0 && admin.checkPassword(pwBuf))
                {
                    *outId = admin.getId();
                    found = true;
                }
            }

            if (found)
            {
                FileHandler::logSecurityEvent(role, idBuf, "SUCCESS");
                return true;
            }

            attempts++;
            FileHandler::logSecurityEvent(role, idBuf, "FAILED");
            printf("Invalid credentials. Attempts remaining: %d\n", 3 - attempts);
        }
        printf("Account locked. Contact admin.\n");
        return false;
    }

    // ── PATIENT MENU ACTIONS ──────────────────────────────────────────────────
    void bookAppointment(Patient *pat)
    {
        char spec[60];
        printf("Enter specialization to search: ");
        readLine(spec, 60);

        // collect matching doctors
        Doctor *matched[100];
        int mCount = 0;
        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (d && myStrEqCI(d->getSpecialization(), spec))
                matched[mCount++] = d;
        }
        if (mCount == 0)
        {
            printf("No doctors available for that specialization.\n");
            return;
        }

        printf("\nAvailable doctors:\n");
        for (int i = 0; i < mCount; i++)
        {
            char fee[32];
            floatToStr(matched[i]->getFee(), fee);
            printf("  ID:%-4d  %-30s  Fee: PKR %s\n",
                   matched[i]->getId(), matched[i]->getName(), fee);
        }

        printf("Enter Doctor ID: ");
        int docId = readInt();
        Doctor *doc = doctors.findById(docId);
        if (!doc)
        {
            printf("Doctor not found.\n");
            return;
        }

        // validate date with 3 attempts
        char date[12];
        int dateAttempts = 0;
        while (true)
        {
            printf("Enter date (DD-MM-YYYY): ");
            readLine(date, 12);
            if (Validator::isValidDate(date))
                break;
            printf("Invalid date. Use format DD-MM-YYYY.\n");
            if (++dateAttempts >= 3)
            {
                printf("Too many invalid attempts.\n");
                return;
            }
        }

        // display available slots
        const char *allSlots[] = {"09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00"};
        while (true)
        {
            printf("Available slots for Dr.%s on %s:\n", doc->getName(), date);
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
                    printf("  %s\n", allSlots[s]);
            }

            char slot[8];
            printf("Enter time slot (e.g. 09:00): ");
            readLine(slot, 8);
            if (!Validator::isValidTimeSlot(slot))
            {
                printf("Invalid slot.\n");
                continue;
            }
            // check conflict
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
                    printf("%s\n", e.what());
                }
                continue;
            }

            // check balance
            if (pat->getBalance() < doc->getFee())
            {
                try
                {
                    throw InsufficientFundsException(doc->getFee(), pat->getBalance());
                }
                catch (InsufficientFundsException &e)
                {
                    printf("%s\n", e.what());
                    return;
                }
            }

            // book
            *pat -= doc->getFee();
            int newApptId = appointments.maxId() + 1;
            char today[12];
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

            FileHandler::saveAllPatients(patients);
            printf("Appointment booked successfully. Appointment ID: %d\n", newApptId);
            return;
        }
    }

    void cancelAppointment(Patient *pat)
    {
        // collect pending appointments for this patient
        Appointment *pending[50];
        int pc = 0;
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pat->getId() &&
                myStrcmp(a->getStatus(), "pending") == 0)
                pending[pc++] = a;
        }
        if (pc == 0)
        {
            printf("You have no pending appointments.\n");
            return;
        }

        printf("\nPending appointments:\n");
        printf("%-6s %-25s %-12s %-8s\n", "ID", "Doctor Name", "Date", "Slot");
        for (int i = 0; i < pc; i++)
        {
            printf("%-6d %-25s %-12s %-8s\n",
                   pending[i]->getId(), getDoctorName(pending[i]->getDoctorId()),
                   pending[i]->getDate(), pending[i]->getTimeSlot());
        }

        printf("Enter Appointment ID to cancel: ");
        int apptId = readInt();

        Appointment *target = nullptr;
        for (int i = 0; i < pc; i++)
            if (pending[i]->getId() == apptId)
            {
                target = pending[i];
                break;
            }
        if (!target)
        {
            printf("Invalid appointment ID.\n");
            return;
        }

        // find doctor fee
        Doctor *doc = doctors.findById(target->getDoctorId());
        float refund = doc ? doc->getFee() : 0.0f;

        target->setStatus("cancelled");
        FileHandler::saveAllAppointments(appointments);

        *pat += refund;
        FileHandler::saveAllPatients(patients);

        // cancel corresponding bill
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

        char refundStr[32];
        floatToStr(refund, refundStr);
        printf("Appointment cancelled. PKR %s refunded to your balance.\n", refundStr);
    }

    void viewMyAppointments(Patient *pat)
    {
        Appointment *arr[100];
        int n = 0;
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pat->getId())
                arr[n++] = a;
        }
        if (n == 0)
        {
            printf("No appointments found.\n");
            return;
        }
        sortAppointmentsAsc(arr, n);

        printf("\n%-6s %-25s %-15s %-12s %-8s %-12s\n",
               "ID", "Doctor Name", "Specialization", "Date", "Slot", "Status");
        for (int i = 0; i < n; i++)
        {
            Doctor *d = doctors.findById(arr[i]->getDoctorId());
            printf("%-6d %-25s %-15s %-12s %-8s %-12s\n",
                   arr[i]->getId(),
                   d ? d->getName() : "Unknown",
                   d ? d->getSpecialization() : "?",
                   arr[i]->getDate(),
                   arr[i]->getTimeSlot(),
                   arr[i]->getStatus());
        }
    }

    void viewMyMedicalRecords(Patient *pat)
    {
        Prescription *arr[100];
        int n = 0;
        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getPatientId() == pat->getId())
                arr[n++] = rx;
        }
        if (n == 0)
        {
            printf("No medical records found.\n");
            return;
        }
        sortPrescriptionsDesc(arr, n);

        printf("\n%-12s %-20s %-40s %s\n", "Date", "Doctor Name", "Medicines", "Notes");
        for (int i = 0; i < n; i++)
        {
            printf("%-12s %-20s %-40s %s\n",
                   arr[i]->getDate(),
                   getDoctorName(arr[i]->getDoctorId()),
                   arr[i]->getMedicines(),
                   arr[i]->getNotes());
        }
    }

    void viewMyBills(Patient *pat)
    {
        float totalUnpaid = 0;
        bool any = false;
        printf("\n%-6s %-12s %-10s %-10s %-12s\n",
               "BillID", "ApptID", "Amount", "Status", "Date");
        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pat->getId())
            {
                char amt[32];
                floatToStr(b->getAmount(), amt);
                printf("%-6d %-12d PKR %-7s %-10s %-12s\n",
                       b->getId(), b->getAppointmentId(),
                       amt, b->getStatus(), b->getDate());
                if (myStrcmp(b->getStatus(), "unpaid") == 0)
                    totalUnpaid += b->getAmount();
                any = true;
            }
        }
        if (!any)
        {
            printf("No bills found.\n");
            return;
        }
        char tot[32];
        floatToStr(totalUnpaid, tot);
        printf("\nTotal outstanding: PKR %s\n", tot);
    }

    void payBill(Patient *pat)
    {
        Bill *unpaid[50];
        int uc = 0;
        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pat->getId() &&
                myStrcmp(b->getStatus(), "unpaid") == 0)
                unpaid[uc++] = b;
        }
        if (uc == 0)
        {
            printf("No unpaid bills.\n");
            return;
        }

        printf("\nUnpaid bills:\n");
        for (int i = 0; i < uc; i++)
        {
            char amt[32];
            floatToStr(unpaid[i]->getAmount(), amt);
            printf("  Bill ID:%-4d Appt:%-4d PKR %s  Date:%s\n",
                   unpaid[i]->getId(), unpaid[i]->getAppointmentId(),
                   amt, unpaid[i]->getDate());
        }

        printf("Enter Bill ID to pay: ");
        int billId = readInt();
        Bill *target = nullptr;
        for (int i = 0; i < uc; i++)
            if (unpaid[i]->getId() == billId)
            {
                target = unpaid[i];
                break;
            }
        if (!target)
        {
            printf("Invalid bill ID.\n");
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
                printf("%s\n", e.what());
                return;
            }
        }

        *pat -= target->getAmount();
        target->setStatus("paid");
        FileHandler::saveAllBills(bills);
        FileHandler::saveAllPatients(patients);

        char bal[32];
        floatToStr(pat->getBalance(), bal);
        printf("Bill paid successfully. Remaining balance: PKR %s\n", bal);
    }

    void topUpBalance(Patient *pat)
    {
        int attempts = 0;
        while (attempts < 3)
        {
            printf("Enter amount to add (PKR): ");
            char buf[32];
            readLine(buf, 32);
            try
            {
                if (!Validator::isPositiveFloat(buf))
                    throw InvalidInputException("Amount must be a positive number.");
                float amt = strToFloat(buf);
                *pat += amt;
                FileHandler::saveAllPatients(patients);
                char bal[32];
                floatToStr(pat->getBalance(), bal);
                printf("Balance updated. New balance: PKR %s\n", bal);
                return;
            }
            catch (InvalidInputException &e)
            {
                printf("%s\n", e.what());
                attempts++;
            }
        }
        printf("Too many invalid attempts.\n");
    }

    // ── DOCTOR MENU ACTIONS ───────────────────────────────────────────────────
    void viewTodayAppointments(Doctor *doc)
    {
        char today[12];
        Validator::getTodayStr(today);
        Appointment *arr[50];
        int n = 0;
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getDoctorId() == doc->getId() &&
                myStrcmp(a->getDate(), today) == 0)
                arr[n++] = a;
        }
        if (n == 0)
        {
            printf("No appointments scheduled for today.\n");
            return;
        }

        // sort by time slot ascending (simple bubble on strings)
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (myStrcmp(arr[j]->getTimeSlot(), arr[j + 1]->getTimeSlot()) > 0)
                {
                    Appointment *tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }

        printf("\n%-6s %-20s %-8s %-12s\n", "ID", "Patient Name", "Slot", "Status");
        for (int i = 0; i < n; i++)
        {
            printf("%-6d %-20s %-8s %-12s\n",
                   arr[i]->getId(),
                   getPatientName(arr[i]->getPatientId()),
                   arr[i]->getTimeSlot(),
                   arr[i]->getStatus());
        }
    }

    void markAppointmentComplete(Doctor *doc)
    {
        char today[12];
        Validator::getTodayStr(today);
        printf("Enter Appointment ID: ");
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);
        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "pending") != 0 ||
            myStrcmp(a->getDate(), today) != 0)
        {
            printf("Invalid appointment ID.\n");
            return;
        }
        a->setStatus("completed");
        FileHandler::saveAllAppointments(appointments);
        printf("Appointment marked as completed.\n");
    }

    void markAppointmentNoShow(Doctor *doc)
    {
        char today[12];
        Validator::getTodayStr(today);
        printf("Enter Appointment ID: ");
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);
        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "pending") != 0 ||
            myStrcmp(a->getDate(), today) != 0)
        {
            printf("Invalid appointment ID.\n");
            return;
        }
        a->setStatus("noshow");
        FileHandler::saveAllAppointments(appointments);
        // cancel corresponding bill
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
        printf("Appointment marked as no-show.\n");
    }

    void writePrescription(Doctor *doc)
    {
        printf("Enter Appointment ID: ");
        int apptId = readInt();
        Appointment *a = appointments.findById(apptId);
        if (!a || a->getDoctorId() != doc->getId() ||
            myStrcmp(a->getStatus(), "completed") != 0)
        {
            printf("Invalid appointment.\n");
            return;
        }
        // check if prescription already written
        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getAppointmentId() == apptId)
            {
                printf("Prescription already written for this appointment.\n");
                return;
            }
        }
        char med[500];
        char notes[300];
        printf("Enter medicines (format: MedicineName Dosage;...): ");
        readLine(med, 500);
        printf("Enter notes (max 300 chars): ");
        readLine(notes, 300);

        int newId = prescriptions.maxId() + 1;
        Prescription *rx = new Prescription(newId, apptId,
                                            a->getPatientId(), doc->getId(),
                                            a->getDate(), med, notes);
        prescriptions.add(rx);
        FileHandler::appendPrescription(*rx);
        printf("Prescription saved.\n");
    }

    void viewPatientHistory(Doctor *doc)
    {
        printf("Enter Patient ID: ");
        int pid = readInt();
        Patient *pat = patients.findById(pid);
        if (!pat)
        {
            printf("Access denied. You can only view records of your own patients.\n");
            return;
        }

        // check at least one completed appointment with this doctor
        bool found = false;
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid && a->getDoctorId() == doc->getId() &&
                myStrcmp(a->getStatus(), "completed") == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            printf("Access denied. You can only view records of your own patients.\n");
            return;
        }

        Prescription *arr[100];
        int n = 0;
        for (int i = 0; i < prescriptions.size(); i++)
        {
            Prescription *rx = prescriptions.getAll()[i];
            if (rx && rx->getPatientId() == pid && rx->getDoctorId() == doc->getId())
                arr[n++] = rx;
        }
        if (n == 0)
        {
            printf("No records found.\n");
            return;
        }
        sortPrescriptionsDesc(arr, n);
        for (int i = 0; i < n; i++)
            std::cout << *arr[i] << "\n";
    }

    // ── ADMIN MENU ACTIONS ────────────────────────────────────────────────────
    void addDoctor()
    {
        char nm[50], spec[50], ct[16], pw[50], fee[32];
        printf("Name: ");
        readLine(nm, 50);
        printf("Specialization: ");
        readLine(spec, 50);

        while (true)
        {
            printf("Contact (11 digits): ");
            readLine(ct, 16);
            if (Validator::isValidContact(ct))
                break;
            printf("Invalid contact number.\n");
        }
        while (true)
        {
            printf("Password (min 6 chars): ");
            readLine(pw, 50);
            if (Validator::isValidPassword(pw))
                break;
            printf("Password too short.\n");
        }
        while (true)
        {
            printf("Consultation fee: ");
            readLine(fee, 32);
            if (Validator::isPositiveFloat(fee))
                break;
            printf("Invalid fee.\n");
        }

        int newId = doctors.maxId() + 1;
        Doctor *doc = new Doctor(newId, nm, spec, ct, pw, strToFloat(fee));
        doctors.add(doc);
        FileHandler::appendDoctor(*doc);
        printf("Doctor added successfully. ID: %d\n", newId);
    }

    void removeDoctor()
    {
        printf("\nAll Doctors:\n");
        printf("%-4s %-25s %-15s %s\n", "ID", "Name", "Specialization", "Fee");
        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (!d)
                continue;
            char fee[32];
            floatToStr(d->getFee(), fee);
            printf("%-4d %-25s %-15s PKR%s\n",
                   d->getId(), d->getName(), d->getSpecialization(), fee);
        }
        printf("Enter Doctor ID to remove: ");
        int docId = readInt();
        // check pending appointments
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getDoctorId() == docId &&
                myStrcmp(a->getStatus(), "pending") == 0)
            {
                printf("Cannot remove doctor with pending appointments. Cancel or reassign them first.\n");
                return;
            }
        }
        Doctor *d = doctors.findById(docId);
        if (!d)
        {
            printf("Doctor not found.\n");
            return;
        }
        doctors.removeById(docId);
        delete d;
        FileHandler::saveAllDoctors(doctors);
        printf("Doctor removed.\n");
    }

    void viewAllPatients()
    {
        printf("\n%-4s %-20s %-4s %-6s %-14s %-10s %s\n",
               "ID", "Name", "Age", "Gender", "Contact", "Balance", "UnpaidBills");
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
            char bal[32];
            floatToStr(p->getBalance(), bal);
            printf("%-4d %-20s %-4d %-6s %-14s PKR%-7s %d\n",
                   p->getId(), p->getName(), p->getAge(), p->getGender(),
                   p->getContact(), bal, unpaid);
        }
    }

    void viewAllDoctors()
    {
        printf("\n%-4s %-25s %-15s %-14s %s\n",
               "ID", "Name", "Specialization", "Contact", "Fee");
        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            if (!d)
                continue;
            char fee[32];
            floatToStr(d->getFee(), fee);
            printf("%-4d %-25s %-15s %-14s PKR%s\n",
                   d->getId(), d->getName(), d->getSpecialization(),
                   d->getContact(), fee);
        }
    }

    void viewAllAppointments()
    {
        Appointment *arr[200];
        int n = 0;
        for (int i = 0; i < appointments.size(); i++)
            if (appointments.getAll()[i])
                arr[n++] = appointments.getAll()[i];
        sortAppointmentsDesc(arr, n);
        printf("\n%-6s %-20s %-20s %-12s %-8s %s\n",
               "ID", "Patient", "Doctor", "Date", "Slot", "Status");
        for (int i = 0; i < n; i++)
        {
            printf("%-6d %-20s %-20s %-12s %-8s %s\n",
                   arr[i]->getId(),
                   getPatientName(arr[i]->getPatientId()),
                   getDoctorName(arr[i]->getDoctorId()),
                   arr[i]->getDate(), arr[i]->getTimeSlot(), arr[i]->getStatus());
        }
    }

    void viewUnpaidBills()
    {
        char today[12];
        Validator::getTodayStr(today);
        printf("\n%-6s %-20s %-10s %-14s\n", "BillID", "Patient", "Amount", "Date");
        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (!b || myStrcmp(b->getStatus(), "unpaid") != 0)
                continue;
            char amt[32];
            floatToStr(b->getAmount(), amt);
            char dateCol[30];
            myStrcpy(dateCol, b->getDate());
            double diff = Validator::daysBetween(b->getDate(), today);
            if (diff > 7.0)
                myStrcat(dateCol, " [OVERDUE]");
            printf("%-6d %-20s PKR %-7s %s\n",
                   b->getId(), getPatientName(b->getPatientId()), amt, dateCol);
        }
    }

    void dischargePatient()
    {
        printf("Enter Patient ID: ");
        int pid = readInt();
        Patient *p = patients.findById(pid);
        if (!p)
        {
            printf("Patient not found.\n");
            return;
        }

        for (int i = 0; i < bills.size(); i++)
        {
            Bill *b = bills.getAll()[i];
            if (b && b->getPatientId() == pid &&
                myStrcmp(b->getStatus(), "unpaid") == 0)
            {
                printf("Cannot discharge patient with unpaid bills.\n");
                return;
            }
        }
        for (int i = 0; i < appointments.size(); i++)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid &&
                myStrcmp(a->getStatus(), "pending") == 0)
            {
                printf("Cannot discharge patient with pending appointments.\n");
                return;
            }
        }

        FileHandler::dischargePatient(pid, patients, appointments, bills, prescriptions);

        // remove from storage
        patients.removeById(pid);
        delete p;
        // remove appointments, bills, prescriptions for this patient
        for (int i = appointments.size() - 1; i >= 0; i--)
        {
            Appointment *a = appointments.getAll()[i];
            if (a && a->getPatientId() == pid)
            {
                appointments.removeById(a->getId());
                delete a;
                i = appointments.size();
            }
        }
        // simpler: rebuild
        FileHandler::saveAllPatients(patients);
        FileHandler::saveAllAppointments(appointments);
        FileHandler::saveAllBills(bills);
        FileHandler::saveAllPrescriptions(prescriptions);
        printf("Patient discharged and archived successfully.\n");
    }

    void generateDailyReport()
    {
        char today[12];
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

        char revStr[32];
        floatToStr(revenue, revStr);
        printf("\n=== Daily Report for %s ===\n", today);
        printf("Total appointments today: %d (Pending:%d Completed:%d No-show:%d Cancelled:%d)\n",
               total, pending, completed, noshow, cancelled);
        printf("Revenue collected today (paid bills): PKR %s\n", revStr);

        printf("\nPatients with outstanding unpaid bills:\n");
        printf("%-20s %-12s\n", "Patient Name", "Total Owed");
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
                char owedStr[32];
                floatToStr(owed, owedStr);
                printf("%-20s PKR %s\n", p->getName(), owedStr);
            }
        }

        printf("\nDoctor-wise summary for today:\n");
        printf("%-25s %-10s %-8s %s\n", "Doctor Name", "Completed", "Pending", "No-show");
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
                printf("%-25s %-10d %-8d %d\n", d->getName(), dc, dp, dn);
        }
    }

public:
    // ── PUBLIC API ────────────────────────────────────────────────────────────
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
            printf("Warning: %s\n", e.what());
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
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice.\n");
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
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice.\n");
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
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice.\n");
            }
        } while (choice != 10);
    }

    void run()
    {
        printf("Welcome to MediCore Hospital Management System\n");
        printf("===============================================\n");
        int roleChoice;
        do
        {
            printf("\nLogin as:\n1. Patient\n2. Doctor\n3. Admin\n4. Exit\nChoice: ");
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
                printf("Invalid choice.\n");
            }
        } while (roleChoice != 4);
        printf("Goodbye!\n");
    }
};