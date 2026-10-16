#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

#define MAX_MED 100
#define MAX_BATCH 100
#define MAX_SUPPLIER 5
#define MAX_TRANS 500

struct Medicine {
    int id, stock, dosage;
    string barcode, name, category;
    double price;
};

struct Batch {
    int batchId, medId, quantity, expiry; // expiry: YYYYMMDD
};

struct Supplier {
    int id;
    string name, contact;
};

struct Transaction {
    int id, medId, quantity;
    string type;
    double amount;
};

class Pharmacy {
    Medicine medicines[MAX_MED];
    Batch batches[MAX_BATCH];
    Supplier suppliers[MAX_SUPPLIER];
    Transaction history[MAX_TRANS];
    Transaction undoStack[MAX_TRANS], redoStack[MAX_TRANS];

    int medCount, batchCount;
    int front, rear, supplierCount;
    int historyCount, undoTop, redoTop;
    int nextTransactionId;

public:
    Pharmacy() {
        medCount = 0;
        batchCount = 0;
        front = 0;
        rear = -1;
        supplierCount = 0;
        historyCount = 0;
        undoTop = -1;
        redoTop = -1;
        nextTransactionId = 1;
    }

    int findMedicine(int id) {
        for (int i = 0; i < medCount; i++) {
            if (medicines[i].id == id) return i;
        }
        return -1;
    }

    void addMedicine() {
        if (medCount == MAX_MED) {
            cout << "Medicine list is full.\n";
            return;
        }

        Medicine m;
        cout << "Enter medicine ID: ";
        cin >> m.id;

        if (findMedicine(m.id) != -1) {
            cout << "This ID already exists.\n";
            return;
        }

        cout << "Enter barcode: ";
        cin >> m.barcode;
        cout << "Enter medicine name (one word): ";
        cin >> m.name;
        cout << "Enter category: ";
        cin >> m.category;
        cout << "Enter price: ";
        cin >> m.price;
        cout << "Enter stock quantity: ";
        cin >> m.stock;
        cout << "Enter dosage in mg: ";
        cin >> m.dosage;

        medicines[medCount] = m;
        medCount++;
        cout << "Medicine added successfully.\n";
    }

    void searchMedicine() {
        int id;
        cout << "Enter medicine ID: ";
        cin >> id;
        int i = findMedicine(id);

        if (i == -1) {
            cout << "Medicine not found.\n";
            return;
        }

        cout << "ID: " << medicines[i].id
             << " | Barcode: " << medicines[i].barcode
             << " | Name: " << medicines[i].name
             << " | Category: " << medicines[i].category
             << " | Price: Rs." << fixed << setprecision(2) << medicines[i].price
             << " | Stock: " << medicines[i].stock
             << " | Dosage: " << medicines[i].dosage << " mg\n";
    }

    void displayMedicines() {
        if (medCount == 0) {
            cout << "No medicines found.\n";
            return;
        }

        for (int i = 0; i < medCount; i++) {
            cout << "ID: " << medicines[i].id
                 << " | Name: " << medicines[i].name
                 << " | Barcode: " << medicines[i].barcode
                 << " | Category: " << medicines[i].category
                 << " | Price: Rs." << fixed << setprecision(2) << medicines[i].price
                 << " | Stock: " << medicines[i].stock
                 << " | Dosage: " << medicines[i].dosage << " mg\n";
        }
    }

    void addBatch() {
        if (batchCount == MAX_BATCH) {
            cout << "Batch list is full.\n";
            return;
        }

        Batch b;
        cout << "Enter batch ID: ";
        cin >> b.batchId;
        cout << "Enter medicine ID: ";
        cin >> b.medId;

        if (findMedicine(b.medId) == -1) {
            cout << "Add this medicine first.\n";
            return;
        }

        cout << "Enter batch quantity: ";
        cin >> b.quantity;
        cout << "Enter expiry date (YYYYMMDD): ";
        cin >> b.expiry;

        if (b.quantity <= 0) {
            cout << "Quantity must be positive.\n";
            return;
        }

        batches[batchCount] = b;
        batchCount++;
        cout << "Batch added successfully.\n";
    }

