#include "LibrarySystem.h"
#include "FineUtility.h"
#include "BorrowLimitException.h"
#include <iostream>
#include <fstream> //save loa data from files
#include <sstream>
#include <algorithm>
#include <direct.h>   // _mkdir for Windows/MinGW
using namespace std;

// ── Helpers ───────────────────────────────────────────────────────────────────

static string toLower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static bool contains(const string& field, const string& query) {
    return toLower(field).find(toLower(query)) != string::npos;
}

// Split a pipe-delimited line into tokens
static vector<string> split(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (getline(ss, token, '|')) tokens.push_back(token);
    return tokens;
}

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

// ── Input helpers (used throughout menus and first-run setup) ─────────────────

static int readInt(const string& prompt) {
    int n;
    while (true) {
        cout << prompt;
        if (cin >> n) { cin.ignore(10000, '\n'); return n; }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input. Please enter a number: ";
    }
}

static string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return s;
}

static void printResults(const vector<LibraryResource*>& results, const string& query) {
    if (results.empty())
        cout << "No resources found matching '" << query << "'.\n";
    else
        for (LibraryResource* r : results) r->displayinfo();
}

// ── Constructor / Destructor ──────────────────────────────────────────────────

LibrarySystem::LibrarySystem() : currentUser(nullptr) {}

LibrarySystem::~LibrarySystem() {
    for (User* u           : users)     delete u;
    for (LibraryResource* r : resources) delete r;
}

// ── User management ───────────────────────────────────────────────────────────

bool LibrarySystem::addUser(User* u) {
    if (findUser(u->getId())) {
        cout << "User ID " << u->getId() << " already exists.\n";
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
    cout << "User " << id << " not found.\n";
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
    for (User* u : users)
        cout << "  [" << u->getRole() << "] " << u->getId()
             << " — " << u->getFirstName() << " " << u->getLastName() << "\n";
}

// ── Resource management ───────────────────────────────────────────────────────

bool LibrarySystem::addResource(LibraryResource* r) {
    if (findResource(r->getIsbn())) {
        cout << "ISBN " << r->getIsbn() << " already exists.\n";
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
                cout << "Cannot remove \"" << resources[i]->getTitle()
                     << "\" — it is currently issued.\n";
                return;
            }
            delete resources[i];
            resources.erase(resources.begin() + i);
            return;
        }
    }
    cout << "Resource " << isbn << " not found.\n";
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
    if (!any) cout << "No resources currently available.\n";
}

