# Library and Resource Management System

A C++ console application for managing library resources and members.

## Group Members

Section: C
Anshara Amir   CS-104
Adina Ather    CS-106
Asman Khalid   CS-108
Eifa Siddiqui  CS-111

## Steps to Run the Program

1) Compile

```
g++ -std=c++17 -Wall -o library main.cpp User.cpp Member.cpp Admin.cpp LibraryResource.cpp ResourceTypes.cpp BorrowRecord.cpp Review.cpp FineUtility.cpp LibrarySystem.cpp
```

2) Run

```
.\library.exe     (Windows)
./library         (Linux/Mac)
```



## Data Files

All data is stored in the `data/` folder (created automatically on first run):

| File                       | Contents                    |
|----------------------------|-----------------------------|
| `data/users.txt`           | User accounts               |
| `data/resources.txt`       | Library resources           |
| `data/borrows.txt`         | Borrow records              |
| `data/member_report.txt`   | Generated member report     |
| `data/resource_report.txt` | Generated resource report   |



## Fine Rates

| Condition    | Rate                      |
|--------------|---------------------------|
| Perfect      | PKR 10 per overdue day    |
| Minor Damage | PKR 10/day + PKR 500      |
| Heavy Damage | PKR 10/day + PKR 2000     |
| Lost         | PKR 10/day + PKR 5000     |



## Borrow Rules

- Maximum **2 resources per day** per member (enforced by `BorrowLimitException`)
- Borrow period: **14 days**
- Resources unavailable while issued; available again after Admin processes the return

## OOP Concepts Used in the Project:
1) Inheritance
---



The project implements inheritance through multiple class hierarchies.

- Hierarchy 1: `LibraryResource` 

`LibraryResource` serves as the abstract base class for all library materials.

| Derived Class    | Additional Attributes             |
| ---------------- | --------------------------------- |
| `ScienceBook`    | `scientificField`                 |
| `LiteratureBook` | `literaryEra`                     |
| `Magazine`       | `issueNumber`, `publicationMonth` |
| `ReferenceBook`  | `edition`                         |
| `DigitalMedia`   | `runTimeMinutes`, `fileFormat`    |

These subclasses inherit common resource attributes such as title, writer, ISBN, and publication details while adding their own specialized properties.

---

- Hierarchy 2: `User`

`User` acts as the abstract base class for all system users.

| Derived Class | Purpose                                                     |
| ------------- | ----------------------------------------------------------- |
| `Member`      | Handles borrowing, fines, reservations, and account balance |
| `Admin`       | Handles library system management                           |

The derived classes inherit shared user functionality while implementing role-specific behavior.

---

- Exception Inheritance

The project also demonstrates inheritance through custom exception handling.

```cpp
class BorrowLimitException : public std::exception
```

`BorrowLimitException` extends the standard C++ exception class to provide custom borrowing-limit error handling.

(File: `BorrowLimitException.h`)

---

2) Composition (Strong Ownership)

---

In composition, the owner object controls the lifetime of the contained objects.

- `Member` owns `BorrowRecord*`

Each `Member` object dynamically stores borrow records and deletes them in its destructor.

(File: `Member.cpp`)

```cpp
delete r;
```

---

- `LibrarySystem` owns all `User*` and `LibraryResource*`

`LibrarySystem` manages and deletes all dynamically allocated users and resources.

(File: `LibrarySystem.cpp`)

```cpp
delete user;
delete resource;
```

---

- `LibraryResource` contains `Review` objects

`Review` is a plain concrete class with no inheritance or polymorphism.
Objects of `Review` are stored **by value** inside:

```cpp
vector<Review> reviews;
```

This is another example of composition because the reviews are fully owned and managed by `LibraryResource`.

(Files: `Review.h`, `Review.cpp`)

---

3) Aggregation (Weak Association)
---

In aggregation, objects reference other objects without owning them.

- `LibraryResource` stores `queue<Member*> reservationList`

The reservation queue references members but does not manage their memory.

(File: `LibraryResource.h`)

```cpp
queue<Member*> reservationList;
```

---

- `BorrowRecord` stores `LibraryResource*`

`BorrowRecord` references a library resource that is owned elsewhere by `LibrarySystem`.

(File: `BorrowRecord.h`)

```cpp
LibraryResource* resource;
```

`BorrowRecord` itself is primarily a data-management class and does not introduce additional OOP patterns beyond aggregation.

---

4) Method Overriding (Runtime Polymorphism)
---

The project uses virtual functions extensively to achieve runtime polymorphism.

- Overriding in `LibraryResource` Subclasses

All five derived resource classes override the following virtual methods:

-- `displayInfo() const override`

Displays resource-specific information.

-- `getType() const override`

Returns the type of the resource, such as:

```cpp
"Science Book"
"Magazine"
"Digital Media"
```

(File: `ResourceTypes.cpp`)

---

- Overriding in `User` Subclasses

-- `displayDashboard() const override`

* `Member` displays borrowing records, fines, and account details.
* `Admin` displays administrative controls and system management features.

(Files: `Member.cpp`, `Admin.cpp`)

---

- `getRole() const override`

Returns the role of the user:

```cpp
"MEMBER"
"ADMIN"
```

(Files: `Member.h`, `Admin.h`)

---

- Exception Method Overriding

