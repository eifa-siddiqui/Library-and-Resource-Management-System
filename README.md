# Library and Resource Management System

A C++ console application for managing library resources and members.
Built as a CS-116 OOP course project demonstrating Inheritance, Abstract Classes,
Method Overriding, Operator Overloading, Composition, and Exception Handling.

---

## Compile

```
g++ -std=c++17 -Wall -o library main.cpp User.cpp Member.cpp Admin.cpp LibraryResource.cpp ResourceTypes.cpp BorrowRecord.cpp Review.cpp FineUtility.cpp LibrarySystem.cpp
```

## Run

```
.\library.exe     (Windows)
./library         (Linux/Mac)
```

---

## First Run

On first launch the system automatically creates default accounts and sample resources:

| Role   | ID   | Password |
|--------|------|----------|
| Admin  | A001 | admin123 |
| Member | M001 | member1  |

Sample resources: Introduction to Physics (S001), Pride and Prejudice (L001), Science Today magazine (M001).

---

## Data Files

All data is stored in the `data/` folder (created automatically on first run):

| File                       | Contents                    |
|----------------------------|-----------------------------|
| `data/users.txt`           | User accounts               |
| `data/resources.txt`       | Library resources           |
| `data/borrows.txt`         | Borrow records              |
| `data/member_report.txt`   | Generated member report     |
| `data/resource_report.txt` | Generated resource report   |

---

## Fine Rates

| Condition    | Rate                      |
|--------------|---------------------------|
| Perfect      | PKR 10 per overdue day    |
| Minor Damage | PKR 10/day + PKR 500      |
| Heavy Damage | PKR 10/day + PKR 2000     |
| Lost         | PKR 10/day + PKR 5000     |

---

## Borrow Rules

- Maximum **2 resources per day** per member (enforced by `BorrowLimitException`)
- Borrow period: **14 days**
- Resources unavailable while issued; available again after Admin processes the return