void LibrarySystem::showAllResources() const {
    if (resources.empty()) { cout << "No resources in the library.\n"; return; }
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

// Uses LibraryResource::operator== for keyword search across isbn/title/writer/genre
vector<LibraryResource*> LibrarySystem::searchByKeyword(const string& kw) const {
    vector<LibraryResource*> res;
    for (LibraryResource* r : resources)
        if (*r == kw) res.push_back(r);
    return res;
}

// ── File I/O — plain fstream, pipe-delimited text files ───────────────────────

bool LibrarySystem::dataFilesExist() const {
    ifstream f("data/users.txt");
    return f.good();
}

void LibrarySystem::ensureDataFolder() const {
    _mkdir("data"); // no-op if folder already exists
}

// Format: MEMBER|id|firstName|lastName|password|email|address|balance|status
//         ADMIN|id|firstName|lastName|password|email|address
void LibrarySystem::saveUsers(const string& filename) const {
    ofstream f(filename);
    for (User* u : users) {
        if (u->getRole() == "MEMBER") {
            Member* m = dynamic_cast<Member*>(u);
            f << "MEMBER|" << m->getId() << "|" << m->getFirstName() << "|"
              << m->getLastName() << "|" << m->getPassword() << "|"
              << m->getEmail() << "|" << m->getAddress()
              << "|" << m->getBalance() << "|" << statusStr(m->getStatus()) << "\n";
        } else {
            f << "ADMIN|" << u->getId() << "|" << u->getFirstName() << "|"
              << u->getLastName() << "|" << u->getPassword() << "|"
              << u->getEmail() << "|" << u->getAddress() << "\n";
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
        f << r->getType() << "|" << r->getIsbn() << "|" << r->getTitle() << "|"
          << r->getWriter() << "|" << r->getPublicationYear() << "|"
          << r->getOrigin() << "|" << r->getLanguage() << "|"
          << r->getGenre() << "|" << categoryStr(r->getCategory()) << "|"
          << (r->isAvailable() ? "1" : "0");

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
        string type = t[0];
        string isbn = t[1], title = t[2], writer = t[3];
        int    year = stoi(t[4]);
        string origin = t[5], language = t[6], genre = t[7];
        BookCategory cat = categoryFromStr(t[8]);
        bool avail = (t[9] == "1");

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

        if (r) {
            r->setAvailable(avail);
            resources.push_back(r);
        }
    }
}

// Format: memberID|isbn|issueDate|dueDate|isReturned|returnDate
void LibrarySystem::saveBorrows(const string& filename) const {
    ofstream f(filename);
    for (User* u : users) {
        Member* m = dynamic_cast<Member*>(u);
        if (!m) continue;
        for (BorrowRecord* rec : m->getBorrowedBooks()) {
            f << m->getId() << "|"
              << rec->getResource()->getIsbn() << "|"
              << rec->getIssueDate() << "|"
              << rec->getDueDate()   << "|"
              << (rec->getIsReturned() ? "1" : "0") << "|"
              << rec->getReturnDate() << "\n";
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
    if (!dataFilesExist()) {
        seedDefaultData();
        return;
    }
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
    cout << "\n==============================\n";
    cout << "  WELCOME — FIRST TIME SETUP\n";
    cout << "==============================\n";
    cout << "No accounts found. Please create the Admin account.\n\n";

    string id    = readLine("Admin ID       : ");
    string first = readLine("First Name     : ");
    string last  = readLine("Last Name      : ");
    string pass  = readLine("Password       : ");
    string email = readLine("Email          : ");
    string addr  = readLine("Address        : ");

    users.push_back(new Admin(id, first, last, pass, email, addr));
    cout << "\nAdmin account created successfully.\n";

    // Add 3 sample resources so the library is not empty on first run
    resources.push_back(new ScienceBook("S001", "Introduction to Physics", "Halliday",
                                         2020, "USA", "English", "Physics",
                                         BookCategory::NON_FICTION, "Mechanics"));
    resources.push_back(new LiteratureBook("L001", "Pride and Prejudice", "Austen",
                                            1813, "UK", "English", "Romance",
                                            BookCategory::FICTION, "Regency"));
    resources.push_back(new Magazine("M001", "Science Today", "Staff",
                                      2024, "USA", "English", "Science",
                                      BookCategory::NON_FICTION, 5, "April"));
    cout << "3 sample resources added to the library.\n";
    cout << "You can now log in as Admin to manage the system.\n";
    saveAllData();
}

// ── getAllResources ───────────────────────────────────────────────────────────

vector<LibraryResource*> LibrarySystem::getAllResources() const {
    return resources;
}

// ═════════════════════════════════════════════════════════════════════════════
// run() — main application loop
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::run() {
    loadAllData();

    while (true) {
        cout << "\n==============================\n";
        cout << "  LIBRARY MANAGEMENT SYSTEM\n";
        cout << "==============================\n";
        cout << "[1] Admin Login\n";
        cout << "[2] Member Login\n";
        cout << "[3] Exit\n";
        int choice = readInt("Enter choice: ");

        if      (choice == 1) loginMenu("ADMIN");
        else if (choice == 2) loginMenu("MEMBER");
        else if (choice == 3) break;
        else cout << "Invalid choice, try again.\n";
    }

    saveAllData();
    cout << "Data saved. Goodbye!\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// loginMenu() — prompts for credentials and dispatches to member/admin menu
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::loginMenu(const string& role) {
    cout << "\n--- " << role << " LOGIN ---\n";
    string id   = readLine("Enter ID      : ");
    string pass = readLine("Enter Password: ");

    User* u = loginCheck(id, pass);
    if (!u) { cout << "Invalid credentials.\n"; return; }

    // Reject if the account role does not match the selected portal
    if (u->getRole() != role) {
        cout << "This ID is not registered as a " << role << ".\n";
        return;
    }

    currentUser = u;
    if (role == "MEMBER")
        memberMenu(dynamic_cast<Member*>(u));
    else
        adminMenu(dynamic_cast<Admin*>(u));

    currentUser = nullptr;
    saveAllData();
}

// ═════════════════════════════════════════════════════════════════════════════
// memberMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::memberMenu(Member* m) {
    while (true) {
        cout << "\n==============================\n";
        cout << "  MEMBER DASHBOARD\n";
        cout << "  Welcome, " << m->getFirstName() << "\n";
        cout << "  Balance: PKR " << m->getBalance() << "\n";
        cout << "==============================\n";
        cout << "[1]  Browse Available Resources\n";
        cout << "[2]  Search Resources\n";
        cout << "[3]  Issue a Resource\n";
        cout << "[4]  Return a Resource\n";
        cout << "[5]  Reserve a Resource\n";
        cout << "[6]  View Borrowing History\n";
        cout << "[7]  View Balance\n";
        cout << "[8]  Deposit Amount\n";
        cout << "[9]  Write a Review\n";
        cout << "[10] Logout\n";
        int choice = readInt("Enter choice: ");

        if      (choice == 1)  showAvailableResources();
        else if (choice == 2)  searchMenu(m);
        else if (choice == 3) {
            string isbn = readLine("Enter ISBN to issue: ");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "Resource not found.\n"; continue; }
            try   { m->issueBook(r); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 4) {
            string isbn = readLine("Enter ISBN to return: ");
            m->returnBook(isbn);
        }
        else if (choice == 5) {
            string isbn = readLine("Enter ISBN to reserve: ");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "Resource not found.\n"; continue; }
            m->reserveBook(r);
        }
        else if (choice == 6)  m->viewBorrowHistory();
        else if (choice == 7)  cout << "Balance: PKR " << m->getBalance() << "\n";
        else if (choice == 8) {
            double amount = readInt("Enter amount to deposit: PKR ");
            try   { m->depositAmount(amount); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 9) {
            string isbn = readLine("Enter ISBN of resource to review: ");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "Resource not found.\n"; continue; }
            // Member must have borrowed this resource at least once
            if (!m->findBorrowRecord(isbn)) {
                cout << "You can only review resources you have borrowed.\n";
                continue;
            }
            int rating = readInt("Enter rating (1-5): ");
            if (rating < 1 || rating > 5) { cout << "Rating must be 1-5.\n"; continue; }
            string comment = readLine("Enter comment: ");
            static int reviewCount = 0;
            r->addReview(Review("R" + to_string(++reviewCount), m->getId(), isbn, rating, comment));
            cout << "Review submitted.\n";
        }
        else if (choice == 10) { cout << "Logged out.\n"; return; }
        else cout << "Invalid choice, try again.\n";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// searchMenu() — member search submenu
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::searchMenu(Member* m) {
    (void)m;
    cout << "\nSearch by:\n";
    cout << "[1] Title\n[2] Author/Writer\n[3] Genre\n";
    cout << "[4] Language\n[5] Year\n[6] Keyword (all fields)\n";
    int choice = readInt("Enter choice: ");

    if (choice == 1) {
        string q = readLine("Title: ");
        printResults(searchByTitle(q), q);
    } else if (choice == 2) {
        string q = readLine("Author/Writer: ");
        printResults(searchByWriter(q), q);
    } else if (choice == 3) {
        string q = readLine("Genre: ");
        printResults(searchByGenre(q), q);
    } else if (choice == 4) {
        string q = readLine("Language: ");
        printResults(searchByLanguage(q), q);
    } else if (choice == 5) {
        int year = readInt("Year: ");
        printResults(searchByYear(year), to_string(year));
    } else if (choice == 6) {
        string q = readLine("Keyword: ");
        printResults(searchByKeyword(q), q);
    } else {
        cout << "Invalid choice.\n";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// adminMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::adminMenu(Admin* a) {
    while (true) {
        cout << "\n==============================\n";
        cout << "  ADMIN DASHBOARD\n";
        cout << "  Logged in as: " << a->getFirstName() << "\n";
        cout << "==============================\n";
        cout << "[1]  Manage Members\n";
        cout << "[2]  Manage Resources\n";
        cout << "[3]  Process a Return\n";
        cout << "[4]  View Overdue Dashboard\n";
        cout << "[5]  Generate Member Report\n";
        cout << "[6]  Generate Resource Report\n";
        cout << "[7]  Logout\n";
        int choice = readInt("Enter choice: ");

        if      (choice == 1) manageMembersMenu(a);
        else if (choice == 2) manageResourcesMenu(a);
        else if (choice == 3) processReturnFlow(a);
        else if (choice == 4) viewOverdueFlow();
        else if (choice == 5) {
            a->generateMemberReport(getAllMembers(), "data/member_report.txt");
        }
        else if (choice == 6) {
            a->generateResourceReport(getAllResources(), getAllMembers(), "data/resource_report.txt");
        }
        else if (choice == 7) { cout << "Logged out.\n"; return; }
        else cout << "Invalid choice, try again.\n";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// manageMembersMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::manageMembersMenu(Admin* a) {
    (void)a;
    cout << "\n[1] Add Member  [2] Remove Member\n";
    cout << "[3] View Member Borrow History  [4] Display All Members\n";
    int choice = readInt("Enter choice: ");

    if (choice == 1) {
        string id    = readLine("Member ID: ");
        string first = readLine("First Name: ");
        string last  = readLine("Last Name: ");
        string pass  = readLine("Password: ");
        string email = readLine("Email: ");
        string addr  = readLine("Address: ");
        double bal   = readInt("Initial Balance (PKR): ");
        Member* m = new Member(id, first, last, pass, email, addr);
        m->setBalance(bal);
        if (addUser(m)) cout << "Member added.\n";
    }
    else if (choice == 2) {
        string id = readLine("Member ID to remove: ");
        Member* m = dynamic_cast<Member*>(findUser(id));
        if (!m) { cout << "Member not found.\n"; return; }
        // Warn if member has active borrows
        int active = 0;
        for (BorrowRecord* r : m->getBorrowedBooks())
            if (!r->getIsReturned()) active++;
        if (active > 0) {
            cout << "Member has " << active << " active borrow(s). Confirm removal? [Y/N]: ";
            string ans; getline(cin, ans);
            if (ans != "Y" && ans != "y") { cout << "Removal cancelled.\n"; return; }
        }
        removeUser(id);
        cout << "Member removed.\n";
    }
    else if (choice == 3) {
        string id = readLine("Member ID: ");
        Member* m = dynamic_cast<Member*>(findUser(id));
        if (!m) { cout << "Member not found.\n"; return; }
        m->viewBorrowHistory();
    }
    else if (choice == 4) {
        displayAllUsers();
    }
    else cout << "Invalid choice.\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// manageResourcesMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::manageResourcesMenu(Admin* a) {
    (void)a;
    cout << "\n[1] Add Resource  [2] Remove Resource\n";
    cout << "[3] Edit Resource Availability  [4] Show Available Resources\n";
    int choice = readInt("Enter choice: ");

    if (choice == 1) {
        LibraryResource* r = promptNewResource();
        if (r && addResource(r)) cout << "Resource added.\n";
    }
    else if (choice == 2) {
        string isbn = readLine("ISBN to remove: ");
        removeResource(isbn);
    }
    else if (choice == 3) {
        string isbn = readLine("ISBN: ");
        LibraryResource* r = findResource(isbn);
        if (!r) { cout << "Resource not found.\n"; return; }
        r->setAvailable(!r->isAvailable());
        cout << "Availability set to: " << (r->isAvailable() ? "Available" : "Unavailable") << "\n";
    }
    else if (choice == 4) showAvailableResources();
    else cout << "Invalid choice.\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// promptNewResource() — reads all fields from the user and returns a new resource
// ═════════════════════════════════════════════════════════════════════════════

LibraryResource* LibrarySystem::promptNewResource() {
    cout << "\nResource type?\n";
    cout << "[1] Science Book  [2] Literature Book  [3] Magazine\n";
    cout << "[4] Reference Book  [5] Digital Media\n";
    int type = readInt("Enter choice: ");
    if (type < 1 || type > 5) { cout << "Invalid type.\n"; return nullptr; }

    // Common fields
    string isbn   = readLine("ISBN: ");
    string title  = readLine("Title: ");
    string writer = readLine("Author/Writer: ");
    int    year   = readInt("Publication Year: ");
    string origin = readLine("Origin (country): ");
    string lang   = readLine("Language: ");
    string genre  = readLine("Genre: ");
    cout << "Category — [1] Fiction  [2] Non-Fiction: ";
    int catChoice = readInt("");
    BookCategory cat = (catChoice == 1) ? BookCategory::FICTION : BookCategory::NON_FICTION;

    if (type == 1) {
        string field = readLine("Scientific Field: ");
        return new ScienceBook(isbn, title, writer, year, origin, lang, genre, cat, field);
    }
    if (type == 2) {
        string era = readLine("Literary Era: ");
        return new LiteratureBook(isbn, title, writer, year, origin, lang, genre, cat, era);
    }
    if (type == 3) {
        int    issue = readInt("Issue Number: ");
        string month = readLine("Publication Month: ");
        return new Magazine(isbn, title, writer, year, origin, lang, genre, cat, issue, month);
    }
    if (type == 4) {
        int edition = readInt("Edition: ");
        return new ReferenceBook(isbn, title, writer, year, origin, lang, genre, cat, edition);
    }
    // type == 5
    float  runtime = (float)readInt("Runtime (minutes): ");
    string format  = readLine("File Format (e.g. MP4): ");
    return new DigitalMedia(isbn, title, writer, year, origin, lang, genre, cat, runtime, format);
}

// ═════════════════════════════════════════════════════════════════════════════
// processReturnFlow()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::processReturnFlow(Admin* a) {
    string mid  = readLine("Enter Member ID: ");
    Member* m = dynamic_cast<Member*>(findUser(mid));
    if (!m) { cout << "Member not found.\n"; return; }

    string isbn = readLine("Enter Resource ISBN: ");

    // Try active borrow first; if already returned by member, find any record
    BorrowRecord* rec = m->findActiveBorrow(isbn);
    bool alreadyReturned = false;
    if (!rec) {
        rec = m->findBorrowRecord(isbn);
        alreadyReturned = true;
    }
    if (!rec) { cout << "No borrow record found for ISBN " << isbn << ".\n"; return; }
    if (alreadyReturned && rec->getResource()->isAvailable()) {
        cout << "This resource has already been fully processed.\n"; return;
    }

    // Show borrow details
    cout << "\nResource : \"" << rec->getResource()->getTitle() << "\"\n";
    time_t issued = rec->getIssueDate();
    cout << "Issued   : " << ctime(&issued);
    int lateDays = rec->calculateLateDays(time(nullptr));
    cout << "Overdue  : " << lateDays << " day(s)\n";

    // Select condition
    cout << "\nCondition?\n";
    cout << "[1] Perfect  [2] Minor Damage  [3] Heavy Damage  [4] Lost\n";
    int c = readInt("Enter choice: ");
    if (c < 1 || c > 4) { cout << "Invalid choice.\n"; return; }
    BookCondition cond = static_cast<BookCondition>(c - 1);

    // Mark returned if the member hasn't done so yet
    if (!alreadyReturned) m->returnBook(isbn);

    a->processReturn(*m, rec, cond, time(nullptr));
    cout << "Updated balance: PKR " << m->getBalance() << "\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// viewOverdueFlow()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::viewOverdueFlow() const {
    time_t now = time(nullptr);
    bool found = false;
    cout << "\n=== OVERDUE RESOURCES ===\n";
    for (Member* m : getAllMembers()) {
        for (BorrowRecord* r : m->getBorrowedBooks()) {
            if (!r->getIsReturned() && r->getDueDate() < now) {
                int days = r->calculateLateDays(now);
                double est = FineUtility::calculateFine(days, BookCondition::PERFECT);
                cout << "Member: " << m->getFirstName() << " " << m->getLastName()
                     << " (" << m->getId() << ")\n";
                cout << "  \"" << r->getResource()->getTitle()
                     << "\" — overdue by " << days
                     << " day(s) | Est. fine: PKR " << est << "\n";
                found = true;
            }
        }
    }
    if (!found) cout << "No overdue resources.\n";
}
