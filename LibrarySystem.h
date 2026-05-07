#pragma once
#include <vector>
#include <string>
#include "ResourceTypes.h"
#include "Member.h"
#include "Admin.h"

// LibrarySystem is the top-level controller for the entire application.
// It owns all User* and LibraryResource* objects (deletes them in destructor).
// It manages the in-memory collections and handles all file I/O directly
// using fstream — no separate database class is needed.
class LibrarySystem {
private:
    std::vector<User*>            users;       // owns all users (Member + Admin)
    std::vector<LibraryResource*> resources;   // owns all resources
    User*                         currentUser; // observing pointer — owned by users vector

    // ── File I/O (plain fstream, pipe-delimited text) ──────────────────────
    void saveUsers(const std::string& filename)     const;
    void loadUsers(const std::string& filename);
    void saveResources(const std::string& filename) const;
    void loadResources(const std::string& filename);
    void saveBorrows(const std::string& filename)   const;
    void loadBorrows(const std::string& filename);

    // ── Seed default data on very first run ────────────────────────────────
    void seedDefaultData();

    // ── Internal helpers ───────────────────────────────────────────────────
    bool dataFilesExist() const;
    void ensureDataFolder() const;

public:
    LibrarySystem();
    ~LibrarySystem(); // deletes all User* and LibraryResource*

    // ── Application entry point ────────────────────────────────────────────
    void run();

    // ── User management ────────────────────────────────────────────────────
    bool  addUser(User* u);      // returns false if ID already exists
    void  removeUser(const std::string& id);
    User* findUser(const std::string& id)                             const;
    User* loginCheck(const std::string& id, const std::string& pass) const;
    std::vector<Member*> getAllMembers()                              const;
    void  displayAllUsers()                                          const;

    // ── Resource management ────────────────────────────────────────────────
    bool             addResource(LibraryResource* r); // returns false if ISBN already exists
    void             removeResource(const std::string& isbn);
    LibraryResource* findResource(const std::string& isbn)           const;
    void             showAvailableResources()                        const;
    void             showAllResources()                              const;

    // ── Search (all return matching resources; case-insensitive substring) ─
    std::vector<LibraryResource*> searchByTitle(const std::string& title)    const;
    std::vector<LibraryResource*> searchByWriter(const std::string& writer)  const;
    std::vector<LibraryResource*> searchByGenre(const std::string& genre)    const;
    std::vector<LibraryResource*> searchByLanguage(const std::string& lang)  const;
    std::vector<LibraryResource*> searchByYear(int year)                     const;
    std::vector<LibraryResource*> searchByKeyword(const std::string& kw)     const;

    // ── Persistence ────────────────────────────────────────────────────────
    void loadAllData();
    void saveAllData() const;

    // ── Accessors needed by Admin report methods ───────────────────────────
    std::vector<LibraryResource*> getAllResources() const;

    // ── Menus (implemented in LibrarySystem.cpp — PR 6) ───────────────────
private:
    void loginMenu(const std::string& role); // role = "ADMIN" or "MEMBER"
    void memberMenu(Member* m);
    void adminMenu(Admin* a);

    // Admin submenus
    void manageMembersMenu(Admin* a);
    void manageResourcesMenu(Admin* a);
    void processReturnFlow(Admin* a);
    void viewOverdueFlow() const;

    // Member submenus
    void searchMenu(Member* m);

    // Resource creation helper (used in Add Resource flow)
    LibraryResource* promptNewResource();
};
