#include "ResourceTypes.h"
#include <iostream>
using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// ScienceBook
// ─────────────────────────────────────────────────────────────────────────────

ScienceBook::ScienceBook(const string& isbn,  const string& title,
                          const string& writer, int year,
                          const string& origin, const string& language,
                          const string& genre,  BookCategory category,
                          const string& scientificField)
    : LibraryResource(isbn, title, writer, year, origin, language, genre, category),
      scientificField(scientificField)
{}

void ScienceBook::displayinfo() const {
    cout << "[SCIENCE] " << title << " by " << writer
         << " (" << publicationYear << ")"
         << " | Field: " << scientificField
         << " | Available: " << (isavailable ? "Yes" : "No") << endl;
}

string ScienceBook::getType()            const { return "SCIENCE"; }
string ScienceBook::getScientificField() const { return scientificField; }

// ─────────────────────────────────────────────────────────────────────────────
// LiteratureBook
// ─────────────────────────────────────────────────────────────────────────────

LiteratureBook::LiteratureBook(const string& isbn,  const string& title,
                                const string& writer, int year,
                                const string& origin, const string& language,
                                const string& genre,  BookCategory category,
                                const string& literaryEra)
    : LibraryResource(isbn, title, writer, year, origin, language, genre, category),
      literaryEra(literaryEra)
{}

void LiteratureBook::displayinfo() const {
    cout << "[LITERATURE] " << title << " by " << writer
         << " (" << publicationYear << ")"
         << " | Era: " << literaryEra
         << " | Available: " << (isavailable ? "Yes" : "No") << endl;
}

string LiteratureBook::getType()       const { return "LITERATURE"; }
string LiteratureBook::getLiteraryEra() const { return literaryEra; }

// ─────────────────────────────────────────────────────────────────────────────
// Magazine
// ─────────────────────────────────────────────────────────────────────────────

Magazine::Magazine(const string& isbn,  const string& title,
                   const string& writer, int year,
                   const string& origin, const string& language,
                   const string& genre,  BookCategory category,
                   int issueNumber, const string& publicationMonth)
    : LibraryResource(isbn, title, writer, year, origin, language, genre, category),
      issueNumber(issueNumber), publicationMonth(publicationMonth)
{}

void Magazine::displayinfo() const {
    cout << "[MAGAZINE] " << title
         << " — Issue " << issueNumber
         << " (" << publicationMonth << " " << publicationYear << ")"
         << " | Available: " << (isavailable ? "Yes" : "No") << endl;
}

string Magazine::getType()             const { return "MAGAZINE"; }
int    Magazine::getIssueNumber()      const { return issueNumber; }
string Magazine::getPublicationMonth() const { return publicationMonth; }

// ─────────────────────────────────────────────────────────────────────────────
// ReferenceBook
// ─────────────────────────────────────────────────────────────────────────────

ReferenceBook::ReferenceBook(const string& isbn,  const string& title,
                              const string& writer, int year,
                              const string& origin, const string& language,
                              const string& genre,  BookCategory category,
                              int edition)
    : LibraryResource(isbn, title, writer, year, origin, language, genre, category),
      edition(edition)
{}

void ReferenceBook::displayinfo() const {
    cout << "[REFERENCE] " << title << " by " << writer
         << " | Edition: " << edition
         << " | Available: " << (isavailable ? "Yes" : "No") << endl;
}

string ReferenceBook::getType()    const { return "REFERENCE"; }
int    ReferenceBook::getEdition() const { return edition; }

// ─────────────────────────────────────────────────────────────────────────────
// DigitalMedia
// ─────────────────────────────────────────────────────────────────────────────

DigitalMedia::DigitalMedia(const string& isbn,  const string& title,
                            const string& writer, int year,
                            const string& origin, const string& language,
                            const string& genre,  BookCategory category,
                            float runTimeMinutes, const string& fileFormat)
    : LibraryResource(isbn, title, writer, year, origin, language, genre, category),
      runTimeMinutes(runTimeMinutes), fileFormat(fileFormat)
{}

void DigitalMedia::displayinfo() const {
    cout << "[DIGITAL] " << title << " by " << writer
         << " | Runtime: " << runTimeMinutes << " min"
         << " | Format: " << fileFormat
         << " | Available: " << (isavailable ? "Yes" : "No") << endl;
}

string DigitalMedia::getType()           const { return "DIGITAL"; }
float  DigitalMedia::getRunTimeMinutes() const { return runTimeMinutes; }
string DigitalMedia::getFileFormat()     const { return fileFormat; }