    void displayBatches() {
        if (batchCount == 0) {
            cout << "No batches available.\n";
            return;
        }

        for (int i = 0; i < batchCount; i++) {
            cout << "Batch ID: " << batches[i].batchId
                 << " | Medicine ID: " << batches[i].medId
                 << " | Quantity: " << batches[i].quantity
                 << " | Expiry: " << batches[i].expiry << '\n';
        }
    }

    void nearestExpiry() {
        int best = -1;

        for (int i = 0; i < batchCount; i++) {
            if (batches[i].quantity > 0 &&
                (best == -1 || batches[i].expiry < batches[best].expiry)) {
                best = i;
            }
        }

        if (best == -1) {
            cout << "No batches available.\n";
        } else {
            cout << "Nearest expiry batch: " << batches[best].batchId
                 << " | Medicine ID: " << batches[best].medId
                 << " | Quantity: " << batches[best].quantity
                 << " | Expiry: " << batches[best].expiry << '\n';
        }
    }

    int batchStock(int medId) {
        int total = 0;
        for (int i = 0; i < batchCount; i++) {
            if (batches[i].medId == medId)
                total += batches[i].quantity;
        }
        return total;
    }

    bool sellFromFEFO(int medId, int quantity) {
        if (quantity <= 0 || batchStock(medId) < quantity) return false;

        int remaining = quantity;

        while (remaining > 0) {
            int best = -1;

            for (int i = 0; i < batchCount; i++) {
                if (batches[i].medId == medId && batches[i].quantity > 0 &&
                    (best == -1 || batches[i].expiry < batches[best].expiry)) {
                    best = i;
                }
            }

            if (best == -1) return false;

            int take = batches[best].quantity;
            if (take > remaining) take = remaining;

            batches[best].quantity -= take;
            remaining -= take;
        }
        return true;
    }

    void addSupplier() {
        if (supplierCount == MAX_SUPPLIER) {
            cout << "Supplier queue is full.\n";
            return;
        }

        Supplier s;
        cout << "Enter supplier ID: ";
        cin >> s.id;
        cout << "Enter supplier name (one word): ";
        cin >> s.name;
        cout << "Enter contact: ";
        cin >> s.contact;

        rear = (rear + 1) % MAX_SUPPLIER;
        suppliers[rear] = s;
        supplierCount++;
        cout << "Supplier added to queue.\n";
    }

    void processSupplier() {
        if (supplierCount == 0) {
            cout << "Supplier queue is empty.\n";
            return;
        }

        cout << "Processing supplier: "
             << suppliers[front].id << " | "
             << suppliers[front].name << " | "
             << suppliers[front].contact << '\n';

        front = (front + 1) % MAX_SUPPLIER;
        supplierCount--;
    }

    void displaySuppliers() {
        if (supplierCount == 0) {
            cout << "Supplier queue is empty.\n";
            return;
        }

        for (int i = 0; i < supplierCount; i++) {
            int j = (front + i) % MAX_SUPPLIER;
            cout << "ID: " << suppliers[j].id
                 << " | Name: " << suppliers[j].name
                 << " | Contact: " << suppliers[j].contact << '\n';
        }
    }

    void saveTransaction(Transaction t) {
        if (historyCount < MAX_TRANS)
            history[historyCount++] = t;

        if (undoTop < MAX_TRANS - 1)
            undoStack[++undoTop] = t;

        // A new sale clears redo history.
        redoTop = -1;
    }

