## **Overview**

MediCore is a hospital management application for doctor, patient, and admin users.

---

## **Prequisites**

### Dependencies:

- raylib

### VS Code Setup

You need to setup c_cpp_properties.json and tasks.json for your vscode to point to compiler path, libs and include folder.

---

## **Command to run**

`g++ -std=c++17 gui_main.cpp src/Patient.cpp src/Doctor.cpp src/Admin.cpp src/Appointment.cpp src/Bill.cpp src/Prescription.cpp src/Validator.cpp src/HospitalException.cpp src/FileNotFoundException.cpp src/InsufficientFundsException.cpp src/InvalidInputException.cpp src/SlotUnavailableException.cpp src/strutils.cpp src/FileHandler.cpp -lraylib -lopengl32 -lgdi32 -lwinmm -o medicore.exe`

---

## **Showcase**

![[main_menu.png]]

![[patient_menu.png]]

---
