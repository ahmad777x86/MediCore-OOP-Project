// ============================================================
//  MediCore Hospital Management System  —  Raylib GUI
//  Compile: g++ -std=c++17 gui_main.cpp Patient.cpp Doctor.cpp
//           Admin.cpp Appointment.cpp Bill.cpp Prescription.cpp
//           -lraylib -lm -o medicore_gui
// ============================================================
#include "raylib.h"
#include "HospitalSystem.h"
#include "src/Validator.h"
#include "src/strutils.h"
#include <cstdio>
#include <cstring>

// ── colour palette ────────────────────────────────────────────────────────────
static const Color BG = {10, 14, 30, 255};          // dark navy
static const Color PANEL = {18, 26, 52, 255};       // slightly lighter
static const Color CARD = {25, 38, 75, 255};        // card bg
static const Color ACCENT = {0, 200, 180, 255};     // teal accent
static const Color ACCENT2 = {255, 160, 50, 255};   // amber
static const Color DANGER = {220, 60, 60, 255};     // red
static const Color TEXT_PRI = {230, 240, 255, 255}; // near-white
static const Color TEXT_SEC = {130, 150, 190, 255}; // muted blue
static const Color BTN_HOVER = {0, 220, 200, 255};  // brighter teal
static const Color INPUT_BG = {15, 22, 45, 255};

// ── UI state machine ──
enum class Screen
{
    MAIN_MENU,
    PATIENT_LOGIN,
    PATIENT_MENU,
    DOCTOR_LOGIN,
    DOCTOR_MENU,
    ADMIN_LOGIN,
    ADMIN_MENU,
    // patient sub-screens
    P_BOOK,
    P_CANCEL,
    P_VIEW_APPTS,
    P_VIEW_RECORDS,
    P_VIEW_BILLS,
    P_PAY_BILL,
    P_TOPUP,
    // doctor sub-screens
    D_TODAY,
    D_COMPLETE,
    D_NOSHOW,
    D_PRESCRIBE,
    D_HISTORY,
    // admin sub-screens
    A_ADD_DOC,
    A_REMOVE_DOC,
    A_VIEW_PATIENTS,
    A_VIEW_DOCTORS,
    A_VIEW_APPTS,
    A_UNPAID,
    A_DISCHARGE,
    A_SECURITY,
    A_REPORT,
    // message overlay
    MSG_SCREEN
};

// ── simple text-input box ─────────────────────────────────────────────────────
struct TextInput
{
    char buf[512];
    int len;
    bool active;
    bool isPassword;

    TextInput() : len(0), active(false), isPassword(false) { buf[0] = '\0'; }
    void clear()
    {
        buf[0] = '\0';
        len = 0;
    }
    void handleInput()
    {
        if (!active)
            return;
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && len < 510)
            {
                buf[len++] = (char)key;
                buf[len] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
        {
            buf[--len] = '\0';
        }
    }
    void draw(Rectangle r, const char *label, Font font) const
    {
        DrawRectangleRec(r, INPUT_BG);
        DrawRectangleLinesEx(r, active ? 2 : 1, active ? ACCENT : TEXT_SEC);
        // label above
        DrawTextEx(font, label, {r.x, r.y - 22}, 16, 1, TEXT_SEC);
        // display text
        const char *display = buf;
        char masked[512];
        if (isPassword)
        {
            for (int i = 0; i < len; i++)
                masked[i] = '*';
            masked[len] = '\0';
            display = masked;
        }
        DrawTextEx(font, display, {r.x + 8, r.y + (r.height - 18) / 2}, 18, 1, TEXT_PRI);
    }
};

// ── button helper ─────────────────────────────────────────────────────────────
static bool GuiButton(Rectangle r, const char *text, Font font,
                      Color col = ACCENT, Color hov = BTN_HOVER)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, hover ? hov : col);
    DrawRectangleLinesEx(r, 1, {255, 255, 255, 30});
    Vector2 ts = MeasureTextEx(font, text, 18, 1);
    DrawTextEx(font, text,
               {r.x + (r.width - ts.x) / 2, r.y + (r.height - ts.y) / 2},
               18, 1, {10, 14, 30, 255});
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool DangerButton(Rectangle r, const char *text, Font font)
{
    return GuiButton(r, text, font, DANGER, {240, 80, 80, 255});
}

// ── scrollable list ───────────────────────────────────────────────────────────
struct ScrollList
{
    float scroll;
    ScrollList() : scroll(0) {}
    void update(Rectangle r)
    {
        if (CheckCollisionPointRec(GetMousePosition(), r))
            scroll -= GetMouseWheelMove() * 30;
        if (scroll < 0)
            scroll = 0;
    }
    void beginClip(Rectangle r)
    {
        BeginScissorMode((int)r.x, (int)r.y, (int)r.width, (int)r.height);
    }
    void endClip() { EndScissorMode(); }
    float offsetY() const { return -scroll; }
};

// ── message overlay ───────────────────────────────────────────────────────────
struct MsgBox
{
    char text[512];
    Screen returnTo;
    bool active;
    MsgBox() : active(false) { text[0] = '\0'; }
    void show(const char *msg, Screen back)
    {
        myStrncpy(text, msg, 512);
        returnTo = back;
        active = true;
    }
};

// ── global app state ──────────────────────────────────────────────────────────
struct AppState
{
    Screen screen;
    int loggedPatientId;
    int loggedDoctorId;
    int loginAttempts;
    char statusMsg[512];
    MsgBox msg;

    // login inputs
    TextInput inId, inPw;

    // booking sub-state
    char bookSpec[60];
    int bookDoctorId;
    char bookDate[12];
    char bookSlot[8];
    int bookStep; // 0=spec, 1=docId, 2=date, 3=slot
    TextInput inSpec, inDocId, inDate, inSlot;

    // generic single input
    TextInput inA, inB, inC, inD, inE;

    // scroll state
    ScrollList scroll;

    AppState() : screen(Screen::MAIN_MENU), loggedPatientId(0),
                 loggedDoctorId(0), loginAttempts(0), bookDoctorId(0),
                 bookStep(0)
    {
        statusMsg[0] = '\0';
        bookSpec[0] = bookDate[0] = bookSlot[0] = '\0';
    }
};

// ── drawing helpers ───────────────────────────────────────────────────────────
static void DrawHeader(const char *title, Font font, int sw)
{
    DrawRectangle(0, 0, sw, 64, PANEL);
    DrawLineEx({0, 64}, {(float)sw, 64}, 2, ACCENT);
    // logo dot
    DrawCircle(36, 32, 14, ACCENT);
    DrawTextEx(font, "M", {28, 20}, 24, 1, {10, 14, 30, 255});
    DrawTextEx(font, "MediCore", {58, 20}, 24, 1, ACCENT);
    Vector2 ts = MeasureTextEx(font, title, 20, 1);
    DrawTextEx(font, title, {(float)(sw - ts.x) / 2, 22}, 20, 1, TEXT_PRI);
}

static void DrawCard(Rectangle r)
{
    DrawRectangleRec(r, CARD);
    DrawRectangleLinesEx(r, 1, {255, 255, 255, 15});
}

// ── row text helper ───────────────────────────────────────────────────────────
static void RowText(Font f, const char *label, const char *val,
                    float x, float y, float lw = 150)
{
    DrawTextEx(f, label, {x, y}, 16, 1, TEXT_SEC);
    DrawTextEx(f, val, {x + lw, y}, 16, 1, TEXT_PRI);
}

// ── SCREENS ───────────────────────────────────────────────────────────────────

// Forward declaration of hospital system (defined in HospitalSystem.h)
// We expose a thin query API here so GUI doesn't need to touch Storage directly.