    void sellMedicine() {
        int id, quantity;
        cout << "Enter medicine ID: ";
        cin >> id;
        cout << "Enter quantity to sell: ";
        cin >> quantity;

        int i = findMedicine(id);
        if (i == -1) {
            cout << "Medicine not found.\n";
            return;
        }

        if (quantity <= 0 || quantity > medicines[i].stock) {
            cout << "Invalid quantity or insufficient stock.\n";
            return;
        }

        if (!sellFromFEFO(id, quantity)) {
            cout << "Not enough quantity in batches. Sale cancelled.\n";
            return;
        }

        medicines[i].stock -= quantity;

        Transaction t;
        t.id = nextTransactionId++;
        t.medId = id;
        t.quantity = quantity;
        t.type = "SELL";
        t.amount = medicines[i].price * quantity;
        saveTransaction(t);

        cout << "Sale completed using FEFO.\n";
        cout << "Total bill: Rs." << fixed << setprecision(2) << t.amount << '\n';
    }

    void undoSale() {
        if (undoTop == -1) {
            cout << "Nothing to undo.\n";
            return;
        }

        Transaction t = undoStack[undoTop--];
        if (t.type == "SELL") {
            int i = findMedicine(t.medId);
            if (i != -1) medicines[i].stock += t.quantity;

            // Restore quantity to a batch of this medicine.
            int j;
            for (j = 0; j < batchCount; j++) {
                if (batches[j].medId == t.medId) {
                    batches[j].quantity += t.quantity;
                    break;
                }
            }
        }

        if (redoTop < MAX_TRANS - 1)
            redoStack[++redoTop] = t;

        cout << "Sale undone.\n";
    }

    void redoSale() {
        if (redoTop == -1) {
            cout << "Nothing to redo.\n";
            return;
        }

        Transaction t = redoStack[redoTop];
        int i = findMedicine(t.medId);

        if (i == -1 || medicines[i].stock < t.quantity ||
            !sellFromFEFO(t.medId, t.quantity)) {
            cout << "Redo failed: insufficient stock.\n";
            return;
        }

        redoTop--;
        medicines[i].stock -= t.quantity;
        if (undoTop < MAX_TRANS - 1)
            undoStack[++undoTop] = t;

        cout << "Sale redone.\n";
        cout << "Bill amount: Rs." << fixed << setprecision(2) << t.amount << '\n';
    }

    void showHistory() {
        if (historyCount == 0) {
            cout << "No transaction history.\n";
            return;
        }

        for (int i = 0; i < historyCount; i++) {
            cout << "Transaction ID: " << history[i].id
                 << " | Type: " << history[i].type
                 << " | Medicine ID: " << history[i].medId
                 << " | Quantity: " << history[i].quantity
                 << " | Amount: Rs." << fixed << setprecision(2)
                 << history[i].amount << '\n';
        }
    }
};

int main() {
    Pharmacy p;
    int choice;

    do {
        cout << "\n===== SMART PHARMACY INVENTORY SYSTEM =====\n";
        cout << "1. Add medicine\n";
        cout << "2. Search medicine by ID\n";
        cout << "3. Display all medicines\n";
        cout << "4. Add medicine batch\n";
        cout << "5. Show nearest expiry batch (FEFO)\n";
        cout << "6. Display all batches\n";
        cout << "7. Add supplier to queue\n";
        cout << "8. Process next supplier\n";
        cout << "9. Display supplier queue\n";
        cout << "10. Sell medicine / generate bill\n";
        cout << "11. Undo last sale\n";
        cout << "12. Redo last sale\n";
        cout << "13. Display transaction history\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: p.addMedicine(); break;
            case 2: p.searchMedicine(); break;
            case 3: p.displayMedicines(); break;
            case 4: p.addBatch(); break;
            case 5: p.nearestExpiry(); break;
            case 6: p.displayBatches(); break;
            case 7: p.addSupplier(); break;
            case 8: p.processSupplier(); break;
            case 9: p.displaySuppliers(); break;
            case 10: p.sellMedicine(); break;
            case 11: p.undoSale(); break;
            case 12: p.redoSale(); break;
            case 13: p.showHistory(); break;
            case 0: cout << "Thank you for using the system.\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    return 0;
}