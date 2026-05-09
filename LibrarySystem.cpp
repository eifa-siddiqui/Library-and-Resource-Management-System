#include "LibrarySystem.h"
#include "FineUtility.h"
#include "BorrowLimitException.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>
using namespace std;

// ── Color codes ───────────────────────────────────────────────────────────────
#define RST "\033[0m"
#define RED "\033[31m"
#define GRN "\033[32m"
#define YLW "\033[33m"
#define BLU "\033[34m"
#define MAG "\033[35m"
#define CYN "\033[36m"

// ── String helpers ────────────────────────────────────────────────────────────

static string toLower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static bool contains(const string& field, const string& query) {
    return toLower(field).find(toLower(query)) != string::npos;
}

static vector<string> split(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (getline(ss, token, '|')) tokens.push_back(token);
    return tokens;
}

// ── Enum converters ───────────────────────────────────────────────────────────

static string categoryStr(BookCategory c) {
    return c == BookCategory::FICTION ? "FICTION" : "NON_FICTION";
}
static BookCategory categoryFromStr(const string& s) {
    return s == "FICTION" ? BookCategory::FICTION : BookCategory::NON_FICTION;
}
static string statusStr(MembershipStatus s) {
    if (s == PREMIUM) return "PREMIUM";
    if (s == ELITE)   return "ELITE";
    return "STANDARD";
}
static MembershipStatus statusFromStr(const string& s) {
    if (s == "PREMIUM") return PREMIUM;
    if (s == "ELITE")   return ELITE;
    return STANDARD;
}

// ── Input helpers ─────────────────────────────────────────────────────────────

static int readInt(const string& prompt) {
    int n;
    while (true) {
        cout << prompt;
        if (cin >> n) { cin.ignore(10000, '\n'); return n; }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << RED "Invalid input. Enter a number: " RST;
    }
}

static string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return s;
}

static string readEmail(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        if (s.find('@') != string::npos) return s;
        cout << RED "Invalid email - must contain '@'. Try again.\n" RST;
    }
}

// ── Display helpers ───────────────────────────────────────────────────────────

static void printResourceList(const vector<LibraryResource*>& results, const string& query) {
    if (results.empty())
        cout << RED "No resources found matching '" RST << query << RED "'.\n" RST;
    else
        for (LibraryResource* r : results) r->displayinfo();
}

static void showMemberDetails(Member* m) {
    int active = 0, total = (int)m->getBorrowedBooks().size();
    for (BorrowRecord* r : m->getBorrowedBooks())
        if (!r->getIsReturned()) active++;

    cout << YLW "\n--- Member Details ---\n" RST;
    cout << BLU "ID      : " RST << m->getId()        << "\n";
    cout << BLU "Name    : " RST << m->getFirstName() << " " << m->getLastName() << "\n";
    cout << BLU "Email   : " RST << m->getEmail()     << "\n";
    cout << BLU "Address : " RST << m->getAddress()   << "\n";
    cout << BLU "Balance : " RST << "PKR " << m->getBalance() << "\n";
    cout << BLU "Status  : " RST << statusStr(m->getStatus()) << "\n";
    cout << BLU "Borrows : " RST << active << " active, " << total << " total\n";
}

static void showResourceDetails(LibraryResource* r) {
    cout << YLW "\n--- Resource Details ---\n" RST;
    cout << BLU "ISBN     : " RST << r->getIsbn()            << "\n";
    cout << BLU "Title    : " RST << r->getTitle()           << "\n";
    cout << BLU "Author   : " RST << r->getWriter()          << "\n";
    cout << BLU "Year     : " RST << r->getPublicationYear() << "\n";
    cout << BLU "Origin   : " RST << r->getOrigin()          << "\n";
    cout << BLU "Language : " RST << r->getLanguage()        << "\n";
    cout << BLU "Genre    : " RST << r->getGenre()           << "\n";
    cout << BLU "Type     : " RST << r->getType()            << "\n";
    cout << BLU "Available: " RST << (r->isAvailable() ? "Yes" : "No") << "\n";

    string type = r->getType();
    if (type == "SCIENCE")
        cout << BLU "Field    : " RST << dynamic_cast<ScienceBook*>(r)->getScientificField() << "\n";
    else if (type == "LITERATURE")
        cout << BLU "Era      : " RST << dynamic_cast<LiteratureBook*>(r)->getLiteraryEra() << "\n";
    else if (type == "MAGAZINE") {
        cout << BLU "Issue No : " RST << dynamic_cast<Magazine*>(r)->getIssueNumber()      << "\n";
        cout << BLU "Month    : " RST << dynamic_cast<Magazine*>(r)->getPublicationMonth() << "\n";
    }
    else if (type == "REFERENCE")
        cout << BLU "Edition  : " RST << dynamic_cast<ReferenceBook*>(r)->getEdition() << "\n";
    else if (type == "DIGITAL") {
        cout << BLU "Runtime  : " RST << dynamic_cast<DigitalMedia*>(r)->getRunTimeMinutes() << " min\n";
        cout << BLU "Format   : " RST << dynamic_cast<DigitalMedia*>(r)->getFileFormat()     << "\n";
    }

    const auto& reviews = r->getReviews();
    if (!reviews.empty()) {
        cout << BLU "Reviews  : " RST << reviews.size() << " review(s)\n";
        for (const Review& rv : reviews)
            cout << "  [" << rv.getRating() << "/5] " << rv.getComment() << "\n";
    }
}

