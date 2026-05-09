#include "LibraryResource.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
using namespace std;

// Helper: returns a lowercase copy of s
static string toLower(const string& s) {
    string result = s;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// ─── Constructors / Destructor ────────────────────────────────────────────────

LibraryResource::LibraryResource()
    : publicationYear(0), category(BookCategory::NON_FICTION), isavailable(true)
{}

LibraryResource::LibraryResource(const string& isbn, const string& title,
                                  const string& writer, int year,
                                  const string& origin, const string& language,
                                  const string& genre, BookCategory category)
    : isbn(isbn), title(title), writer(writer), publicationYear(year),
      origin(origin), language(language), genre(genre),
      category(category), isavailable(true)
{}

LibraryResource::~LibraryResource() {}

// ─── Setters (taken directly from rev_mem.cpp — logic is same ) ────────────

void LibraryResource::setAvailable(bool val) { isavailable = val; }
bool LibraryResource::isAvailable()          const { return isavailable; }

void LibraryResource::setTitle(const string& t) {
    if (t.empty())
        throw invalid_argument("\033[31m Title cannot be empty.\033[0m");
    for (int i = 0; i < (int)t.size(); i++)
        if (t[i] != ' ') { title = t; return; }
    throw invalid_argument("\033[31m Title cannot be only spaces.\033[0m");
}

void LibraryResource::setWriter(const string& w) {
    if (w.empty())
        throw invalid_argument("\033[31m Writer name cannot be empty.\033[0m");
    for (int i = 0; i < (int)w.size(); i++)
        if (w[i] != ' ') { writer = w; return; }
    throw invalid_argument("\033[31m Writer name cannot be only spaces.\033[0m");
}

void LibraryResource::setISBN(const string& i) {
    if (i.empty())
        throw invalid_argument("\033[31m ISBN cannot be empty.\033[0m");
    if (i.size() != 13)
        throw invalid_argument("\033[31m ISBN must be exactly 13 digits. You entered "
                               + to_string(i.size()) + " characters.\033[0m");
    for (int j = 0; j < (int)i.size(); j++)
        if (i[j] < '0' || i[j] > '9')
            throw invalid_argument("\033[31m ISBN must contain digits only. Invalid character: \033[0m"
                                   + string(1, i[j]));
    isbn = i;
}

void LibraryResource::setOrigin(const string& o) {
    if (o.empty())
        throw invalid_argument("\033[31m Origin cannot be empty.\033[0m");
    for (int i = 0; i < (int)o.size(); i++)
        if (o[i] != ' ') { origin = o; return; }
    throw invalid_argument("\033[31m Origin cannot be only spaces.\033[0m");
}

void LibraryResource::setLanguage(const string& l) {
    if (l.empty())
        throw invalid_argument("\033[31m Language cannot be empty.\033[0m");
    for (int i = 0; i < (int)l.size(); i++)
        if (l[i] != ' ') { language = l; return; }
    throw invalid_argument("\033[31m Language cannot be only spaces.\033[0m");
}

void LibraryResource::setPublicationYear(int y) {
    if (y < 1000 || y > 2026)
        throw invalid_argument("\033[31m Publication year " + to_string(y) +
                               " is not valid. Must be between 1000 and 2026.\033[0m");
    publicationYear = y;
}

void LibraryResource::setGenre(const string& g)    { genre = g; }
void LibraryResource::setCategory(BookCategory c)  { category = c; }

// ─── Getters ─────────────────────────────────────────────────────────────────

string       LibraryResource::getIsbn()            const { return isbn; }
string       LibraryResource::getTitle()           const { return title; }
string       LibraryResource::getWriter()          const { return writer; }
string       LibraryResource::getOrigin()          const { return origin; }
string       LibraryResource::getLanguage()        const { return language; }
int          LibraryResource::getPublicationYear() const { return publicationYear; }
string       LibraryResource::getGenre()           const { return genre; }
BookCategory LibraryResource::getCategory()        const { return category; }
bool         LibraryResource::getIsAvailable()     const { return isavailable; }

// ─── Review management ────────────────────────────────────────────────────────

void LibraryResource::addReview(const Review& r) {
    reviews.push_back(r);
}

const vector<Review>& LibraryResource::getReviews() const {
    return reviews;
}

// ─── Reservation queue ────────────────────────────────────────────────────────

void LibraryResource::addToReservation(Member* m) {
    reservationList.push(m);
}

Member* LibraryResource::nextReservation() {
    if (reservationList.empty()) return nullptr;
    Member* m = reservationList.front();
    reservationList.pop();
    return m;
}

bool LibraryResource::hasReservation() const {
    return !reservationList.empty();
}

int LibraryResource::getReservationCount() const {
    return static_cast<int>(reservationList.size());
}

// ─── operator== ───────────────────────────────────────────────────────────────
// Case-insensitive substring match against isbn, title, writer, genre.
// Used by BookDatabase::searchByKeyword for cross-field search.
// Extends rev_mem.cpp's exact-match version to support partial queries.

bool LibraryResource::operator==(const string& query) const {
    string q = toLower(query);
    return toLower(isbn).find(q)   != string::npos ||
           toLower(title).find(q)  != string::npos ||
           toLower(writer).find(q) != string::npos ||
           toLower(genre).find(q)  != string::npos;
}

// ─── displayinfo (base) ───────────────────────────────────────────────────────
// Prints fields common to all resource types.
// Subclasses override this and add their own type-specific line.

void LibraryResource::displayinfo() const {
    cout << "Title: "     << title           << endl;
    cout << "Author: "    << writer          << endl;
    cout << "ISBN: "      << isbn            << endl;
    cout << "Year: "      << publicationYear << endl;
    cout << "Available: " << (isavailable ? "Yes" : "No") << endl;
}
