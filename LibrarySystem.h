#pragma once
#include <vector>
#include <string>
#include "ResourceTypes.h"
#include "Member.h"
#include "Admin.h"

// LibrarySystem is the top-level controller for the entire application.
// It owns all User* and LibraryResource* objects (deletes them in destructor).
class LibrarySystem {
private:
    std::vector<User*>            users;
    std::vector<LibraryResource*> resources;
    User*                         currentUser;

    // File I/O (pipe-delimited text files)
    void saveUsers(const std::string& filename)     const;
    void loadUsers(const std::string& filename);
    void saveResources(const std::string& filename) const;
    void loadResources(const std::string& filename);
    void saveBorrows(const std::string& filename)   const;
    void loadBorrows(const std::string& filename);

    void seedDefaultData();
    bool dataFilesExist()   const;
    void ensureDataFolder() const;

public:
    LibrarySystem();
    ~LibrarySystem();

    void run();

    // User management
    bool  addUser(User* u);
    void  removeUser(const std::string& id);
    User* findUser(const std::string& id)                             const;
    User* loginCheck(const std::string& id, const std::string& pass) const;
    std::vector<Member*> getAllMembers()                              const;
    void  displayAllUsers()                                          const;

    // Resource management
    bool             addResource(LibraryResource* r);
    void             removeResource(const std::string& isbn);
    LibraryResource* findResource(const std::string& isbn) const;
    void             showAvailableResources()              const;
    void             showAllResources()                    const;

    // Resource search (case-insensitive substring)
    std::vector<LibraryResource*> searchByTitle(const std::string& q)    const;
    std::vector<LibraryResource*> searchByWriter(const std::string& q)   const;
    std::vector<LibraryResource*> searchByGenre(const std::string& q)    const;
    std::vector<LibraryResource*> searchByLanguage(const std::string& q) const;
    std::vector<LibraryResource*> searchByYear(int year)                 const;
    std::vector<LibraryResource*> searchByKeyword(const std::string& kw) const;

    void loadAllData();
    void saveAllData() const;

    std::vector<LibraryResource*> getAllResources() const;

private:
    // Menus
    void loginMenu(const std::string& role);
    void memberMenu(Member* m);
    void adminMenu(Admin* a);

    void manageMembersMenu(Admin* a);
    void manageResourcesMenu(Admin* a);
    void processReturnFlow(Admin* a);
    void viewOverdueFlow() const;

    void searchMenu();           // resource search (used by both member and admin)
    void adminMemberSearch();    // member search (admin only)

    LibraryResource* promptNewResource();
};