// ── Constructor / Destructor ──────────────────────────────────────────────────

LibrarySystem::LibrarySystem() : currentUser(nullptr) {}

LibrarySystem::~LibrarySystem() {
    for (User* u            : users)     delete u;
    for (LibraryResource* r : resources) delete r;
}

// ── User management ───────────────────────────────────────────────────────────

bool LibrarySystem::addUser(User* u) {
    if (findUser(u->getId())) {
        cout << RED "User ID " RST << u->getId() << RED " already exists.\n" RST;
        delete u;
        return false;
    }
    users.push_back(u);
    return true;
}

void LibrarySystem::removeUser(const string& id) {
    for (int i = 0; i < (int)users.size(); i++) {
        if (users[i]->getId() == id) {
            delete users[i];
            users.erase(users.begin() + i);
            return;
        }
    }
    cout << RED "User " RST << id << RED " not found.\n" RST;
}

User* LibrarySystem::findUser(const string& id) const {
    for (User* u : users)
        if (u->getId() == id) return u;
    return nullptr;
}

User* LibrarySystem::loginCheck(const string& id, const string& pass) const {
    User* u = findUser(id);
    if (u && u->checkPassword(pass)) return u;
    return nullptr;
}

vector<Member*> LibrarySystem::getAllMembers() const {
    vector<Member*> members;
    for (User* u : users) {
        Member* m = dynamic_cast<Member*>(u);
        if (m) members.push_back(m);
    }
    return members;
}

void LibrarySystem::displayAllUsers() const {
    if (users.empty()) { cout << YLW "No users registered.\n" RST; return; }
    for (User* u : users)
        cout << CYN "[" RST << u->getRole() << CYN "] " RST
             << u->getId() << " - " << u->getFirstName() << " " << u->getLastName() << "\n";
}

// ── Resource management ───────────────────────────────────────────────────────

bool LibrarySystem::addResource(LibraryResource* r) {
    if (findResource(r->getIsbn())) {
        cout << RED "ISBN " RST << r->getIsbn() << RED " already exists.\n" RST;
        delete r;
        return false;
    }
    resources.push_back(r);
    return true;
}

void LibrarySystem::removeResource(const string& isbn) {
    for (int i = 0; i < (int)resources.size(); i++) {
        if (resources[i]->getIsbn() == isbn) {
            if (!resources[i]->isAvailable()) {
                cout << RED "Cannot remove \"" RST << resources[i]->getTitle()
                     << RED "\" - it is currently issued.\n" RST;
                return;
            }
            delete resources[i];
            resources.erase(resources.begin() + i);
            cout << GRN "Resource removed.\n" RST;
            return;
        }
    }
    cout << RED "Resource " RST << isbn << RED " not found.\n" RST;
}

LibraryResource* LibrarySystem::findResource(const string& isbn) const {
    for (LibraryResource* r : resources)
        if (r->getIsbn() == isbn) return r;
    return nullptr;
}

void LibrarySystem::showAvailableResources() const {
    bool any = false;
    for (LibraryResource* r : resources)
        if (r->isAvailable()) { r->displayinfo(); any = true; }
    if (!any) cout << YLW "No resources currently available.\n" RST;
}

void LibrarySystem::showAllResources() const {
    if (resources.empty()) { cout << YLW "No resources in the library.\n" RST; return; }
    for (LibraryResource* r : resources) r->displayinfo();
}

// ── Search ────────────────────────────────────────────────────────────────────

vector<LibraryResource*> LibrarySystem::searchByTitle(const string& q) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (contains(r->getTitle(), q)) res.push_back(r);
    return res;
}

vector<LibraryResource*> LibrarySystem::searchByWriter(const string& q) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (contains(r->getWriter(), q)) res.push_back(r);
    return res;
}

vector<LibraryResource*> LibrarySystem::searchByGenre(const string& q) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (contains(r->getGenre(), q)) res.push_back(r);
    return res;
}

vector<LibraryResource*> LibrarySystem::searchByLanguage(const string& q) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (contains(r->getLanguage(), q)) res.push_back(r);
    return res;
}

vector<LibraryResource*> LibrarySystem::searchByYear(int year) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (r->getPublicationYear() == year) res.push_back(r);
    return res;
}

vector<LibraryResource*> LibrarySystem::searchByKeyword(const string& kw) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (*r == kw) res.push_back(r);
    return res;
}

// ── File I/O ──────────────────────────────────────────────────────────────────

bool LibrarySystem::dataFilesExist() const {
    ifstream f("data/users.txt");
    return f.good();
}

void LibrarySystem::ensureDataFolder() const {
    _mkdir("data");
}

