#pragma once
#include "LibraryResource.h"

// Each subclass inherits ALL shared behaviour from LibraryResource.
// The only job of each subclass is to:
//   1. Store its type-specific extra attribute(s)
//   2. Override displayinfo() to show those extra attributes
//   3. Override getType() to return its file-persistence type tag

// ─────────────────────────────────────────────────────────────────────────────
class ScienceBook : public LibraryResource {
private:
    std::string scientificField;

public:
    ScienceBook(const std::string& isbn,  const std::string& title,
                const std::string& writer, int year,
                const std::string& origin, const std::string& language,
                const std::string& genre,  BookCategory category,
                const std::string& scientificField);

    void        displayinfo() const override;
    std::string getType()     const override; // returns "SCIENCE"
    std::string getScientificField() const;
};

// ─────────────────────────────────────────────────────────────────────────────
class LiteratureBook : public LibraryResource {
private:
    std::string literaryEra;

public:
    LiteratureBook(const std::string& isbn,  const std::string& title,
                   const std::string& writer, int year,
                   const std::string& origin, const std::string& language,
                   const std::string& genre,  BookCategory category,
                   const std::string& literaryEra);

    void        displayinfo() const override;
    std::string getType()     const override; // returns "LITERATURE"
    std::string getLiteraryEra() const;
};

// ─────────────────────────────────────────────────────────────────────────────
class Magazine : public LibraryResource {
private:
    int         issueNumber;
    std::string publicationMonth;

public:
    Magazine(const std::string& isbn,  const std::string& title,
             const std::string& writer, int year,
             const std::string& origin, const std::string& language,
             const std::string& genre,  BookCategory category,
             int issueNumber, const std::string& publicationMonth);

    void        displayinfo() const override;
    std::string getType()     const override; // returns "MAGAZINE"
    int         getIssueNumber()      const;
    std::string getPublicationMonth() const;
};

// ─────────────────────────────────────────────────────────────────────────────
class ReferenceBook : public LibraryResource {
private:
    int edition;

public:
    ReferenceBook(const std::string& isbn,  const std::string& title,
                  const std::string& writer, int year,
                  const std::string& origin, const std::string& language,
                  const std::string& genre,  BookCategory category,
                  int edition);

    void        displayinfo() const override;
    std::string getType()     const override; // returns "REFERENCE"
    int         getEdition()  const;
};

// ─────────────────────────────────────────────────────────────────────────────
class DigitalMedia : public LibraryResource {
private:
    float       runTimeMinutes;
    std::string fileFormat;

public:
    DigitalMedia(const std::string& isbn,  const std::string& title,
                 const std::string& writer, int year,
                 const std::string& origin, const std::string& language,
                 const std::string& genre,  BookCategory category,
                 float runTimeMinutes, const std::string& fileFormat);

    void        displayinfo() const override;
    std::string getType()     const override; // returns "DIGITAL"
    float       getRunTimeMinutes() const;
    std::string getFileFormat()     const;
};
