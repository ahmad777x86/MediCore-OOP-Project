#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "FileNotFoundException.h"
#include "Validator.h"

// FileHandler is the ONLY class allowed to read from and write to files.
// All functions are static so you can call them without creating an object.
// Example: FileHandler::loadPatients(myStorage);
class FileHandler
{
public:
    // reads all patients from patients.txt and adds them to the storage
    static void loadPatients(Storage<Patient> &store);

    // reads all doctors from doctors.txt and adds them to the storage
    static void loadDoctors(Storage<Doctor> &store);

    // reads the admin account from admin.txt
    // throws FileNotFoundException if admin.txt does not exist
    static void loadAdmin(Admin &admin);

    // reads all appointments from appointments.txt and adds them to the storage
    static void loadAppointments(Storage<Appointment> &store);

    // reads all bills from bills.txt and adds them to the storage
    static void loadBills(Storage<Bill> &store);

    // reads all prescriptions from prescriptions.txt and adds them to the storage
    static void loadPrescriptions(Storage<Prescription> &store);

    // overwrites patients.txt with all patients currently in storage
    static void saveAllPatients(Storage<Patient> &store);

    // overwrites doctors.txt with all doctors currently in storage
    static void saveAllDoctors(Storage<Doctor> &store);

    // overwrites appointments.txt with all appointments currently in storage
    static void saveAllAppointments(Storage<Appointment> &store);

    // overwrites bills.txt with all bills currently in storage
    static void saveAllBills(Storage<Bill> &store);

    // overwrites prescriptions.txt with all prescriptions currently in storage
    static void saveAllPrescriptions(Storage<Prescription> &store);

    // adds one new appointment to the end of appointments.txt
    static void appendAppointment(const Appointment &a);

    // adds one new bill to the end of bills.txt
    static void appendBill(const Bill &b);

    // adds one new doctor to the end of doctors.txt
    static void appendDoctor(const Doctor &d);

    // adds one new prescription to the end of prescriptions.txt
    static void appendPrescription(const Prescription &rx);

    // adds one new patient to the end of patients.txt
    static void appendPatient(const Patient &p);

    // writes one login attempt to security_log.txt
    // role is "Patient", "Doctor", or "Admin"
    // enteredId is whatever the user typed as their ID
    // result is either "SUCCESS" or "FAILED"
    static void logSecurityEvent(const char *role, const char *enteredId, const char *result);

    // prints every line of security_log.txt to the console
    static void printSecurityLog();

    // reads the entire security_log.txt into buf (used for GUI display)
    static void readSecurityLog(char *buf, int maxLen);

    // copies a patient's full record into discharged.txt
    // includes their appointments, bills, and prescriptions
    static void dischargePatient(int patientId,
                                 Storage<Patient> &patients,
                                 Storage<Appointment> &appointments,
                                 Storage<Bill> &bills,
                                 Storage<Prescription> &prescriptions);
};