// Format: MEMBER|id|firstName|lastName|password|email|address|balance|status
//         ADMIN|id|firstName|lastName|password|email|address
void LibrarySystem::saveUsers(const string& filename) const {
    ofstream f(filename);
    for (User* u : users) {
        if (u->getRole() == "MEMBER") {
            Member* m = dynamic_cast<Member*>(u);
            f << "MEMBER|" << m->getId()        << "|" << m->getFirstName() << "|"
              << m->getLastName()                << "|" << m->getPassword()  << "|"
              << m->getEmail()                   << "|" << m->getAddress()   << "|"
              << m->getBalance()                 << "|" << statusStr(m->getStatus()) << "\n";
        } else {
            f << "ADMIN|"  << u->getId()         << "|" << u->getFirstName() << "|"
              << u->getLastName()                 << "|" << u->getPassword()  << "|"
              << u->getEmail()                    << "|" << u->getAddress()   << "\n";
        }
    }
}

void LibrarySystem::loadUsers(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) return;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> t = split(line);
        if (t[0] == "MEMBER" && t.size() >= 9) {
            Member* m = new Member(t[1], t[2], t[3], t[4], t[5], t[6]);
            m->setBalance(stod(t[7]));
            m->setStatus(statusFromStr(t[8]));
            users.push_back(m);
        } else if (t[0] == "ADMIN" && t.size() >= 7) {
            users.push_back(new Admin(t[1], t[2], t[3], t[4], t[5], t[6]));
        }
    }
}

// Format: TYPE|isbn|title|writer|year|origin|language|genre|category|isAvailable|[extras]
void LibrarySystem::saveResources(const string& filename) const {
    ofstream f(filename);
    for (LibraryResource* r : resources) {
        f << r->getType()            << "|" << r->getIsbn()            << "|"
          << r->getTitle()           << "|" << r->getWriter()          << "|"
          << r->getPublicationYear() << "|" << r->getOrigin()          << "|"
          << r->getLanguage()        << "|" << r->getGenre()           << "|"
          << categoryStr(r->getCategory()) << "|" << (r->isAvailable() ? "1" : "0");

        string type = r->getType();
        if (type == "SCIENCE")
            f << "|" << dynamic_cast<ScienceBook*>(r)->getScientificField();
        else if (type == "LITERATURE")
            f << "|" << dynamic_cast<LiteratureBook*>(r)->getLiteraryEra();
        else if (type == "MAGAZINE")
            f << "|" << dynamic_cast<Magazine*>(r)->getIssueNumber()
              << "|" << dynamic_cast<Magazine*>(r)->getPublicationMonth();
        else if (type == "REFERENCE")
            f << "|" << dynamic_cast<ReferenceBook*>(r)->getEdition();
        else if (type == "DIGITAL")
            f << "|" << dynamic_cast<DigitalMedia*>(r)->getRunTimeMinutes()
              << "|" << dynamic_cast<DigitalMedia*>(r)->getFileFormat();
        f << "\n";
    }
}

void LibrarySystem::loadResources(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) return;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> t = split(line);
        if (t.size() < 10) continue;
        string isbn = t[1], title = t[2], writer = t[3];
        int    year = stoi(t[4]);
        string origin = t[5], language = t[6], genre = t[7];
        BookCategory cat = categoryFromStr(t[8]);
        bool avail = (t[9] == "1");
        string type = t[0];

        LibraryResource* r = nullptr;
        if      (type == "SCIENCE"    && t.size() >= 11)
            r = new ScienceBook(isbn, title, writer, year, origin, language, genre, cat, t[10]);
        else if (type == "LITERATURE" && t.size() >= 11)
            r = new LiteratureBook(isbn, title, writer, year, origin, language, genre, cat, t[10]);
        else if (type == "MAGAZINE"   && t.size() >= 12)
            r = new Magazine(isbn, title, writer, year, origin, language, genre, cat, stoi(t[10]), t[11]);
        else if (type == "REFERENCE"  && t.size() >= 11)
            r = new ReferenceBook(isbn, title, writer, year, origin, language, genre, cat, stoi(t[10]));
        else if (type == "DIGITAL"    && t.size() >= 12)
            r = new DigitalMedia(isbn, title, writer, year, origin, language, genre, cat, stof(t[10]), t[11]);

        if (r) { r->setAvailable(avail); resources.push_back(r); }
    }
}

// Format: memberID|isbn|issueDate|dueDate|isReturned|returnDate
void LibrarySystem::saveBorrows(const string& filename) const {
    ofstream f(filename);
    for (User* u : users) {
        Member* m = dynamic_cast<Member*>(u);
        if (!m) continue;
        for (BorrowRecord* rec : m->getBorrowedBooks()) {
            f << m->getId()                          << "|"
              << rec->getResource()->getIsbn()        << "|"
              << rec->getIssueDate()                  << "|"
              << rec->getDueDate()                    << "|"
              << (rec->getIsReturned() ? "1" : "0")  << "|"
              << rec->getReturnDate()                 << "\n";
        }
    }
}