// ============================================================
//  GuiHospital — GUI-friendly wrapper around HospitalSystem
// ============================================================
class GuiHospital
{
public:
    Storage<Patient> patients;
    Storage<Doctor> doctors;
    Storage<Appointment> appointments;
    Storage<Bill> bills;
    Storage<Prescription> prescriptions;
    Admin admin;

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
        catch (...)
        {
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

    // auth
    Patient *authPatient(const char *id, const char *pw)
    {
        for (int i = 0; i < patients.size(); i++)
        {
            Patient *p = patients.getAll()[i];
            char tmp[16];
            intToStr(p->getId(), tmp);
            if (myStrcmp(tmp, id) == 0 && p->checkPassword(pw))
                return p;
        }
        return nullptr;
    }
    Doctor *authDoctor(const char *id, const char *pw)
    {
        for (int i = 0; i < doctors.size(); i++)
        {
            Doctor *d = doctors.getAll()[i];
            char tmp[16];
            intToStr(d->getId(), tmp);
            if (myStrcmp(tmp, id) == 0 && d->checkPassword(pw))
                return d;
        }
        return nullptr;
    }
    bool authAdmin(const char *id, const char *pw)
    {
        char tmp[16];
        intToStr(admin.getId(), tmp);
        return myStrcmp(tmp, id) == 0 && admin.checkPassword(pw);
    }

    const char *getPatientName(int pid)
    {
        Patient *p = patients.findById(pid);
        return p ? p->getName() : "Unknown";
    }
    const char *getDoctorName(int did)
    {
        Doctor *d = doctors.findById(did);
        return d ? d->getName() : "Unknown";
    }
};

// ── MAIN ─────────────────────────────────────────────────────────────────────
int main()
{
    const int SW = 1100, SH = 700;
    InitWindow(SW, SH, "MediCore — Hospital Management System");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // load font — falls back to default if not found
    Font font = GetFontDefault();
    // try to load a nicer font if you have one
    // Font font = LoadFontEx("Roboto-Regular.ttf", 20, nullptr, 0);

    GuiHospital hosp;
    hosp.loadAll();

    AppState st;

    // ── working buffers used across frames ────────────────────────────────────
    // booking flow
    int bookFoundDocs[100], bookFoundCount = 0;
    float bookDoctorFee = 0;
    const char *ALL_SLOTS[] = {"09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00"};

    // ── main loop ─────────────────────────────────────────────────────────────
    while (!WindowShouldClose())
    {
        // always handle active text inputs
        st.inId.handleInput();
        st.inPw.handleInput();
        st.inSpec.handleInput();
        st.inDocId.handleInput();
        st.inDate.handleInput();
        st.inSlot.handleInput();
        st.inA.handleInput();
        st.inB.handleInput();
        st.inC.handleInput();
        st.inD.handleInput();
        st.inE.handleInput();

        BeginDrawing();
        ClearBackground(BG);

        // ── message overlay (highest priority) ────────────────────────────────
        if (st.msg.active)
        {
            DrawRectangle(0, 0, SW, SH, {0, 0, 0, 160});
            Rectangle box = {(float)(SW / 2 - 250), (float)(SH / 2 - 100), 500, 200};
            DrawCard(box);
            DrawRectangleLinesEx(box, 2, ACCENT);
            // wrap text manually
            DrawTextEx(font, st.msg.text, {box.x + 20, box.y + 30}, 18, 1, TEXT_PRI);
            if (GuiButton({box.x + 180, box.y + 140, 140, 40}, "OK", font))
            {
                st.msg.active = false;
                st.screen = st.msg.returnTo;
            }
            EndDrawing();
            continue;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  MAIN MENU
        // ─────────────────────────────────────────────────────────────────────
        if (st.screen == Screen::MAIN_MENU)
        {
            DrawHeader("Welcome", font, SW);
            // big logo area
            DrawCircleGradient(SW / 2, 200, 80, {0, 200, 180, 40}, {0, 0, 0, 0});
            DrawTextEx(font, "MediCore", {(float)(SW / 2 - 80), 165}, 42, 2, ACCENT);
            DrawTextEx(font, "Hospital Management System",
                       {(float)(SW / 2 - 165), 215}, 22, 1, TEXT_SEC);

            float bw = 260, bh = 52, bx = (SW - bw) / 2;
            if (GuiButton({bx, 310, bw, bh}, "  Patient Login", font))
            {
                st.screen = Screen::PATIENT_LOGIN;
                st.inId.clear();
                st.inPw.clear();
                st.loginAttempts = 0;
                st.inId.active = true;
                st.inPw.active = false;
                st.inPw.isPassword = true;
            }
            if (GuiButton({bx, 380, bw, bh}, "  Doctor Login", font, ACCENT2, {255, 190, 80, 255}))
            {
                st.screen = Screen::DOCTOR_LOGIN;
                st.inId.clear();
                st.inPw.clear();
                st.loginAttempts = 0;
                st.inId.active = true;
                st.inPw.active = false;
                st.inPw.isPassword = true;
            }
            if (GuiButton({bx, 450, bw, bh}, "  Admin Login", font, {100, 120, 200, 255}, {130, 150, 230, 255}))
            {
                st.screen = Screen::ADMIN_LOGIN;
                st.inId.clear();
                st.inPw.clear();
                st.loginAttempts = 0;
                st.inId.active = true;
                st.inPw.active = false;
                st.inPw.isPassword = true;
            }
            if (DangerButton({bx, 530, bw, bh}, "  Exit", font))
                break;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  LOGIN SCREENS (patient / doctor / admin share same layout)
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::PATIENT_LOGIN ||
                 st.screen == Screen::DOCTOR_LOGIN ||
                 st.screen == Screen::ADMIN_LOGIN)
        {

            const char *role = (st.screen == Screen::PATIENT_LOGIN) ? "Patient" : (st.screen == Screen::DOCTOR_LOGIN) ? "Doctor"
                                                                                                                      : "Admin";
            char title[64];
            myStrcpy(title, role);
            myStrcat(title, " Login");
            DrawHeader(title, font, SW);

            DrawCard({(float)(SW / 2 - 200), 130, 400, 340});
            float ix = SW / 2 - 160, iy = 200;

            // click to activate
            Rectangle rId = {ix, iy, 320, 44};
            Rectangle rPw = {ix, iy + 80, 320, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                st.inId.active = CheckCollisionPointRec(GetMousePosition(), rId);
                st.inPw.active = CheckCollisionPointRec(GetMousePosition(), rPw);
            }
            st.inId.draw(rId, "ID", font);
            st.inPw.draw(rPw, "Password", font);

            bool login = GuiButton({(float)(SW / 2 - 80), iy + 180, 160, 46}, "Login", font);
            bool back = GuiButton({(float)(SW / 2 - 80), iy + 240, 160, 38}, "Back", font,
                                  {60, 70, 120, 255}, {80, 90, 150, 255});

            if (login || IsKeyPressed(KEY_ENTER))
            {
                bool ok = false;
                if (st.screen == Screen::PATIENT_LOGIN)
                {
                    Patient *p = hosp.authPatient(st.inId.buf, st.inPw.buf);
                    if (p)
                    {
                        st.loggedPatientId = p->getId();
                        ok = true;
                        st.screen = Screen::PATIENT_MENU;
                    }
                }
                else if (st.screen == Screen::DOCTOR_LOGIN)
                {
                    Doctor *d = hosp.authDoctor(st.inId.buf, st.inPw.buf);
                    if (d)
                    {
                        st.loggedDoctorId = d->getId();
                        ok = true;
                        st.screen = Screen::DOCTOR_MENU;
                    }
                }
                else
                {
                    if (hosp.authAdmin(st.inId.buf, st.inPw.buf))
                    {
                        ok = true;
                        st.screen = Screen::ADMIN_MENU;
                    }
                }
                if (ok)
                {
                    FileHandler::logSecurityEvent(role, st.inId.buf, "SUCCESS");
                    st.loginAttempts = 0;
                }
                else
                {
                    FileHandler::logSecurityEvent(role, st.inId.buf, "FAILED");
                    st.loginAttempts++;
                    if (st.loginAttempts >= 3)
                    {
                        st.msg.show("Account locked. Contact admin.", Screen::MAIN_MENU);
                    }
                    else
                    {
                        char tmp[128];
                        myStrcpy(tmp, "Invalid credentials. Attempts: ");
                        char n[4];
                        intToStr(st.loginAttempts, n);
                        myStrcat(tmp, n);
                        myStrcat(tmp, "/3");
                        myStrncpy(st.statusMsg, tmp, 512);
                    }
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 160), iy + 160}, 15, 1, DANGER);
            if (back)
            {
                st.screen = Screen::MAIN_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  PATIENT MENU
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::PATIENT_MENU)
        {
            Patient *pat = hosp.patients.findById(st.loggedPatientId);
            DrawHeader("Patient Dashboard", font, SW);

            // info card
            DrawCard({20, 80, 340, 100});
            if (pat)
            {
                char bal[32];
                floatToStr(pat->getBalance(), bal);
                DrawTextEx(font, pat->getName(), {40, 95}, 22, 1, ACCENT);
                DrawTextEx(font, "Balance:", {40, 125}, 16, 1, TEXT_SEC);
                DrawTextEx(font, bal, {135, 125}, 18, 1, ACCENT2);
                DrawTextEx(font, "PKR", {103, 127}, 14, 1, TEXT_SEC);
            }

            const char *labels[] = {
                "Book Appointment", "Cancel Appointment", "My Appointments",
                "Medical Records", "My Bills", "Pay Bill", "Top Up Balance", "Logout"};
            Screen targets[] = {
                Screen::P_BOOK, Screen::P_CANCEL, Screen::P_VIEW_APPTS,
                Screen::P_VIEW_RECORDS, Screen::P_VIEW_BILLS, Screen::P_PAY_BILL,
                Screen::P_TOPUP, Screen::MAIN_MENU};
            for (int i = 0; i < 8; i++)
            {
                float bx = 20 + (i % 2) * 370, by = 210 + (i / 2) * 90.0f;
                bool danger = (i == 7);
                bool clicked = danger ? DangerButton({bx, by, 340, 64}, labels[i], font) : GuiButton({bx, by, 340, 64}, labels[i], font, i < 4 ? ACCENT : ACCENT2, i < 4 ? BTN_HOVER : (Color){255, 190, 80, 255});
                if (clicked)
                {
                    if (targets[i] == Screen::MAIN_MENU)
                    {
                        st.loggedPatientId = 0;
                    }
                    st.screen = targets[i];
                    // reset sub-inputs
                    st.inA.clear();
                    st.inB.clear();
                    st.inC.clear();
                    st.inD.clear();
                    st.inE.clear();
                    st.inA.active = true;
                    st.bookStep = 0;
                    bookFoundCount = 0;
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_BOOK: Book Appointment (step-by-step)
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_BOOK)
        {
            DrawHeader("Book Appointment", font, SW);
            Patient *pat = hosp.patients.findById(st.loggedPatientId);
            if (!pat)
            {
                st.screen = Screen::PATIENT_MENU;
                goto end_draw;
            }

            // Step 0: enter specialization
            if (st.bookStep == 0)
            {
                DrawCard({(float)(SW / 2 - 250), 120, 500, 200});
                Rectangle r = {(float)(SW / 2 - 210), 180, 420, 44};
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
                st.inA.draw(r, "Specialization (e.g. Cardiology)", font);

                if (GuiButton({(float)(SW / 2 - 70), 260, 140, 44}, "Search", font))
                {
                    myStrncpy(st.bookSpec, st.inA.buf, 60);
                    bookFoundCount = 0;
                    for (int i = 0; i < hosp.doctors.size(); i++)
                    {
                        Doctor *d = hosp.doctors.getAll()[i];
                        if (d && myStrEqCI(d->getSpecialization(), st.bookSpec))
                            bookFoundDocs[bookFoundCount++] = d->getId();
                    }
                    if (bookFoundCount == 0)
                        myStrncpy(st.statusMsg, "No doctors for that specialization.", 512);
                    else
                    {
                        st.bookStep = 1;
                        st.statusMsg[0] = '\0';
                        st.inB.clear();
                        st.inB.active = true;
                    }
                }
                if (st.statusMsg[0])
                    DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 210), 315}, 16, 1, DANGER);
            }
            // Step 1: choose doctor
            else if (st.bookStep == 1)
            {
                DrawCard({30, 100, SW - 60, (float)(bookFoundCount * 50 + 80)});
                DrawTextEx(font, "Available Doctors:", {50, 110}, 18, 1, TEXT_SEC);
                for (int i = 0; i < bookFoundCount; i++)
                {
                    Doctor *d = hosp.doctors.findById(bookFoundDocs[i]);
                    if (!d)
                        continue;
                    char line[200];
                    char fee[32];
                    floatToStr(d->getFee(), fee);
                    myStrcpy(line, "ID:");
                    char sid[8];
                    intToStr(d->getId(), sid);
                    myStrcat(line, sid);
                    myStrcat(line, "  ");
                    myStrcat(line, d->getName());
                    myStrcat(line, "  Fee:PKR ");
                    myStrcat(line, fee);
                    DrawTextEx(font, line, {50, (float)(150 + i * 48)}, 17, 1, TEXT_PRI);
                }
                float baseY = 100 + bookFoundCount * 50 + 90;
                Rectangle r = {(float)(SW / 2 - 200), baseY, 400, 44};
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    st.inB.active = CheckCollisionPointRec(GetMousePosition(), r);
                st.inB.draw(r, "Enter Doctor ID", font);
                if (GuiButton({(float)(SW / 2 - 60), baseY + 60, 120, 44}, "Next", font))
                {
                    int did = strToInt(st.inB.buf);
                    bool valid = false;
                    for (int i = 0; i < bookFoundCount; i++)
                        if (bookFoundDocs[i] == did)
                            valid = true;
                    if (!valid)
                        myStrncpy(st.statusMsg, "Doctor not found.", 512);
                    else
                    {
                        st.bookDoctorId = did;
                        Doctor *d = hosp.doctors.findById(did);
                        bookDoctorFee = d ? d->getFee() : 0;
                        st.bookStep = 2;
                        st.statusMsg[0] = '\0';
                        st.inC.clear();
                        st.inC.active = true;
                    }
                }
                if (st.statusMsg[0])
                    DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 200), baseY + 115}, 16, 1, DANGER);
            }
            // Step 2: choose date
            else if (st.bookStep == 2)
            {
                DrawCard({(float)(SW / 2 - 250), 120, 500, 220});
                Rectangle r = {(float)(SW / 2 - 210), 180, 420, 44};
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    st.inC.active = CheckCollisionPointRec(GetMousePosition(), r);
                st.inC.draw(r, "Date (DD-MM-YYYY)", font);
                if (GuiButton({(float)(SW / 2 - 60), 265, 120, 44}, "Next", font))
                {
                    if (!Validator::isValidDate(st.inC.buf))
                        myStrncpy(st.statusMsg, "Invalid date. Use DD-MM-YYYY.", 512);
                    else
                    {
                        myStrncpy(st.bookDate, st.inC.buf, 12);
                        st.bookStep = 3;
                        st.statusMsg[0] = '\0';
                        st.inD.clear();
                        st.inD.active = true;
                    }
                }
                if (st.statusMsg[0])
                    DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 210), 320}, 16, 1, DANGER);
            }
            // Step 3: choose slot
            else if (st.bookStep == 3)
            {
                DrawTextEx(font, "Available Slots:", {30, 100}, 18, 1, TEXT_SEC);
                int col = 0;
                for (int s = 0; s < 8; s++)
                {
                    bool taken = false;
                    for (int i = 0; i < hosp.appointments.size(); i++)
                    {
                        Appointment *a = hosp.appointments.getAll()[i];
                        if (!a)
                            continue;
                        if (a->getDoctorId() == st.bookDoctorId &&
                            myStrcmp(a->getDate(), st.bookDate) == 0 &&
                            myStrcmp(a->getTimeSlot(), ALL_SLOTS[s]) == 0 &&
                            myStrcmp(a->getStatus(), "cancelled") != 0)
                        {
                            taken = true;
                            break;
                        }
                    }
                    float sx = 30 + (col % 4) * 200.0f, sy = 140 + (col / 4) * 70.0f;
                    Color bc = taken ? DANGER : ACCENT;
                    if (!taken && GuiButton({sx, sy, 160, 50}, ALL_SLOTS[s], font, bc, BTN_HOVER))
                    {
                        myStrncpy(st.bookSlot, ALL_SLOTS[s], 8);
                        // check funds
                        if (pat->getBalance() < bookDoctorFee)
                        {
                            st.msg.show("Insufficient funds!", Screen::PATIENT_MENU);
                        }
                        else
                        {
                            *pat -= bookDoctorFee;
                            int newApptId = hosp.appointments.maxId() + 1;
                            char today[12];
                            Validator::getTodayStr(today);
                            Appointment *appt = new Appointment(newApptId, pat->getId(),
                                                                st.bookDoctorId, st.bookDate, st.bookSlot, "pending");
                            hosp.appointments.add(appt);
                            FileHandler::appendAppointment(*appt);
                            int newBillId = hosp.bills.maxId() + 1;
                            Bill *bill = new Bill(newBillId, pat->getId(), newApptId,
                                                  bookDoctorFee, "unpaid", today);
                            hosp.bills.add(bill);
                            FileHandler::appendBill(*bill);
                            FileHandler::saveAllPatients(hosp.patients);
                            char tmp[128];
                            myStrcpy(tmp, "Appointment booked! ID: ");
                            char sid[8];
                            intToStr(newApptId, sid);
                            myStrcat(tmp, sid);
                            st.msg.show(tmp, Screen::PATIENT_MENU);
                            st.bookStep = 0;
                        }
                    }
                    if (!taken)
                        col++;
                    else
                    {
                        DrawRectangle((int)sx, (int)sy, 160, 50, {60, 20, 20, 200});
                        DrawTextEx(font, ALL_SLOTS[s], {sx + 50, sy + 15}, 16, 1, {150, 80, 80, 255});
                        DrawTextEx(font, "TAKEN", {sx + 55, sy + 32}, 12, 1, DANGER);
                        col++;
                    }
                }
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::PATIENT_MENU;
                st.statusMsg[0] = '\0';
                st.bookStep = 0;
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_VIEW_APPTS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_VIEW_APPTS)
        {
            DrawHeader("My Appointments", font, SW);
            // collect & sort
            Appointment *arr[100];
            int n = 0;
            for (int i = 0; i < hosp.appointments.size(); i++)
            {
                Appointment *a = hosp.appointments.getAll()[i];
                if (a && a->getPatientId() == st.loggedPatientId)
                    arr[n++] = a;
            }
            // bubble sort by date asc
            for (int i = 0; i < n - 1; i++)
                for (int j = 0; j < n - i - 1; j++)
                    if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) > 0)
                    {
                        Appointment *t = arr[j];
                        arr[j] = arr[j + 1];
                        arr[j + 1] = t;
                    }

            Rectangle listR = {20, 80, (float)(SW - 40), (float)(SH - 150)};
            st.scroll.update(listR);
            st.scroll.beginClip(listR);
            float y = 80 + st.scroll.offsetY();
            // header row
            DrawTextEx(font, "ID", {30, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Doctor", {90, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Spec", {290, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Date", {470, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Slot", {610, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Status", {710, y}, 16, 1, TEXT_SEC);
            y += 26;
            DrawLineEx({20, y}, {(float)(SW - 20), y}, 1, TEXT_SEC);
            y += 8;
            for (int i = 0; i < n; i++)
            {
                Doctor *d = hosp.doctors.findById(arr[i]->getDoctorId());
                Color sc = (myStrcmp(arr[i]->getStatus(), "completed") == 0) ? ACCENT : (myStrcmp(arr[i]->getStatus(), "pending") == 0) ? ACCENT2
                                                                                                                                        : DANGER;
                char sid[8];
                intToStr(arr[i]->getId(), sid);
                DrawTextEx(font, sid, {30, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, d ? d->getName() : "?", {90, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, d ? d->getSpecialization() : "?", {290, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getDate(), {470, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getTimeSlot(), {610, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getStatus(), {710, y}, 16, 1, sc);
                y += 38;
            }
            if (n == 0)
                DrawTextEx(font, "No appointments found.", {(float)(SW / 2 - 120), 300}, 20, 1, TEXT_SEC);
            st.scroll.endClip();
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::PATIENT_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_VIEW_BILLS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_VIEW_BILLS)
        {
            DrawHeader("My Bills", font, SW);
            float y = 90;
            float totalUnpaid = 0;
            DrawTextEx(font, "BillID", {30, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "ApptID", {110, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Amount", {200, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Status", {340, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Date", {460, y}, 16, 1, TEXT_SEC);
            y += 28;
            DrawLineEx({20, y}, {800, y}, 1, TEXT_SEC);
            y += 8;
            bool any = false;
            for (int i = 0; i < hosp.bills.size(); i++)
            {
                Bill *b = hosp.bills.getAll()[i];
                if (!b || b->getPatientId() != st.loggedPatientId)
                    continue;
                any = true;
                char sid[8], said[8], amt[32];
                intToStr(b->getId(), sid);
                intToStr(b->getAppointmentId(), said);
                floatToStr(b->getAmount(), amt);
                Color sc = (myStrcmp(b->getStatus(), "paid") == 0) ? ACCENT : (myStrcmp(b->getStatus(), "unpaid") == 0) ? ACCENT2
                                                                                                                        : DANGER;
                DrawTextEx(font, sid, {30, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, said, {110, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, amt, {200, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, b->getStatus(), {340, y}, 16, 1, sc);
                DrawTextEx(font, b->getDate(), {460, y}, 16, 1, TEXT_PRI);
                if (myStrcmp(b->getStatus(), "unpaid") == 0)
                    totalUnpaid += b->getAmount();
                y += 36;
            }
            if (!any)
                DrawTextEx(font, "No bills found.", {(float)(SW / 2 - 100), 300}, 20, 1, TEXT_SEC);
            char tot[32];
            floatToStr(totalUnpaid, tot);
            DrawTextEx(font, "Total Outstanding:", {30, y + 10}, 17, 1, TEXT_SEC);
            DrawTextEx(font, tot, {190, y + 10}, 17, 1, ACCENT2);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::PATIENT_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_TOPUP
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_TOPUP)
        {
            DrawHeader("Top Up Balance", font, SW);
            Patient *pat = hosp.patients.findById(st.loggedPatientId);
            DrawCard({(float)(SW / 2 - 220), 120, 440, 220});
            Rectangle r = {(float)(SW / 2 - 180), 190, 360, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
            st.inA.draw(r, "Amount (PKR)", font);
            if (GuiButton({(float)(SW / 2 - 70), 265, 140, 44}, "Top Up", font))
            {
                try
                {
                    if (!Validator::isPositiveFloat(st.inA.buf))
                        throw InvalidInputException("Amount must be positive.");
                    float amt = strToFloat(st.inA.buf);
                    *pat += amt;
                    FileHandler::saveAllPatients(hosp.patients);
                    char msg[128];
                    myStrcpy(msg, "Balance updated! New: PKR ");
                    char bal[32];
                    floatToStr(pat->getBalance(), bal);
                    myStrcat(msg, bal);
                    st.msg.show(msg, Screen::PATIENT_MENU);
                }
                catch (InvalidInputException &e)
                {
                    myStrncpy(st.statusMsg, e.what(), 512);
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 180), 320}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::PATIENT_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_PAY_BILL
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_PAY_BILL)
        {
            DrawHeader("Pay Bill", font, SW);
            Patient *pat = hosp.patients.findById(st.loggedPatientId);
            float y = 90;
            DrawTextEx(font, "Unpaid Bills:", {30, y}, 18, 1, TEXT_SEC);
            y += 34;
            bool any = false;
            for (int i = 0; i < hosp.bills.size(); i++)
            {
                Bill *b = hosp.bills.getAll()[i];
                if (!b || b->getPatientId() != st.loggedPatientId || myStrcmp(b->getStatus(), "unpaid") != 0)
                    continue;
                char sid[8], said[8], amt[32];
                intToStr(b->getId(), sid);
                intToStr(b->getAppointmentId(), said);
                floatToStr(b->getAmount(), amt);
                DrawTextEx(font, sid, {30, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, said, {100, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, amt, {180, y}, 16, 1, ACCENT2);
                DrawTextEx(font, b->getDate(), {310, y}, 16, 1, TEXT_PRI);
                any = true;
                y += 36;
            }
            if (!any)
            {
                DrawTextEx(font, "No unpaid bills.", {(float)(SW / 2 - 100), 300}, 20, 1, TEXT_SEC);
            }
            else
            {
                DrawCard({(float)(SW / 2 - 200), y + 10, 400, 120});
                Rectangle r = {(float)(SW / 2 - 160), y + 50, 320, 44};
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
                st.inA.draw(r, "Enter Bill ID", font);
                if (GuiButton({(float)(SW / 2 - 60), y + 110, 120, 40}, "Pay", font))
                {
                    int bid = strToInt(st.inA.buf);
                    Bill *target = nullptr;
                    for (int i = 0; i < hosp.bills.size(); i++)
                    {
                        Bill *b = hosp.bills.getAll()[i];
                        if (b && b->getId() == bid && b->getPatientId() == st.loggedPatientId &&
                            myStrcmp(b->getStatus(), "unpaid") == 0)
                        {
                            target = b;
                            break;
                        }
                    }
                    if (!target)
                        myStrncpy(st.statusMsg, "Invalid bill ID.", 512);
                    else if (pat && pat->getBalance() < target->getAmount())
                        myStrncpy(st.statusMsg, "Insufficient funds.", 512);
                    else if (pat)
                    {
                        *pat -= target->getAmount();
                        target->setStatus("paid");
                        FileHandler::saveAllBills(hosp.bills);
                        FileHandler::saveAllPatients(hosp.patients);
                        char msg[128];
                        myStrcpy(msg, "Bill paid! Balance: PKR ");
                        char bal[32];
                        floatToStr(pat->getBalance(), bal);
                        myStrcat(msg, bal);
                        st.msg.show(msg, Screen::PATIENT_MENU);
                    }
                }
                if (st.statusMsg[0])
                    DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 160), y + 160}, 16, 1, DANGER);
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::PATIENT_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_CANCEL
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_CANCEL)
        {
            DrawHeader("Cancel Appointment", font, SW);
            Patient *pat = hosp.patients.findById(st.loggedPatientId);
            float y = 90;
            bool any = false;
            for (int i = 0; i < hosp.appointments.size(); i++)
            {
                Appointment *a = hosp.appointments.getAll()[i];
                if (!a || a->getPatientId() != st.loggedPatientId || myStrcmp(a->getStatus(), "pending") != 0)
                    continue;
                char sid[8];
                intToStr(a->getId(), sid);
                DrawTextEx(font, sid, {30, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, hosp.getDoctorName(a->getDoctorId()), {100, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, a->getDate(), {340, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, a->getTimeSlot(), {490, y}, 16, 1, TEXT_PRI);
                any = true;
                y += 36;
            }
            if (!any)
                DrawTextEx(font, "No pending appointments.", {(float)(SW / 2 - 130), 300}, 20, 1, TEXT_SEC);
            else
            {
                DrawCard({(float)(SW / 2 - 200), y + 10, 400, 120});
                Rectangle r = {(float)(SW / 2 - 160), y + 50, 320, 44};
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
                st.inA.draw(r, "Enter Appointment ID to cancel", font);
                if (DangerButton({(float)(SW / 2 - 60), y + 108, 120, 40}, "Cancel", font))
                {
                    int aid = strToInt(st.inA.buf);
                    Appointment *target = nullptr;
                    for (int i = 0; i < hosp.appointments.size(); i++)
                    {
                        Appointment *a = hosp.appointments.getAll()[i];
                        if (a && a->getId() == aid && a->getPatientId() == st.loggedPatientId &&
                            myStrcmp(a->getStatus(), "pending") == 0)
                        {
                            target = a;
                            break;
                        }
                    }
                    if (!target)
                        myStrncpy(st.statusMsg, "Invalid appointment ID.", 512);
                    else
                    {
                        Doctor *d = hosp.doctors.findById(target->getDoctorId());
                        float refund = d ? d->getFee() : 0;
                        target->setStatus("cancelled");
                        if (pat)
                            *pat += refund;
                        for (int i = 0; i < hosp.bills.size(); i++)
                        {
                            Bill *b = hosp.bills.getAll()[i];
                            if (b && b->getAppointmentId() == aid)
                            {
                                b->setStatus("cancelled");
                                break;
                            }
                        }
                        FileHandler::saveAllAppointments(hosp.appointments);
                        FileHandler::saveAllBills(hosp.bills);
                        FileHandler::saveAllPatients(hosp.patients);
                        char msg[128];
                        myStrcpy(msg, "Cancelled. Refunded PKR ");
                        char rs[32];
                        floatToStr(refund, rs);
                        myStrcat(msg, rs);
                        st.msg.show(msg, Screen::PATIENT_MENU);
                    }
                }
                if (st.statusMsg[0])
                    DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 160), y + 155}, 16, 1, DANGER);
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::PATIENT_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  P_VIEW_RECORDS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::P_VIEW_RECORDS)
        {
            DrawHeader("Medical Records", font, SW);
            Prescription *arr[100];
            int n = 0;
            for (int i = 0; i < hosp.prescriptions.size(); i++)
            {
                Prescription *rx = hosp.prescriptions.getAll()[i];
                if (rx && rx->getPatientId() == st.loggedPatientId)
                    arr[n++] = rx;
            }
            // sort desc
            for (int i = 0; i < n - 1; i++)
                for (int j = 0; j < n - i - 1; j++)
                    if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                    {
                        Prescription *t = arr[j];
                        arr[j] = arr[j + 1];
                        arr[j + 1] = t;
                    }

            Rectangle listR = {20, 80, (float)(SW - 40), (float)(SH - 150)};
            st.scroll.update(listR);
            st.scroll.beginClip(listR);
            float y = 90 + st.scroll.offsetY();
            if (n == 0)
                DrawTextEx(font, "No medical records found.", {(float)(SW / 2 - 130), 300}, 20, 1, TEXT_SEC);
            for (int i = 0; i < n; i++)
            {
                DrawCard({20, y, (float)(SW - 40), 90});
                DrawTextEx(font, arr[i]->getDate(), {35, y + 8}, 16, 1, ACCENT2);
                DrawTextEx(font, hosp.getDoctorName(arr[i]->getDoctorId()), {150, y + 8}, 16, 1, ACCENT);
                DrawTextEx(font, "Medicines:", {35, y + 32}, 14, 1, TEXT_SEC);
                DrawTextEx(font, arr[i]->getMedicines(), {120, y + 32}, 14, 1, TEXT_PRI);
                DrawTextEx(font, "Notes:", {35, y + 54}, 14, 1, TEXT_SEC);
                DrawTextEx(font, arr[i]->getNotes(), {90, y + 54}, 14, 1, TEXT_PRI);
                y += 100;
            }
            st.scroll.endClip();
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::PATIENT_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  DOCTOR MENU
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::DOCTOR_MENU)
        {
            Doctor *doc = hosp.doctors.findById(st.loggedDoctorId);
            DrawHeader("Doctor Dashboard", font, SW);
            if (doc)
            {
                DrawCard({20, 80, 500, 80});
                DrawTextEx(font, doc->getName(), {40, 95}, 22, 1, ACCENT);
                DrawTextEx(font, doc->getSpecialization(), {40, 122}, 16, 1, TEXT_SEC);
            }
            const char *labels[] = {"Today's Appointments", "Mark Complete", "Mark No-Show",
                                    "Write Prescription", "Patient History", "Logout"};
            Screen tgts[] = {Screen::D_TODAY, Screen::D_COMPLETE, Screen::D_NOSHOW,
                             Screen::D_PRESCRIBE, Screen::D_HISTORY, Screen::MAIN_MENU};
            for (int i = 0; i < 6; i++)
            {
                float bx = 20 + (i % 2) * 380.0f, by = 200 + (i / 2) * 90.0f;
                bool danger = (i == 5);
                if (danger ? DangerButton({bx, by, 340, 64}, labels[i], font) : GuiButton({bx, by, 340, 64}, labels[i], font))
                {
                    if (tgts[i] == Screen::MAIN_MENU)
                        st.loggedDoctorId = 0;
                    st.screen = tgts[i];
                    st.inA.clear();
                    st.inA.active = true;
                    st.statusMsg[0] = '\0';
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  D_TODAY
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::D_TODAY)
        {
            DrawHeader("Today's Appointments", font, SW);
            char today[12];
            Validator::getTodayStr(today);
            float y = 90;
            DrawTextEx(font, "ID", {30, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Patient", {100, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Slot", {320, y}, 16, 1, TEXT_SEC);
            DrawTextEx(font, "Status", {420, y}, 16, 1, TEXT_SEC);
            y += 28;
            bool any = false;
            for (int i = 0; i < hosp.appointments.size(); i++)
            {
                Appointment *a = hosp.appointments.getAll()[i];
                if (!a || a->getDoctorId() != st.loggedDoctorId || myStrcmp(a->getDate(), today) != 0)
                    continue;
                char sid[8];
                intToStr(a->getId(), sid);
                Color sc = (myStrcmp(a->getStatus(), "completed") == 0) ? ACCENT : (myStrcmp(a->getStatus(), "pending") == 0) ? ACCENT2
                                                                                                                              : DANGER;
                DrawTextEx(font, sid, {30, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, hosp.getPatientName(a->getPatientId()), {100, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, a->getTimeSlot(), {320, y}, 16, 1, TEXT_PRI);
                DrawTextEx(font, a->getStatus(), {420, y}, 16, 1, sc);
                y += 36;
                any = true;
            }
            if (!any)
                DrawTextEx(font, "No appointments today.", {(float)(SW / 2 - 120), 300}, 20, 1, TEXT_SEC);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::DOCTOR_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  D_COMPLETE / D_NOSHOW (shared layout)
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::D_COMPLETE || st.screen == Screen::D_NOSHOW)
        {
            bool isComplete = (st.screen == Screen::D_COMPLETE);
            DrawHeader(isComplete ? "Mark Complete" : "Mark No-Show", font, SW);
            DrawCard({(float)(SW / 2 - 220), 120, 440, 200});
            Rectangle r = {(float)(SW / 2 - 180), 190, 360, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
            st.inA.draw(r, "Appointment ID", font);
            if (GuiButton({(float)(SW / 2 - 70), 265, 140, 44}, isComplete ? "Mark Complete" : "Mark No-Show", font,
                          isComplete ? ACCENT : DANGER, isComplete ? BTN_HOVER : (Color){240, 80, 80, 255}))
            {
                char today[12];
                Validator::getTodayStr(today);
                int aid = strToInt(st.inA.buf);
                Appointment *a = hosp.appointments.findById(aid);
                if (!a || a->getDoctorId() != st.loggedDoctorId ||
                    myStrcmp(a->getStatus(), "pending") != 0 ||
                    myStrcmp(a->getDate(), today) != 0)
                    myStrncpy(st.statusMsg, "Invalid appointment.", 512);
                else
                {
                    a->setStatus(isComplete ? "completed" : "noshow");
                    if (!isComplete)
                    {
                        for (int i = 0; i < hosp.bills.size(); i++)
                        {
                            Bill *b = hosp.bills.getAll()[i];
                            if (b && b->getAppointmentId() == aid)
                            {
                                b->setStatus("cancelled");
                                break;
                            }
                        }
                        FileHandler::saveAllBills(hosp.bills);
                    }
                    FileHandler::saveAllAppointments(hosp.appointments);
                    st.msg.show(isComplete ? "Marked as completed." : "Marked as no-show.", Screen::DOCTOR_MENU);
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 180), 320}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::DOCTOR_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  D_PRESCRIBE
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::D_PRESCRIBE)
        {
            DrawHeader("Write Prescription", font, SW);
            DrawCard({(float)(SW / 2 - 280), 90, 560, 420});
            float ix = (float)(SW / 2 - 240), iy = 130;
            Rectangle rA = {ix, iy, 480, 44}, rB = {ix, iy + 80, 480, 44}, rC = {ix, iy + 160, 480, 100};
            st.inA.isPassword = false;
            st.inB.isPassword = false;
            st.inC.isPassword = false;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), rA);
                st.inB.active = CheckCollisionPointRec(GetMousePosition(), rB);
                st.inC.active = CheckCollisionPointRec(GetMousePosition(), rC);
            }
            st.inA.draw(rA, "Appointment ID", font);
            st.inB.draw(rB, "Medicines (Name Dose;Name Dose;...)", font);
            st.inC.draw(rC, "Notes", font);
            if (GuiButton({(float)(SW / 2 - 70), iy + 285, 140, 44}, "Save", font))
            {
                int aid = strToInt(st.inA.buf);
                Appointment *a = hosp.appointments.findById(aid);
                if (!a || a->getDoctorId() != st.loggedDoctorId || myStrcmp(a->getStatus(), "completed") != 0)
                    myStrncpy(st.statusMsg, "Invalid or incomplete appointment.", 512);
                else
                {
                    bool exists = false;
                    for (int i = 0; i < hosp.prescriptions.size(); i++)
                    {
                        Prescription *rx = hosp.prescriptions.getAll()[i];
                        if (rx && rx->getAppointmentId() == aid)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if (exists)
                        myStrncpy(st.statusMsg, "Prescription already written.", 512);
                    else
                    {
                        int newId = hosp.prescriptions.maxId() + 1;
                        Prescription *rx = new Prescription(newId, aid, a->getPatientId(),
                                                            st.loggedDoctorId, a->getDate(), st.inB.buf, st.inC.buf);
                        hosp.prescriptions.add(rx);
                        FileHandler::appendPrescription(*rx);
                        st.msg.show("Prescription saved.", Screen::DOCTOR_MENU);
                    }
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {ix, iy + 340}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::DOCTOR_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  ADMIN MENU
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::ADMIN_MENU)
        {
            DrawHeader("Admin Panel", font, SW);
            const char *labels[] = {"Add Doctor", "Remove Doctor", "All Patients", "All Doctors",
                                    "All Appointments", "Unpaid Bills", "Discharge Patient",
                                    "Security Log", "Daily Report", "Logout"};
            Screen tgts[] = {Screen::A_ADD_DOC, Screen::A_REMOVE_DOC, Screen::A_VIEW_PATIENTS,
                             Screen::A_VIEW_DOCTORS, Screen::A_VIEW_APPTS, Screen::A_UNPAID,
                             Screen::A_DISCHARGE, Screen::A_SECURITY, Screen::A_REPORT, Screen::MAIN_MENU};
            for (int i = 0; i < 10; i++)
            {
                float bx = 20 + (i % 5) * 210.0f, by = 100 + (i / 5) * 110.0f;
                bool danger = (i == 9);
                if (danger ? DangerButton({bx, by, 190, 80}, labels[i], font) : GuiButton({bx, by, 190, 80}, labels[i], font, i < 5 ? ACCENT : ACCENT2, i < 5 ? BTN_HOVER : (Color){255, 190, 80, 255}))
                {
                    st.screen = tgts[i];
                    st.inA.clear();
                    st.inB.clear();
                    st.inC.clear();
                    st.inD.clear();
                    st.inE.clear();
                    st.inA.active = true;
                    st.statusMsg[0] = '\0';
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_VIEW_PATIENTS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_VIEW_PATIENTS)
        {
            DrawHeader("All Patients", font, SW);
            Rectangle listR = {20, 80, (float)(SW - 40), (float)(SH - 150)};
            st.scroll.update(listR);
            st.scroll.beginClip(listR);
            float y = 90 + st.scroll.offsetY();
            DrawTextEx(font, "ID", {30, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Name", {80, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Age", {280, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Gender", {330, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Contact", {410, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Balance", {570, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Unpaid", {680, y}, 15, 1, TEXT_SEC);
            y += 26;
            DrawLineEx({20, y}, {(float)(SW - 20), y}, 1, TEXT_SEC);
            y += 8;
            for (int i = 0; i < hosp.patients.size(); i++)
            {
                Patient *p = hosp.patients.getAll()[i];
                if (!p)
                    continue;
                int unpaid = 0;
                for (int j = 0; j < hosp.bills.size(); j++)
                {
                    Bill *b = hosp.bills.getAll()[j];
                    if (b && b->getPatientId() == p->getId() && myStrcmp(b->getStatus(), "unpaid") == 0)
                        unpaid++;
                }
                char sid[8], age[8], bal[32];
                intToStr(p->getId(), sid);
                intToStr(p->getAge(), age);
                floatToStr(p->getBalance(), bal);
                DrawTextEx(font, sid, {30, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, p->getName(), {80, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, age, {280, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, p->getGender(), {330, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, p->getContact(), {410, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, bal, {570, y}, 15, 1, TEXT_PRI);
                char su[8];
                intToStr(unpaid, su);
                DrawTextEx(font, su, {680, y}, 15, 1, unpaid > 0 ? DANGER : TEXT_PRI);
                y += 34;
            }
            st.scroll.endClip();
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::ADMIN_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_VIEW_DOCTORS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_VIEW_DOCTORS)
        {
            DrawHeader("All Doctors", font, SW);
            float y = 90;
            DrawTextEx(font, "ID", {30, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Name", {80, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Specialization", {310, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Contact", {510, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Fee", {670, y}, 15, 1, TEXT_SEC);
            y += 26;
            for (int i = 0; i < hosp.doctors.size(); i++)
            {
                Doctor *d = hosp.doctors.getAll()[i];
                if (!d)
                    continue;
                char sid[8], fee[32];
                intToStr(d->getId(), sid);
                floatToStr(d->getFee(), fee);
                DrawTextEx(font, sid, {30, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, d->getName(), {80, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, d->getSpecialization(), {310, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, d->getContact(), {510, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, fee, {670, y}, 15, 1, ACCENT2);
                y += 34;
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::ADMIN_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_ADD_DOC
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_ADD_DOC)
        {
            DrawHeader("Add Doctor", font, SW);
            DrawCard({(float)(SW / 2 - 300), 80, 600, 500});
            float ix = (float)(SW / 2 - 260), iy = 110;
            st.inA.isPassword = false;
            st.inB.isPassword = false;
            st.inC.isPassword = false;
            st.inD.isPassword = true;
            st.inE.isPassword = false;
            Rectangle rA = {ix, iy, 520, 44}, rB = {ix, iy + 80, 520, 44}, rC = {ix, iy + 160, 520, 44};
            Rectangle rD = {ix, iy + 240, 520, 44}, rE = {ix, iy + 320, 520, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), rA);
                st.inB.active = CheckCollisionPointRec(GetMousePosition(), rB);
                st.inC.active = CheckCollisionPointRec(GetMousePosition(), rC);
                st.inD.active = CheckCollisionPointRec(GetMousePosition(), rD);
                st.inE.active = CheckCollisionPointRec(GetMousePosition(), rE);
            }
            st.inA.draw(rA, "Name", font);
            st.inB.draw(rB, "Specialization", font);
            st.inC.draw(rC, "Contact (11 digits)", font);
            st.inD.draw(rD, "Password (min 6 chars)", font);
            st.inE.draw(rE, "Consultation Fee (PKR)", font);
            if (GuiButton({(float)(SW / 2 - 70), iy + 390, 140, 44}, "Add Doctor", font))
            {
                bool ok = true;
                if (!Validator::isValidContact(st.inC.buf))
                {
                    myStrncpy(st.statusMsg, "Invalid contact.", 512);
                    ok = false;
                }
                else if (!Validator::isValidPassword(st.inD.buf))
                {
                    myStrncpy(st.statusMsg, "Password too short.", 512);
                    ok = false;
                }
                else if (!Validator::isPositiveFloat(st.inE.buf))
                {
                    myStrncpy(st.statusMsg, "Invalid fee.", 512);
                    ok = false;
                }
                if (ok)
                {
                    int newId = hosp.doctors.maxId() + 1;
                    Doctor *doc = new Doctor(newId, st.inA.buf, st.inB.buf,
                                             st.inC.buf, st.inD.buf, strToFloat(st.inE.buf));
                    hosp.doctors.add(doc);
                    FileHandler::appendDoctor(*doc);
                    char msg[128];
                    myStrcpy(msg, "Doctor added! ID: ");
                    char sid[8];
                    intToStr(newId, sid);
                    myStrcat(msg, sid);
                    st.msg.show(msg, Screen::ADMIN_MENU);
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {ix, iy + 445}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::ADMIN_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_DISCHARGE
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_DISCHARGE)
        {
            DrawHeader("Discharge Patient", font, SW);
            DrawCard({(float)(SW / 2 - 220), 120, 440, 200});
            Rectangle r = {(float)(SW / 2 - 180), 190, 360, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
            st.inA.draw(r, "Patient ID", font);
            if (DangerButton({(float)(SW / 2 - 80), 265, 160, 44}, "Discharge", font))
            {
                int pid = strToInt(st.inA.buf);
                Patient *p = hosp.patients.findById(pid);
                if (!p)
                {
                    myStrncpy(st.statusMsg, "Patient not found.", 512);
                }
                else
                {
                    bool hasUnpaid = false, hasPending = false;
                    for (int i = 0; i < hosp.bills.size(); i++)
                    {
                        Bill *b = hosp.bills.getAll()[i];
                        if (b && b->getPatientId() == pid && myStrcmp(b->getStatus(), "unpaid") == 0)
                        {
                            hasUnpaid = true;
                            break;
                        }
                    }
                    for (int i = 0; i < hosp.appointments.size(); i++)
                    {
                        Appointment *a = hosp.appointments.getAll()[i];
                        if (a && a->getPatientId() == pid && myStrcmp(a->getStatus(), "pending") == 0)
                        {
                            hasPending = true;
                            break;
                        }
                    }
                    if (hasUnpaid)
                        myStrncpy(st.statusMsg, "Cannot discharge: unpaid bills.", 512);
                    else if (hasPending)
                        myStrncpy(st.statusMsg, "Cannot discharge: pending appointments.", 512);
                    else
                    {
                        FileHandler::dischargePatient(pid, hosp.patients, hosp.appointments, hosp.bills, hosp.prescriptions);
                        hosp.patients.removeById(pid);
                        delete p;
                        FileHandler::saveAllPatients(hosp.patients);
                        FileHandler::saveAllAppointments(hosp.appointments);
                        FileHandler::saveAllBills(hosp.bills);
                        FileHandler::saveAllPrescriptions(hosp.prescriptions);
                        st.msg.show("Patient discharged and archived.", Screen::ADMIN_MENU);
                    }
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 180), 320}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::ADMIN_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_SECURITY
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_SECURITY)
        {
            DrawHeader("Security Log", font, SW);
            static char logBuf[4096] = {};
            static bool loaded = false;
            if (!loaded)
            {
                FileHandler::readSecurityLog(logBuf, 4096);
                loaded = true;
            }
            Rectangle listR = {20, 80, (float)(SW - 40), (float)(SH - 150)};
            st.scroll.update(listR);
            DrawCard(listR);
            st.scroll.beginClip(listR);
            float y = 90 + st.scroll.offsetY();
            // draw line by line
            const char *p = logBuf;
            char line[256];
            int li = 0;
            while (*p)
            {
                if (*p == '\n')
                {
                    line[li] = '\0';
                    DrawTextEx(font, line, {30, y}, 15, 1, TEXT_PRI);
                    y += 22;
                    li = 0;
                }
                else if (li < 254)
                    line[li++] = *p;
                p++;
            }
            if (li > 0)
            {
                line[li] = '\0';
                DrawTextEx(font, line, {30, y}, 15, 1, TEXT_PRI);
            }
            st.scroll.endClip();
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::ADMIN_MENU;
                loaded = false;
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_REPORT (Daily Report)
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_REPORT)
        {
            DrawHeader("Daily Report", font, SW);
            char today[12];
            Validator::getTodayStr(today);
            int total = 0, pending = 0, completed = 0, noshow = 0, cancelled = 0;
            float revenue = 0;
            for (int i = 0; i < hosp.appointments.size(); i++)
            {
                Appointment *a = hosp.appointments.getAll()[i];
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
            for (int i = 0; i < hosp.bills.size(); i++)
            {
                Bill *b = hosp.bills.getAll()[i];
                if (b && myStrcmp(b->getDate(), today) == 0 && myStrcmp(b->getStatus(), "paid") == 0)
                    revenue += b->getAmount();
            }
            float y = 90;
            char revStr[32];
            floatToStr(revenue, revStr);
            char tmp[128];
            myStrcpy(tmp, "Date: ");
            myStrcat(tmp, today);
            DrawTextEx(font, tmp, {30, y}, 18, 1, ACCENT2);
            y += 36;
            myStrcpy(tmp, "Appointments — Total: ");
            char n[8];
            intToStr(total, n);
            myStrcat(tmp, n);
            myStrcat(tmp, "  Pending: ");
            intToStr(pending, n);
            myStrcat(tmp, n);
            myStrcat(tmp, "  Completed: ");
            intToStr(completed, n);
            myStrcat(tmp, n);
            myStrcat(tmp, "  No-show: ");
            intToStr(noshow, n);
            myStrcat(tmp, n);
            DrawTextEx(font, tmp, {30, y}, 16, 1, TEXT_PRI);
            y += 32;
            myStrcpy(tmp, "Revenue (paid bills): PKR ");
            myStrcat(tmp, revStr);
            DrawTextEx(font, tmp, {30, y}, 16, 1, ACCENT);
            y += 42;
            DrawTextEx(font, "Patients with unpaid bills:", {30, y}, 16, 1, TEXT_SEC);
            y += 28;
            for (int i = 0; i < hosp.patients.size(); i++)
            {
                Patient *p = hosp.patients.getAll()[i];
                if (!p)
                    continue;
                float owed = 0;
                for (int j = 0; j < hosp.bills.size(); j++)
                {
                    Bill *b = hosp.bills.getAll()[j];
                    if (b && b->getPatientId() == p->getId() && myStrcmp(b->getStatus(), "unpaid") == 0)
                        owed += b->getAmount();
                }
                if (owed > 0)
                {
                    char os[32];
                    floatToStr(owed, os);
                    myStrcpy(tmp, p->getName());
                    myStrcat(tmp, " — PKR ");
                    myStrcat(tmp, os);
                    DrawTextEx(font, tmp, {50, y}, 15, 1, DANGER);
                    y += 26;
                }
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::ADMIN_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_VIEW_APPTS
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_VIEW_APPTS)
        {
            DrawHeader("All Appointments", font, SW);
            Rectangle listR = {20, 80, (float)(SW - 40), (float)(SH - 150)};
            st.scroll.update(listR);
            st.scroll.beginClip(listR);
            float y = 90 + st.scroll.offsetY();
            DrawTextEx(font, "ID", {30, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Patient", {80, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Doctor", {270, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Date", {460, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Slot", {590, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Status", {670, y}, 15, 1, TEXT_SEC);
            y += 26;
            Appointment *arr[200];
            int n = 0;
            for (int i = 0; i < hosp.appointments.size(); i++)
                if (hosp.appointments.getAll()[i])
                    arr[n++] = hosp.appointments.getAll()[i];
            for (int i = 0; i < n - 1; i++)
                for (int j = 0; j < n - i - 1; j++)
                    if (Validator::compareDates(arr[j]->getDate(), arr[j + 1]->getDate()) < 0)
                    {
                        Appointment *t = arr[j];
                        arr[j] = arr[j + 1];
                        arr[j + 1] = t;
                    }
            for (int i = 0; i < n; i++)
            {
                char sid[8];
                intToStr(arr[i]->getId(), sid);
                Color sc = (myStrcmp(arr[i]->getStatus(), "completed") == 0) ? ACCENT : (myStrcmp(arr[i]->getStatus(), "pending") == 0) ? ACCENT2
                                                                                                                                        : DANGER;
                DrawTextEx(font, sid, {30, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, hosp.getPatientName(arr[i]->getPatientId()), {80, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, hosp.getDoctorName(arr[i]->getDoctorId()), {270, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getDate(), {460, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getTimeSlot(), {590, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, arr[i]->getStatus(), {670, y}, 15, 1, sc);
                y += 32;
            }
            st.scroll.endClip();
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::ADMIN_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_UNPAID
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_UNPAID)
        {
            DrawHeader("Unpaid Bills", font, SW);
            char today[12];
            Validator::getTodayStr(today);
            float y = 90;
            DrawTextEx(font, "BillID", {30, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Patient", {110, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Amount", {320, y}, 15, 1, TEXT_SEC);
            DrawTextEx(font, "Date", {440, y}, 15, 1, TEXT_SEC);
            y += 28;
            for (int i = 0; i < hosp.bills.size(); i++)
            {
                Bill *b = hosp.bills.getAll()[i];
                if (!b || myStrcmp(b->getStatus(), "unpaid") != 0)
                    continue;
                char sid[8], amt[32];
                intToStr(b->getId(), sid);
                floatToStr(b->getAmount(), amt);
                DrawTextEx(font, sid, {30, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, hosp.getPatientName(b->getPatientId()), {110, y}, 15, 1, TEXT_PRI);
                DrawTextEx(font, amt, {320, y}, 15, 1, ACCENT2);
                char dateCol[30];
                myStrcpy(dateCol, b->getDate());
                double diff = Validator::daysBetween(b->getDate(), today);
                if (diff > 7.0)
                    myStrcat(dateCol, " [OVERDUE]");
                DrawTextEx(font, dateCol, {440, y}, 15, 1, diff > 7 ? DANGER : TEXT_PRI);
                y += 34;
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
                st.screen = Screen::ADMIN_MENU;
        }

        // ─────────────────────────────────────────────────────────────────────
        //  A_REMOVE_DOC
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::A_REMOVE_DOC)
        {
            DrawHeader("Remove Doctor", font, SW);
            float y = 90;
            for (int i = 0; i < hosp.doctors.size(); i++)
            {
                Doctor *d = hosp.doctors.getAll()[i];
                if (!d)
                    continue;
                char sid[8], fee[32];
                intToStr(d->getId(), sid);
                floatToStr(d->getFee(), fee);
                char line[200];
                myStrcpy(line, sid);
                myStrcat(line, "  ");
                myStrcat(line, d->getName());
                myStrcat(line, "  ");
                myStrcat(line, d->getSpecialization());
                myStrcat(line, "  PKR");
                myStrcat(line, fee);
                DrawTextEx(font, line, {30, y}, 15, 1, TEXT_PRI);
                y += 30;
            }
            DrawCard({(float)(SW / 2 - 200), y + 10, 400, 120});
            Rectangle r = {(float)(SW / 2 - 160), y + 50, 320, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
            st.inA.draw(r, "Enter Doctor ID to Remove", font);
            if (DangerButton({(float)(SW / 2 - 60), y + 108, 120, 40}, "Remove", font))
            {
                int did = strToInt(st.inA.buf);
                bool hasPending = false;
                for (int i = 0; i < hosp.appointments.size(); i++)
                {
                    Appointment *a = hosp.appointments.getAll()[i];
                    if (a && a->getDoctorId() == did && myStrcmp(a->getStatus(), "pending") == 0)
                    {
                        hasPending = true;
                        break;
                    }
                }
                if (hasPending)
                    myStrncpy(st.statusMsg, "Cannot remove: pending appointments exist.", 512);
                else
                {
                    Doctor *d = hosp.doctors.findById(did);
                    if (!d)
                        myStrncpy(st.statusMsg, "Doctor not found.", 512);
                    else
                    {
                        hosp.doctors.removeById(did);
                        delete d;
                        FileHandler::saveAllDoctors(hosp.doctors);
                        st.msg.show("Doctor removed.", Screen::ADMIN_MENU);
                    }
                }
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 160), y + 158}, 16, 1, DANGER);
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::ADMIN_MENU;
                st.statusMsg[0] = '\0';
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        //  D_HISTORY
        // ─────────────────────────────────────────────────────────────────────
        else if (st.screen == Screen::D_HISTORY)
        {
            DrawHeader("Patient Medical History", font, SW);
            DrawCard({(float)(SW / 2 - 220), 90, 440, 120});
            Rectangle r = {(float)(SW / 2 - 180), 150, 360, 44};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                st.inA.active = CheckCollisionPointRec(GetMousePosition(), r);
            st.inA.draw(r, "Patient ID", font);
            static int viewPid = 0;
            if (GuiButton({(float)(SW / 2 - 60), 220, 120, 40}, "View", font))
            {
                viewPid = strToInt(st.inA.buf);
                bool ok = false;
                Patient *pp = hosp.patients.findById(viewPid);
                if (pp)
                    for (int i = 0; i < hosp.appointments.size(); i++)
                    {
                        Appointment *a = hosp.appointments.getAll()[i];
                        if (a && a->getPatientId() == viewPid && a->getDoctorId() == st.loggedDoctorId &&
                            myStrcmp(a->getStatus(), "completed") == 0)
                        {
                            ok = true;
                            break;
                        }
                    }
                if (!ok)
                {
                    myStrncpy(st.statusMsg, "Access denied.", 512);
                    viewPid = 0;
                }
                else
                    st.statusMsg[0] = '\0';
            }
            if (st.statusMsg[0])
                DrawTextEx(font, st.statusMsg, {(float)(SW / 2 - 180), 270}, 16, 1, DANGER);
            if (viewPid > 0)
            {
                float y = 290;
                for (int i = 0; i < hosp.prescriptions.size(); i++)
                {
                    Prescription *rx = hosp.prescriptions.getAll()[i];
                    if (!rx || rx->getPatientId() != viewPid || rx->getDoctorId() != st.loggedDoctorId)
                        continue;
                    DrawCard({20, y, (float)(SW - 40), 80});
                    DrawTextEx(font, rx->getDate(), {35, y + 8}, 15, 1, ACCENT2);
                    DrawTextEx(font, rx->getMedicines(), {150, y + 8}, 14, 1, TEXT_PRI);
                    DrawTextEx(font, rx->getNotes(), {35, y + 36}, 14, 1, TEXT_SEC);
                    y += 90;
                }
            }
            if (GuiButton({SW - 130, (float)(SH - 60), 110, 40}, "Back", font, {60, 70, 120, 255}, {80, 90, 150, 255}))
            {
                st.screen = Screen::DOCTOR_MENU;
                st.statusMsg[0] = '\0';
                viewPid = 0;
            }
        }

    end_draw:
        EndDrawing();
    }

    hosp.freeAll();
    CloseWindow();
    return 0;
}