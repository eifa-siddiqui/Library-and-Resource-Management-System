#pragma once
#include <string>
#include <queue>
#include <vector>
#include "Enums.h"
#include "Review.h"

class Member; // forward declaration — avoids circular includes

// Abstract base class for every resource in the library.
// Protected attributes allow subclasses to access fields directly in displayinfo().
// getType() = 0 makes this class abstract (cannot be instantiated directly).
class LibraryResource {
protected:
    std::string  isbn;
    std::string  title;
    std::string  writer;
    int          publicationYear;
    std::string  origin;
    std::string  language;
    std::string  genre;
    BookCategory category;
    bool         isavailable;          // default true
    std::queue<Member*> reservationList; // observing pointers — never deleted here
    std::vector<Review>  reviews;

public:
    virtual ~LibraryResource();

    LibraryResource();  // isavailable = true, year = 0

    LibraryResource(const std::string& isbn,
                    const std::string& title,
                    const std::string& writer,
                    int                year,
                    const std::string& origin,
                    const std::string& language,
                    const std::string& genre,
                    BookCategory       category);

    // ---------- Setters (validation from rev_mem.cpp — unchanged) ----------
    void setAvailable(bool val);
    bool isAvailable()                        const;
    void setTitle(const std::string& t);
    void setWriter(const std::string& w);
    void setISBN(const std::string& i);       // validates 13-digit format
    void setOrigin(const std::string& o);
    void setLanguage(const std::string& l);
    void setPublicationYear(int y);
    void setGenre(const std::string& g);
    void setCategory(BookCategory c);

    // ---------- Getters ----------
    std::string  getIsbn()            const;
    std::string  getTitle()           const;
    std::string  getWriter()          const;
    std::string  getOrigin()          const;
    std::string  getLanguage()        const;
    int          getPublicationYear() const;
    std::string  getGenre()           const;
    BookCategory getCategory()        const;
    bool         getIsAvailable()     const;  // alias for isAvailable()

    // ---------- Review management ----------
    void addReview(const Review& r);
    const std::vector<Review>& getReviews() const;

    // ---------- Reservation queue ----------
    void    addToReservation(Member* m);
    Member* nextReservation();        // pops front; returns nullptr if empty
    bool    hasReservation()     const;
    int     getReservationCount() const; // queue size — used to display position to reserver

    // Operator overloading — case-insensitive substring search across isbn/title/writer/genre.
    // Fulfils the operator overloading OOP requirement.
    bool operator==(const std::string& query) const;

    // Displays common fields; overridden in each subclass to add type-specific fields.
    virtual void displayinfo() const;

    // Pure virtual — each subclass returns its type tag used for file persistence.
    // This makes LibraryResource abstract (cannot be instantiated directly).
    virtual std::string getType() const = 0;
};