void LibrarySystem::loadBorrows(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) return;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> t = split(line);
        if (t.size() < 6) continue;
        Member*          m   = dynamic_cast<Member*>(findUser(t[0]));
        LibraryResource* res = findResource(t[1]);
        if (!m || !res) continue;
        BorrowRecord* rec = new BorrowRecord(t[0], res, stoll(t[2]), stoll(t[3]));
        rec->setIsReturned(t[4] == "1");
        rec->setReturnDate(stoll(t[5]));
        m->addBorrowRecord(rec);
    }
}

// ── Persistence entry points ──────────────────────────────────────────────────

void LibrarySystem::loadAllData() {
    ensureDataFolder();
    if (!dataFilesExist()) { seedDefaultData(); return; }
    loadUsers("data/users.txt");
    loadResources("data/resources.txt");
    loadBorrows("data/borrows.txt");
}

void LibrarySystem::saveAllData() const {
    ensureDataFolder();
    saveUsers("data/users.txt");
    saveResources("data/resources.txt");
    saveBorrows("data/borrows.txt");
}

// ── Seed data (first run only) ────────────────────────────────────────────────

void LibrarySystem::seedDefaultData() {
    cout << YLW "\n==============================\n" RST;
    cout << BLU "  WELCOME - FIRST TIME SETUP\n" RST;
    cout << YLW "==============================\n" RST;
    cout << RED "No accounts found. Please create the Admin account.\n\n" RST;

    string id    = readLine(MAG "Admin ID    : " RST);
    string first = readLine(MAG "First Name  : " RST);
    string last  = readLine(MAG "Last Name   : " RST);
    string pass  = readLine(MAG "Password    : " RST);
    string email = readEmail(MAG "Email       : " RST);
    string addr  = readLine(MAG "Address     : " RST);

    users.push_back(new Admin(id, first, last, pass, email, addr));
    cout << GRN "\nAdmin account created successfully.\n" RST;

    resources.push_back(new ScienceBook("S001", "Introduction to Physics", "Halliday",
                                         2020, "USA", "English", "Physics",
                                         BookCategory::NON_FICTION, "Mechanics"));
    resources.push_back(new LiteratureBook("L001", "Pride and Prejudice", "Austen",
                                            1813, "UK", "English", "Romance",
                                            BookCategory::FICTION, "Regency"));
    resources.push_back(new Magazine("M001", "Science Today", "Staff",
                                      2024, "USA", "English", "Science",
                                      BookCategory::NON_FICTION, 5, "April"));
    cout << GRN "3 sample resources added. You can now log in as Admin.\n" RST;
    saveAllData();
}

vector<LibraryResource*> LibrarySystem::getAllResources() const { return resources; }

// =============================================================================
// run()
// =============================================================================

void LibrarySystem::run() {
    loadAllData();

    while (true) {
        cout << YLW "\n==============================\n" RST;
        cout << BLU "  LIBRARY MANAGEMENT SYSTEM\n" RST;
        cout << YLW "==============================\n" RST;
        cout << CYN "[1] Admin Login\n" RST;
        cout << CYN "[2] Member Login\n" RST;
        cout << CYN "[3] Exit\n" RST;
        int choice = readInt(MAG "Enter choice: " RST);

        if      (choice == 1) loginMenu("ADMIN");
        else if (choice == 2) loginMenu("MEMBER");
        else if (choice == 3) break;
        else cout << RED "Invalid choice.\n" RST;
    }

    saveAllData();
    cout << GRN "Data saved. Goodbye!\n" RST;
}

// =============================================================================
// loginMenu()
// =============================================================================

void LibrarySystem::loginMenu(const string& role) {
    cout << YLW "\n--- " RST << role << YLW " LOGIN ---\n" RST;
    string id   = readLine(MAG "ID       : " RST);
    string pass = readLine(MAG "Password : " RST);

    User* u = loginCheck(id, pass);
    if (!u) { cout << RED "Invalid credentials.\n" RST; return; }

    if (u->getRole() != role) {
        cout << RED "This ID is not registered as " RST << role << ".\n";
        return;
    }

    currentUser = u;
    if (role == "MEMBER") memberMenu(dynamic_cast<Member*>(u));
    else                  adminMenu(dynamic_cast<Admin*>(u));

    currentUser = nullptr;
    saveAllData();
}

// =============================================================================
// memberMenu()
// =============================================================================