`BorrowLimitException` overrides the standard exception method:

```cpp
what() const noexcept override
```

This provides a custom error message when borrowing limits are exceeded.

(File: `BorrowLimitException.h`)

---

5) Operator Overloading
---
The project overloads the equality operator inside `LibraryResource`.

- `operator==`

```cpp
bool LibraryResource::operator==(const string& query) const
```

(File: `LibraryResource.cpp`)

-- Functionality

Performs a case-insensitive substring search across:

* ISBN
* Title
* Writer
* Genre

This overloaded operator is used in:

```cpp
searchByKeyword()
```

(File: `LibrarySystem.cpp`)

The operator simplifies searching and improves readability.

---

6) Abstract Classes
---
The system uses abstract classes to define common interfaces for derived classes.



- `LibraryResource` as an Abstract Class

(File: `LibraryResource.h`)

Cannot be instantiated because it contains a pure virtual function:

```cpp
virtual std::string getType() const = 0;
```

---

- `User` as an Abstract Class

(File: `User.h`)

Contains pure virtual functions:

```cpp
virtual void displayDashboard() const = 0;
virtual std::string getRole() const = 0;
```

Therefore, `User` only acts as a blueprint for derived user types.

---

- Protected Members

Both abstract base classes use `protected` attributes so derived classes can directly access inherited data members while implementing overridden methods.

---

7) Exception Handling
---
The project includes extensive exception handling using both standard and custom exceptions.



- Custom Exception Class

-- `BorrowLimitException`

```cpp
class BorrowLimitException : public std::exception
```

Used when a member exceeds the allowed daily borrowing limit.

(File: `BorrowLimitException.h`)

---

- Exceptions in `LibraryResource.cpp`

(File: `LibraryResource.cpp`)

Throws `invalid_argument` exceptions for invalid resource data such as:

* Invalid title
* Invalid writer name
* Incorrect ISBN
* Invalid origin
* Invalid language
* Invalid publication year

Example:

```cpp
throw invalid_argument("Invalid ISBN");
```

---

- Exceptions in `Member.cpp`

-- Invalid Deposit Amount

```cpp
throw invalid_argument(...)
```

Thrown when the deposited amount is less than or equal to zero.

---

-- Overdue Books Exist

```cpp
throw runtime_error(...)
```

Thrown when the member has overdue books.

---

-- Daily Borrow Limit Reached

```cpp
throw BorrowLimitException(...)
```

Thrown when a member exceeds the daily borrowing limit.

---

-- Resource Not Available

```cpp
throw runtime_error(...)
```

Thrown when the requested resource is unavailable.

---

- Try-Catch Blocks in `LibrarySystem.cpp`

The system safely handles exceptions using `try-catch` blocks.

-- Book Issuing Exception Handling

```cpp
try {
    m->issueBook(r);
}
catch (...) {
}
```

(File: `LibrarySystem.cpp`)

Catches all exception subtypes generated during book issuance.

---

-- Deposit Exception Handling

```cpp
try {
    m->depositAmount(amount);
}
catch (invalid_argument&) {
}
```

(File: `LibrarySystem.cpp`)

Handles invalid deposit attempts gracefully.

---

8) Utility and Supporting Classes
---
Some project files support the system design without introducing new major OOP concepts.



- `FineUtility` — Static Utility Class

(Files: `FineUtility.h`, `FineUtility.cpp`)

`FineUtility` is implemented as a static utility class containing helper methods related to fine calculations.

Its constructor is explicitly disabled:

```cpp
FineUtility() = delete;
```

This prevents object instantiation and enforces static-only usage — a useful design technique.

---

- `Review` — Plain Concrete Class

(Files: `Review.h`, `Review.cpp`)

`Review` is a standard concrete class with no inheritance or virtual functions.
Its main contribution is participation in composition through storage inside `LibraryResource`.

---

- `BorrowRecord`

(Files: `BorrowRecord.h`, `BorrowRecord.cpp`)

`BorrowRecord` mainly acts as a data-management class.

It demonstrates aggregation through:

```cpp
LibraryResource* resource;
```

It also includes utility methods such as:

```cpp
calculateLateDays()
```

but does not introduce additional OOP concepts beyond aggregation.

---

- `Enums.h`

Defines simple enumerations such as:

* `BookCondition`
* `MembershipStatus`
* `BookCategory`

These are standard enums and are not directly related to OOP concepts.

## Overall OOP Features Demonstrated

The project demonstrates:

* Inheritance
* Abstract Classes
* Runtime Polymorphism
* Method Overriding
* Composition
* Aggregation
* Operator Overloading
* Exception Handling
* Utility-Class Design
* Dynamic Memory Management

These concepts improve:

* Code reusability
* Scalability
* Maintainability
* Encapsulation of behavior
* Modular system organization
* Error handling reliability

## Resources:

1) https://www.geeksforgeeks.org/cpp/library-management-system-2/
* Based on library standards and technical documentation, we added an ISBN attribute to each book. This helps make book searching easier and keeps book information organized and standardized.

2) https://github.com/abdulsamie10/Library-Management-System
* We added a balance system that lets students check their balance and deposit money while keeping financial records updated.

3) C how to program with an introduction to C++ by Paul Dietal and Harvey Dietal
* For basic C++ concept's understanding
