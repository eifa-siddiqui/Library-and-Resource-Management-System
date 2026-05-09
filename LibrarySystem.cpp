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
        cout << "\033[31m Invalid input. Please enter a number: \033[0m";
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
        cout << "\033[31m No resources found matching '\033[0m" << query << "'.\n";
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
        cout << "\033[31m User ID \033[0m" << u->getId() << "\033[31m already exists.\033[0m\n";
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
    cout << "\033[31mUser \033[0m" << id << "\033[31m not found.\033[0m\n";
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
        cout << "\033[31m ISBN \033[0m" << r->getIsbn() << "\033[31m already exists.\033[0m\n";
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
                cout << "\033[31m Cannot remove \"\033[0m" << resources[i]->getTitle()
                     << "\033[31m\" — it is currently issued.\033[0m\n";
                return;
            }
            delete resources[i];
            resources.erase(resources.begin() + i);
            return;
        }
    }
    cout << "\033[31m Resource \033[0m" << isbn  << "\033[31m not found.\033[0m \n";
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
    if (!any) cout << "\033[31m No resources currently available.\033[0m\n";
}

void LibrarySystem::showAllResources() const {
    if (resources.empty()) { cout << "\033[31m No resources in the library.\033[0m \n"; return; }
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
    cout << "\033[33m\n==============================\n \033[0m";
    cout << "  \033[34m WELCOME — FIRST TIME SETUP\n\033[0m";
    cout << "\033[33m ==============================\n\033[0m";
    cout << "\033[31m No accounts found. Please create the Admin account.\n\n\033[0m";

    string id    = readLine("\033[36m Admin ID       : \033[0m");
    string first = readLine("\033[36m First Name     : \033[0m");
    string last  = readLine("\033[36m Last Name      : \033[0m");
    string pass  = readLine("\033[36m Password       : \033[0m");
    string email = readLine("\033[36m Email          : \033[0m");
    string addr  = readLine("\033[36m Address        : \033[0m");

    users.push_back(new Admin(id, first, last, pass, email, addr));
    cout << "\033[32m\nAdmin account created successfully.\n\033[0m";

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
    cout << "\033[32m 3 sample resources added to the library.\n\033[0m";
    cout << "\033[32m You can now log in as Admin to manage the system.\n\033[0m";
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
        cout << "\033[33m \n==============================\n\033[0m";
        cout << "\033[34m  LIBRARY MANAGEMENT SYSTEM\n\033[0m";
        cout << "\033[33m ==============================\n\033[0m";
        cout << "\033[36m [1] Admin Login\n\033[0m";
        cout << "\033[36m [2] Member Login\n\033[0m";
        cout << "\033[36m [3] Exit\n";
        int choice = readInt("\033[35m Enter choice: \033[0m");

        if      (choice == 1) loginMenu("ADMIN");
        else if (choice == 2) loginMenu("MEMBER");
        else if (choice == 3) break;
        else cout << "\033[31m Invalid choice, try again.\033[0m\n";
    }

    saveAllData();
    cout << "\033[32m Data saved. Goodbye!\033[0m\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// loginMenu() — prompts for credentials and dispatches to member/admin menu
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::loginMenu(const string& role) {
    cout << "\033[34m \n--- \033[0m" << role << "\033[34m LOGIN ---\n\033[0m";
    string id   = readLine("\033[35m Enter ID      : \033[0m");
    string pass = readLine("\033[35m Enter Password: \033[0m");

    User* u = loginCheck(id, pass);
    if (!u) { cout << "\033[31m Invalid credentials.\n\033[0m"; return; }

    // Reject if the account role does not match the selected portal
    if (u->getRole() != role) {
        cout << "\033[31m This ID is not registered as a \033[0m" << role << ".\n";
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
        cout << "\033[33m \n==============================\n\033[0m";
        cout << "\033[34m  MEMBER DASHBOARD\n\033[0m";
        cout << "\033[33m  Welcome, \033[0m" << m->getFirstName() << "\n";
        cout << "\033[32m  Balance: PKR \033[0m" << m->getBalance() << "\n";
        cout << "\033[33m==============================\n\033[0m";
        cout << "\033[36m [1]  Browse Available Resources\n\033[0m";
        cout << "\033[36m[2]  Search Resources\n\033[0m";
        cout << "\033[36m[3]  Issue a Resource\n\033[0m";
        cout << "\033[36m[4]  Return a Resource\n\033[0m";
        cout << "\033[36m[5]  Reserve a Resource\n\033[0m";
        cout << "\033[36m[6]  View Borrowing History\n\033[0m";
        cout << "\033[36m[7]  View Balance\n\033[0m";
        cout << "\033[36m[8]  Deposit Amount\n\033[0m";
        cout << "\033[36m[9]  Write a Review\n\033[0m";
        cout << "\033[36m[10] Logout\n\033[0m";
        int choice = readInt("\033[35m Enter choice: \033[0m");

        if      (choice == 1)  showAvailableResources();
        else if (choice == 2)  searchMenu(m);
        else if (choice == 3) {
            string isbn = readLine("\033[36m Enter ISBN to issue: \033[0m");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "\033[31m Resource not found.\n\033[0m"; continue; }
            try   { m->issueBook(r); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 4) {
            string isbn = readLine("\033[36m Enter ISBN to return: \033[0m");
            m->returnBook(isbn);
        }
        else if (choice == 5) {
            string isbn = readLine("\033[36m Enter ISBN to reserve: \033[0m");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "\033[31m Resource not found.\n\033[0m"; continue; }
            m->reserveBook(r);
        }
        else if (choice == 6)  m->viewBorrowHistory();
        else if (choice == 7)  cout << "\033[32m Balance: PKR \033[0m" << m->getBalance() << "\n";
        else if (choice == 8) {
            double amount = readInt("\033[36m Enter amount to deposit: PKR \033[0m");
            try   { m->depositAmount(amount); }
            catch (const exception& e) { cout << e.what() << "\n"; }
        }
        else if (choice == 9) {
            string isbn = readLine("\033[36m Enter ISBN of resource to review: \033[0m");
            LibraryResource* r = findResource(isbn);
            if (!r) { cout << "\033[31m Resource not found.\n \033[0m"; continue; }
            // Member must have borrowed this resource at least once
            if (!m->findBorrowRecord(isbn)) {
                cout << "\033[31m You can only review resources you have borrowed.\n\033[0m";
                continue;
            }
            int rating = readInt("\033[36m Enter rating (1-5): \033[0m");
            if (rating < 1 || rating > 5) { cout << "\033[31m Rating must be 1-5.\n\033[0m"; continue; }
            string comment = readLine("\033[35m Enter comment: \033[0m");
            static int reviewCount = 0;
            r->addReview(Review("R" + to_string(++reviewCount), m->getId(), isbn, rating, comment));
            cout << "\033[32mReview submitted.\n\033[0m";
        }
        else if (choice == 10) { cout << "\033[33m Logged out.\n\033[0m"; return; }
        else cout << "\033[31m Invalid choice, try again.\n\033[0m";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// searchMenu() — member search submenu
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::searchMenu(Member* m) {
    (void)m;
    cout << "\033[36m\nSearch by:\n\033[0m";
    cout << "\033[35m [1] Title\n[2] Author/Writer\n[3] Genre\n\033[0m";
    cout << "\033[35m[4] Language\n[5] Year\n[6] Keyword (all fields)\n\033[0m";
    int choice = readInt("\033[33mEnter choice: \033[0m");

    if (choice == 1) {
        string q = readLine("\033[34mTitle: \033[0m");
        printResults(searchByTitle(q), q);
    } else if (choice == 2) {
        string q = readLine("\033[34mAuthor/Writer: \033[0m");
        printResults(searchByWriter(q), q);
    } else if (choice == 3) {
        string q = readLine("\033[34mGenre: \033[0m");
        printResults(searchByGenre(q), q);
    } else if (choice == 4) {
        string q = readLine("\033[34mLanguage: \033[0m");
        printResults(searchByLanguage(q), q);
    } else if (choice == 5) {
        int year = readInt("\033[34mYear: \033[0m");
        printResults(searchByYear(year), to_string(year));
    } else if (choice == 6) {
        string q = readLine("\033[34mKeyword: \033[0m");
        printResults(searchByKeyword(q), q);
    } else {
        cout << "\033[31mInvalid choice.\n\033[0m";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// adminMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::adminMenu(Admin* a) {
    while (true) {
        cout << "\033[33m\n==============================\n\033[0m";
        cout << "  \033[34mADMIN DASHBOARD\n\033[0m";
        cout << "  \033[32m Logged in as: \033[0m" << a->getFirstName() << "\n";
        cout << "\033[33m==============================\n\033[0m";
        cout << "\033[36m[1]  Manage Members\n\033[0m";
        cout << "\033[36m[2]  Manage Resources\n\033[0m";
        cout << "\033[36m[3]  Process a Return\n\033[0m";
        cout << "\033[36m[4]  View Overdue Dashboard\n\033[0m";
        cout << "\033[36m[5]  Generate Member Report\n\033[0m";
        cout << "\033[36m[6]  Generate Resource Report\n\033[0m";
        cout << "\033[36m[7]  Logout\n\033[0m";
        int choice = readInt("\033[35m Enter choice: \033[0m");

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
        else if (choice == 7) { cout << "\033[32mLogged out.\n"; return; }
        else cout << "\033[31m Invalid choice, try again.\n\033[0m";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// manageMembersMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::manageMembersMenu(Admin* a) {
    (void)a;
    cout << "\n\033[36m[1] Add Member  [2] Remove Member\n\033[0m";
    cout << "\033[36m [3] View Member Borrow History  [4] Display All Members\n\033[0m";
    int choice = readInt("\033[35mEnter choice: \033[0m");

    if (choice == 1) {
        string id    = readLine("\033[34mMember ID: \033[0m");
        string first = readLine("\033[34mFirst Name: \033[0m");
        string last  = readLine("\033[34mLast Name: \033[0m");
        string pass  = readLine("\033[34mPassword: \033[0m");
        string email = readLine("\033[34mEmail: \033[0m");
        string addr  = readLine("\033[34mAddress: \033[0m");
        double bal   = readInt("\033[34mInitial Balance (PKR): \033[0m");
        Member* m = new Member(id, first, last, pass, email, addr);
        m->setBalance(bal);
        if (addUser(m)) cout << "\033[32mMember added.\n\033[0m";
    }
    else if (choice == 2) {
        string id = readLine("\033[34mMember ID to remove: \033[0m");
        Member* m = dynamic_cast<Member*>(findUser(id));
        if (!m) { cout << "\033[31mMember not found.\n\033[0m"; return; }
        // Warn if member has active borrows
        int active = 0;
        for (BorrowRecord* r : m->getBorrowedBooks())
            if (!r->getIsReturned()) active++;
        if (active > 0) {
            cout << "\033[31mMember has \033[0m" << active << "\033[31m active borrow(s). Confirm removal? [Y/N]: \033[0m";
            string ans; getline(cin, ans);
            if (ans != "Y" && ans != "y") { cout << "\033[32mRemoval cancelled.\n\033[0m"; return; }
        }
        removeUser(id);
        cout << "\033[32mMember removed.\n\033[0m";
    }
    else if (choice == 3) {
        string id = readLine("\033[34mMember ID: \033[0m");
        Member* m = dynamic_cast<Member*>(findUser(id));
        if (!m) { cout << "\033[31mMember not found.\n\033[0m"; return; }
        m->viewBorrowHistory();
    }
    else if (choice == 4) {
        displayAllUsers();
    }
    else cout << "\033[31mInvalid choice.\n\033[0m";
}

// ═════════════════════════════════════════════════════════════════════════════
// manageResourcesMenu()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::manageResourcesMenu(Admin* a) {
    (void)a;
    cout << "\n\033[36m[1] Add Resource  [2] Remove Resource\n\033[0m";
    cout << "\033[36m [3] Edit Resource Availability  [4] Show Available Resources\n\033[0m";
    int choice = readInt("\033[35mEnter choice: \033[0m");

    if (choice == 1) {
        LibraryResource* r = promptNewResource();
        if (r && addResource(r)) cout << "\033[32mResource added.\n\033[0m";
    }
    else if (choice == 2) {
        string isbn = readLine("\033[34mISBN to remove: \033[0m");
        removeResource(isbn);
    }
    else if (choice == 3) {
        string isbn = readLine("\033[33m ISBN: \033[0m");
        LibraryResource* r = findResource(isbn);
        if (!r) { cout << "\033[31mResource not found.\n\033[0m"; return; }
        r->setAvailable(!r->isAvailable());
        cout << "\033[33mAvailability set to: \033[0m" << (r->isAvailable() ? "\033[32mAvailable\033[0m" : "\033[31mUnavailable\033[0m") << "\n";
    }
    else if (choice == 4) showAvailableResources();
    else cout << "\033[31mInvalid choice.\n\033[0m";
}

// ═════════════════════════════════════════════════════════════════════════════
// promptNewResource() — reads all fields from the user and returns a new resource
// ═════════════════════════════════════════════════════════════════════════════

LibraryResource* LibrarySystem::promptNewResource() {
    cout << "\n\033[33mResource type?\n\033[0m";
    cout << "\033[36m[1] Science Book  [2] Literature Book  [3] Magazine\n\033[0m";
    cout << "\033[36m[4] Reference Book  [5] Digital Media\n\033[0m";
    int type = readInt("\033[35mEnter choice: \033[0m");
    if (type < 1 || type > 5) { cout << "\033[31mInvalid type.\n\033[0m"; return nullptr; }

    // Common fields
    string isbn   = readLine("\033[34mISBN: \033[0m");
    string title  = readLine("\033[34mTitle: \033[0m");
    string writer = readLine("\033[34mAuthor/Writer: \033[0m");
    int    year   = readInt("\033[34mPublication Year: \033[0m");
    string origin = readLine("\033[34mOrigin (country): \033[0m");
    string lang   = readLine("\033[34mLanguage: \033[0m");
    string genre  = readLine("\033[34mGenre: \033[0m");
    cout << "\033[36mCategory — [1] Fiction  [2] Non-Fiction: \033[0m";
    int catChoice = readInt("");
    BookCategory cat = (catChoice == 1) ? BookCategory::FICTION : BookCategory::NON_FICTION;

    if (type == 1) {
        string field = readLine("\033[34mScientific Field: \033[0m");
        return new ScienceBook(isbn, title, writer, year, origin, lang, genre, cat, field);
    }
    if (type == 2) {
        string era = readLine("\033[34mLiterary Era: \033[0m");
        return new LiteratureBook(isbn, title, writer, year, origin, lang, genre, cat, era);
    }
    if (type == 3) {
        int    issue = readInt("\033[33mIssue Number: \033[0m");
        string month = readLine("\033[33mPublication Month: \033[0m");
        return new Magazine(isbn, title, writer, year, origin, lang, genre, cat, issue, month);
    }
    if (type == 4) {
        int edition = readInt("\033[33mEdition: \033[0m");
        return new ReferenceBook(isbn, title, writer, year, origin, lang, genre, cat, edition);
    }
    // type == 5
    float  runtime = (float)readInt("\033[33mRuntime (minutes): \033[0m");
    string format  = readLine("\033[33mFile Format (e.g. MP4): \033[0m");
    return new DigitalMedia(isbn, title, writer, year, origin, lang, genre, cat, runtime, format );
}

// ═════════════════════════════════════════════════════════════════════════════
// processReturnFlow()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::processReturnFlow(Admin* a) {
    string mid  = readLine("\033[34mEnter Member ID: \033[0m");
    Member* m = dynamic_cast<Member*>(findUser(mid));
    if (!m) { cout << "\033[31mMember not found.\n\033[0m"; return; }

    string isbn = readLine("\033[34mEnter Resource ISBN: \033[0m");

    // Try active borrow first; if already returned by member, find any record
    BorrowRecord* rec = m->findActiveBorrow(isbn);
    bool alreadyReturned = false;
    if (!rec) {
        rec = m->findBorrowRecord(isbn);
        alreadyReturned = true;
    }
    if (!rec) { cout << "\033[31mNo borrow record found for ISBN \033[0m" << isbn << ".\n"; return; }
    if (alreadyReturned && rec->getResource()->isAvailable()) {
        cout << "\033[35mThis resource has already been fully processed.\n\033[0m"; return;
    }

    // Show borrow details
    cout << "\033[34m\nResource : \"\033[0m" << rec->getResource()->getTitle() << "\"\n";
    time_t issued = rec->getIssueDate();
    cout << "\033[34mIssued   : \033[0m" << ctime(&issued);
    int lateDays = rec->calculateLateDays(time(nullptr));
    cout << "\033[34mOverdue  : \033[0m" << lateDays << " day(s)\n";

    // Select condition
    cout << "\033[35m\nCondition?\n\033[0m";
    cout << "\033[36m[1] Perfect  [2] Minor Damage  [3] Heavy Damage  [4] Lost\n\033[0m";
    int c = readInt("\033[35mEnter choice: \033[0m");
    if (c < 1 || c > 4) { cout << "\033[31mInvalid choice.\n\033[0m"; return; }
    BookCondition cond = static_cast<BookCondition>(c - 1);

    // Mark returned if the member hasn't done so yet
    if (!alreadyReturned) m->returnBook(isbn);

    a->processReturn(*m, rec, cond, time(nullptr));
    cout << "\033[32mUpdated balance: PKR \033[0m" << m->getBalance() << "\n";
}

// ═════════════════════════════════════════════════════════════════════════════
// viewOverdueFlow()
// ═════════════════════════════════════════════════════════════════════════════

void LibrarySystem::viewOverdueFlow() const {
    time_t now = time(nullptr);
    bool found = false;
    cout << "\033[33m\n=== OVERDUE RESOURCES ===\n\033[0m";
    for (Member* m : getAllMembers()) {
        for (BorrowRecord* r : m->getBorrowedBooks()) {
            if (!r->getIsReturned() && r->getDueDate() < now) {
                int days = r->calculateLateDays(now);
                double est = FineUtility::calculateFine(days, BookCondition::PERFECT);
                cout << "\033[36mMember: \033[0m" << m->getFirstName() << " " << m->getLastName()
                     << "\033[36m (" << m->getId() << ")\n\033[0m";
                cout << "\033[36m  \"" << r->getResource()->getTitle()
                     << "\033[36m\" — overdue by \033[0m" << days
                     << "\033[36m day(s) | Est. fine: PKR \033[0m" << est << "\n";
                found = true;
            }
        }
    }
    if (!found) cout << "\033[33mNo overdue resources.\n\033[0m";
}