void LibrarySystem::memberMenu(Member* m) {
    while (true) {
        cout << YLW "\n==============================\n" RST;
        cout << BLU "  MEMBER DASHBOARD\n" RST;
        cout << GRN "  Welcome, " RST << m->getFirstName()
             << GRN "  |  Balance: PKR " RST << m->getBalance() << "\n";
        cout << YLW "==============================\n" RST;
        cout << CYN "[1]  Browse Available Resources\n" RST;
        cout << CYN "[2]  Search Resources\n" RST;
        cout << CYN "[3]  Issue a Resource\n" RST;
        cout << CYN "[4]  Return a Resource\n" RST;
        cout << CYN "[5]  Reserve a Resource\n" RST;
        cout << CYN "[6]  View Borrowing History\n" RST;
        cout << CYN "[7]  View Balance\n" RST;
        cout << CYN "[8]  Deposit Amount\n" RST;
        cout << CYN "[9]  Write a Review\n" RST;
        cout << CYN "[10] Logout\n" RST;
        int choice = readInt(MAG "Enter choice: " RST);

        if (choice == 1) {
            showAvailableResources();
        }
        else if (choice == 2) {
            searchMenu();
        }
        else if (choice == 3) {
            cout << YLW "\nAvailable Resources:\n" RST;
            showAvailableResources();
            string isbn = readLine(MAG "\nEnter ISBN to issue: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            try   { m->issueBook(r); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 4) {
            cout << YLW "\nYour Currently Borrowed Resources:\n" RST;
            m->viewBorrowHistory();
            string isbn = readLine(MAG "\nEnter ISBN to return: " RST);
            m->returnBook(isbn);
        }
        else if (choice == 5) {
            cout << YLW "\nAll Resources:\n" RST;
            showAllResources();
            string isbn = readLine(MAG "\nEnter ISBN to reserve: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            m->reserveBook(r);
        }
        else if (choice == 6) { m->viewBorrowHistory(); }
        else if (choice == 7) { cout << GRN "Balance: PKR " RST << m->getBalance() << "\n"; }
        else if (choice == 8) {
            double amount = readInt(MAG "Amount to deposit (PKR): " RST);
            try   { m->depositAmount(amount); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 9) {
            string isbn = readLine(MAG "Enter ISBN of resource to review: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            if (!m->findBorrowRecord(isbn)) {
                cout << RED "You can only review resources you have borrowed.\n" RST;
                continue;
            }
            int rating = readInt(MAG "Rating (1-5): " RST);
            if (rating < 1 || rating > 5) { cout << RED "Rating must be 1-5.\n" RST; continue; }
            string comment = readLine(MAG "Comment: " RST);
            static int reviewCount = 0;
            r->addReview(Review("R" + to_string(++reviewCount), m->getId(), isbn, rating, comment));
            cout << GRN "Review submitted.\n" RST;
        }
        else if (choice == 10) { cout << YLW "Logged out.\n" RST; return; }
        else cout << RED "Invalid choice.\n" RST;
    }
}

// =============================================================================
// searchMenu() — resource search, used by both member and admin
// =============================================================================

void LibrarySystem::searchMenu() {
    cout << YLW "\n--- Search Resources ---\n" RST;
    cout << CYN "[1] Title\n" RST;
    cout << CYN "[2] Author\n" RST;
    cout << CYN "[3] Genre\n" RST;
    cout << CYN "[4] Language\n" RST;
    cout << CYN "[5] Year\n" RST;
    cout << CYN "[6] ISBN\n" RST;
    cout << CYN "[7] Country of Origin\n" RST;
    cout << CYN "[8] Resource Type\n" RST;
    cout << CYN "[9] Availability\n" RST;
    cout << CYN "[0] Back\n" RST;
    int choice = readInt(MAG "Enter choice: " RST);

    if (choice == 0) { return; }

    if (choice == 1) {
        string q = readLine(MAG "Title: " RST);
        printResourceList(searchByTitle(q), q);
    }
    else if (choice == 2) {
        string q = readLine(MAG "Author: " RST);
        printResourceList(searchByWriter(q), q);
    }
    else if (choice == 3) {
        string q = readLine(MAG "Genre: " RST);
        printResourceList(searchByGenre(q), q);
    }
    else if (choice == 4) {
        string q = readLine(MAG "Language: " RST);
        printResourceList(searchByLanguage(q), q);
    }
    else if (choice == 5) {
        int year = readInt(MAG "Year: " RST);
        printResourceList(searchByYear(year), to_string(year));
    }
    else if (choice == 6) {
        string q = readLine(MAG "ISBN (partial): " RST);
        vector<LibraryResource*> res;
        for (LibraryResource* r : resources)
            if (contains(r->getIsbn(), q)) res.push_back(r);
        printResourceList(res, q);
    }
    else if (choice == 7) {
        string q = readLine(MAG "Country of Origin: " RST);
        vector<LibraryResource*> res;
        for (LibraryResource* r : resources)
            if (contains(r->getOrigin(), q)) res.push_back(r);
        printResourceList(res, q);
    }
    else if (choice == 8) {
        cout << CYN "[1] Science  [2] Literature  [3] Magazine  [4] Reference  [5] Digital\n" RST;
        int t = readInt(MAG "Type: " RST);
        string types[] = {"SCIENCE", "LITERATURE", "MAGAZINE", "REFERENCE", "DIGITAL"};
        if (t < 1 || t > 5) { cout << RED "Invalid choice.\n" RST; return; }
        string tstr = types[t - 1];
        vector<LibraryResource*> res;
        for (LibraryResource* r : resources)
            if (r->getType() == tstr) res.push_back(r);
        printResourceList(res, tstr);
    }
    else if (choice == 9) {
        cout << CYN "[1] Available only  [2] All resources\n" RST;
        int t = readInt(MAG "Choice: " RST);
        if (t == 1) showAvailableResources();
        else        showAllResources();
    }
    else { cout << RED "Invalid choice.\n" RST; }
}

// =============================================================================
// adminMenu()
// =============================================================================

void LibrarySystem::adminMenu(Admin* a) {
    while (true) {
        cout << YLW "\n==============================\n" RST;
        cout << BLU "  ADMIN DASHBOARD\n" RST;
        cout << GRN "  Logged in as: " RST << a->getFirstName() << "\n";
        cout << YLW "==============================\n" RST;
        cout << CYN "[1] Manage Members\n" RST;
        cout << CYN "[2] Manage Resources\n" RST;
        cout << CYN "[3] Process a Return\n" RST;
        cout << CYN "[4] View Overdue Dashboard\n" RST;
        cout << CYN "[5] Generate Member Report\n" RST;
        cout << CYN "[6] Generate Resource Report\n" RST;
        cout << CYN "[7] Logout\n" RST;
        int choice = readInt(MAG "Enter choice: " RST);

        if      (choice == 1) manageMembersMenu(a);
        else if (choice == 2) manageResourcesMenu(a);
        else if (choice == 3) processReturnFlow(a);
        else if (choice == 4) viewOverdueFlow();
        else if (choice == 5) {
            cout << CYN "[1] By ID  [2] By Name  [3] All Members\n" RST;
            int sc = readInt(MAG "Choice: " RST);
            vector<Member*> targets;
            if (sc == 1) {
                string id = readLine(MAG "Member ID: " RST);
                Member* m = dynamic_cast<Member*>(findUser(id));
                if (!m) { cout << RED "Member not found.\n" RST; }
                else targets.push_back(m);
            } else if (sc == 2) {
                string q = readLine(MAG "Name (full or partial): " RST);
                for (Member* m : getAllMembers())
                    if (contains(m->getFirstName() + " " + m->getLastName(), q))
                        targets.push_back(m);
                if (targets.empty()) cout << RED "No matching members found.\n" RST;
            } else {
                targets = getAllMembers();
            }
            if (!targets.empty()) a->generateMemberReport(targets, "");
        }
        else if (choice == 6) a->generateResourceReport(getAllResources(), getAllMembers(), "data/resource_report.txt");
        else if (choice == 7) { cout << GRN "Logged out.\n" RST; return; }
        else cout << RED "Invalid choice.\n" RST;
    }
}

// =============================================================================
// manageMembersMenu()
// =============================================================================

void LibrarySystem::manageMembersMenu(Admin* a) {
    (void)a;
    while (true) {
        cout << YLW "\n--- Manage Members ---\n" RST;
        cout << CYN "[1] Add Member\n" RST;
        cout << CYN "[2] Remove Member\n" RST;
        cout << CYN "[3] View Member Details\n" RST;
        cout << CYN "[4] Search Members\n" RST;
        cout << CYN "[5] View Borrow History\n" RST;
        cout << CYN "[6] Display All Members\n" RST;
        cout << CYN "[7] Update Member Status\n" RST;
        cout << CYN "[8] Back\n" RST;
        int choice = readInt(MAG "Enter choice: " RST);

        if (choice == 1) {
            string id    = readLine(MAG "Member ID   : " RST);
            string first = readLine(MAG "First Name  : " RST);
            string last  = readLine(MAG "Last Name   : " RST);
            string pass  = readLine(MAG "Password    : " RST);
            string email = readEmail(MAG "Email       : " RST);
            string addr  = readLine(MAG "Address     : " RST);
            double bal   = readInt(MAG "Balance(PKR): " RST);
            Member* m = new Member(id, first, last, pass, email, addr);
            m->setBalance(bal);
            if (addUser(m)) cout << GRN "Member added.\n" RST;
        }
        else if (choice == 2) {
            string id = readLine(MAG "Member ID to remove: " RST);
            Member* m = dynamic_cast<Member*>(findUser(id));
            if (!m) { cout << RED "Member not found.\n" RST; continue; }
            showMemberDetails(m);
            int active = 0;
            for (BorrowRecord* r : m->getBorrowedBooks())
                if (!r->getIsReturned()) active++;
            if (active > 0)
                cout << RED "Warning: member has " RST << active << RED " active borrow(s).\n" RST;
            cout << MAG "Confirm removal? [Y/N]: " RST;
            string ans; getline(cin, ans);
            if (ans != "Y" && ans != "y") { cout << GRN "Removal cancelled.\n" RST; continue; }
            else
            cout<<"Member removed successfully!"<<endl;
            removeUser(id);
        }
        else if (choice == 3) {
            string id = readLine(MAG "Member ID: " RST);
            Member* m = dynamic_cast<Member*>(findUser(id));
            if (!m) { cout << RED "Member not found.\n" RST; continue; }
            showMemberDetails(m);
        }
        else if (choice == 4) {
            adminMemberSearch();
        }
        else if (choice == 5) {
            string id = readLine(MAG "Member ID: " RST);
            Member* m = dynamic_cast<Member*>(findUser(id));
            if (!m) { cout << RED "Member not found.\n" RST; continue; }
            m->viewBorrowHistory();
        }
        else if (choice == 6) {
            displayAllUsers();
        }
        else if (choice == 7) {
            string id = readLine(MAG "Member ID: " RST);
            Member* m = dynamic_cast<Member*>(findUser(id));
            if (!m) { cout << RED "Member not found.\n" RST; continue; }
            cout << BLU "Current status: " RST << (m->getStatus() == STANDARD ? "STANDARD" :
                                                    m->getStatus() == PREMIUM  ? "PREMIUM"  : "ELITE") << "\n";
            cout << CYN "[1] Standard  [2] Premium  [3] Elite\n" RST;
            int sc = readInt(MAG "New status: " RST);
            if      (sc == 1) { m->setStatus(STANDARD); cout << GRN "Status set to STANDARD.\n" RST; }
            else if (sc == 2) { m->setStatus(PREMIUM);  cout << GRN "Status set to PREMIUM.\n"  RST; }
            else if (sc == 3) { m->setStatus(ELITE);    cout << GRN "Status set to ELITE.\n"    RST; }
            else cout << RED "Invalid choice.\n" RST;
        }
        else if (choice == 8) { return; }
        else cout << RED "Invalid choice.\n" RST;
    }
}

// =============================================================================
// adminMemberSearch() — search members by ID, name, or email
// =============================================================================

void LibrarySystem::adminMemberSearch() {
    cout << YLW "\n--- Search Members ---\n" RST;
    cout << CYN "[1] By ID\n[2] By Name\n[3] By Email\n[0] Back\n" RST;
    int sc = readInt(MAG "Choice: " RST);
    if (sc == 0) return;

    if (sc == 1) {
        string q = readLine(MAG "Member ID: " RST);
        Member* m = dynamic_cast<Member*>(findUser(q));
        if (m) showMemberDetails(m);
        else   cout << RED "No member found with that ID.\n" RST;
    }
    else if (sc == 2) {
        string q = readLine(MAG "Name (full or partial): " RST);
        bool found = false;
        for (Member* m : getAllMembers()) {
            if (contains(m->getFirstName() + " " + m->getLastName(), q)) {
                showMemberDetails(m);
                found = true;
            }
        }
        if (!found) cout << RED "No member found with that name.\n" RST;
    }
    else if (sc == 3) {
        string q = readLine(MAG "Email (partial): " RST);
        bool found = false;
        for (Member* m : getAllMembers()) {
            if (contains(m->getEmail(), q)) {
                showMemberDetails(m);
                found = true;
            }
        }
        if (!found) cout << RED "No member found with that email.\n" RST;
    }
    else { cout << RED "Invalid choice.\n" RST; }
}

// =============================================================================
// manageResourcesMenu()
// =============================================================================

void LibrarySystem::manageResourcesMenu(Admin* a) {
    (void)a;
    while (true) {
        cout << YLW "\n--- Manage Resources ---\n" RST;
        cout << CYN "[1] Add Resource\n" RST;
        cout << CYN "[2] Remove Resource\n" RST;
        cout << CYN "[3] Toggle Availability\n" RST;
        cout << CYN "[4] Show All Resources\n" RST;
        cout << CYN "[5] Search Resources\n" RST;
        cout << CYN "[6] View Resource Details\n" RST;
        cout << CYN "[7] Back\n" RST;
        int choice = readInt(MAG "Enter choice: " RST);

        if (choice == 1) {
            LibraryResource* r = promptNewResource();
            if (r && addResource(r)) cout << GRN "Resource added.\n" RST;
        }
        else if (choice == 2) {
            string isbn = readLine(MAG "ISBN to remove: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            if (!r->isAvailable()) {
                cout << RED "Cannot remove \"" RST << r->getTitle()
                     << RED "\" - it is currently issued.\n" RST;
                continue;
            }
            showResourceDetails(r);
            cout << MAG "Confirm removal? [Y/N]: " RST;
            string ans; getline(cin, ans);
            if (ans != "Y" && ans != "y") { cout << GRN "Removal cancelled.\n" RST; continue; }
            removeResource(isbn);
        }
        else if (choice == 3) {
            string isbn = readLine(MAG "ISBN: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            r->setAvailable(!r->isAvailable());
            cout << (r->isAvailable() ? GRN "Now: Available\n" RST : YLW "Now: Unavailable\n" RST);
        }
        else if (choice == 4) { showAllResources(); }
        else if (choice == 5) { searchMenu(); }
        else if (choice == 6) {
            string isbn = readLine(MAG "ISBN: " RST);
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << RED "Resource not found.\n" RST; continue; }
            showResourceDetails(r);
        }
        else if (choice == 7) { return; }
        else cout << RED "Invalid choice.\n" RST;
    }
}

// =============================================================================
// promptNewResource() — asks for fields and returns a new resource object
// Category is auto-assigned based on resource type (no need to ask separately)
// =============================================================================

LibraryResource* LibrarySystem::promptNewResource() {
    cout << YLW "\n--- Resource Type ---\n" RST;
    cout << CYN "[1] Science Book\n[2] Literature Book\n[3] Magazine\n" RST;
    cout << CYN "[4] Reference Book\n[5] Digital Media\n[0] Cancel\n" RST;
    int type = readInt(MAG "Enter choice: " RST);
    if (type == 0) return nullptr;
    if (type < 1 || type > 5) { cout << RED "Invalid type.\n" RST; return nullptr; }

    // Category is determined automatically by type:
    // Science/Magazine/Reference/Digital = Non-Fiction, Literature = Fiction
    BookCategory cat = (type == 2) ? BookCategory::FICTION : BookCategory::NON_FICTION;

    string isbn   = readLine(MAG "ISBN            : " RST);
    string title  = readLine(MAG "Title           : " RST);
    string writer = readLine(MAG "Author/Writer   : " RST);
    int    year   = readInt(MAG  "Publication Year: " RST);
    string origin = readLine(MAG "Country of Origin: " RST);
    string lang   = readLine(MAG "Language        : " RST);
    string genre  = readLine(MAG "Genre           : " RST);

    if (type == 1) {
        string field = readLine(MAG "Scientific Field: " RST);
        return new ScienceBook(isbn, title, writer, year, origin, lang, genre, cat, field);
    }
    if (type == 2) {
        string era = readLine(MAG "Literary Era    : " RST);
        return new LiteratureBook(isbn, title, writer, year, origin, lang, genre, cat, era);
    }
    if (type == 3) {
        int    issue = readInt(MAG  "Issue Number    : " RST);
        string month = readLine(MAG "Publication Month: " RST);
        return new Magazine(isbn, title, writer, year, origin, lang, genre, cat, issue, month);
    }
    if (type == 4) {
        int edition = readInt(MAG "Edition         : " RST);
        return new ReferenceBook(isbn, title, writer, year, origin, lang, genre, cat, edition);
    }
    // type == 5: Digital Media
    float  runtime = (float)readInt(MAG "Runtime (minutes): " RST);
    string format  = readLine(MAG "File Format (e.g. MP4): " RST);
    return new DigitalMedia(isbn, title, writer, year, origin, lang, genre, cat, runtime, format);
}

// =============================================================================
// processReturnFlow()
// =============================================================================

void LibrarySystem::processReturnFlow(Admin* a) {
    string mid = readLine(MAG "Member ID: " RST);
    Member* m = dynamic_cast<Member*>(findUser(mid));
    if (!m) { cout << RED "Member not found.\n" RST; return; }

    string isbn = readLine(MAG "Resource ISBN: " RST);

    BorrowRecord* rec = m->findActiveBorrow(isbn);
    bool alreadyReturned = false;
    if (!rec) { rec = m->findBorrowRecord(isbn); alreadyReturned = true; }
    if (!rec) { cout << RED "No borrow record found for ISBN " RST << isbn << ".\n"; return; }
    if (alreadyReturned && rec->getResource()->isAvailable()) {
        cout << YLW "This resource has already been fully processed.\n" RST;
        return;
    }

    cout << BLU "\nResource : " RST << rec->getResource()->getTitle() << "\n";
    time_t issued = rec->getIssueDate();
    cout << BLU "Issued   : " RST << ctime(&issued);
    int lateDays = rec->calculateLateDays(time(nullptr));
    cout << BLU "Overdue  : " RST << lateDays << " day(s)\n";

    cout << YLW "\nCondition?\n" RST;
    cout << CYN "[1] Perfect  [2] Minor Damage  [3] Heavy Damage  [4] Lost\n" RST;
    int c = readInt(MAG "Enter choice: " RST);
    if (c < 1 || c > 4) { cout << RED "Invalid choice.\n" RST; return; }
    BookCondition cond = static_cast<BookCondition>(c - 1);

    if (!alreadyReturned) m->returnBook(isbn);
    a->processReturn(*m, rec, cond, time(nullptr));
    cout << GRN "Updated balance: PKR " RST << m->getBalance() << "\n";
}

// =============================================================================
// viewOverdueFlow()
// =============================================================================

void LibrarySystem::viewOverdueFlow() const {
    time_t now = time(nullptr);
    bool found = false;
    cout << YLW "\n=== OVERDUE RESOURCES ===\n" RST;
    for (Member* m : getAllMembers()) {
        for (BorrowRecord* r : m->getBorrowedBooks()) {
            if (!r->getIsReturned() && r->getDueDate() < now) {
                int days = r->calculateLateDays(now);
                double est = FineUtility::calculateFine(days, BookCondition::PERFECT);
                cout << BLU "Member: " RST << m->getFirstName() << " " << m->getLastName()
                     << " (" << m->getId() << ")\n";
                cout << "  \"" << r->getResource()->getTitle()
                     << "\" - overdue by " << RED << days << RST << " day(s)"
                     << " | Est. fine: " << RED "PKR " RST << est << "\n";
                found = true;
            }
        }
    }
    if (!found) cout << GRN "No overdue resources.\n" RST;
}
