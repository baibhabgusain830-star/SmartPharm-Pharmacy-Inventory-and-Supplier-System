#include <iostream>
#include <string>
#include "m2_medicine.h"
#include "m1_batch.h"
#include "m3_supplier.h"
#include "m4_transaction.h"

using namespace std;

class PharmacyInventory {
private:
    HashTable medTable;
    MinHeap expiryHeap;
    CircularQueue restockQueue;
    DualStack transactionStack;

public:
    PharmacyInventory() {} 

    // 1. Medicine & Barcode Operations (Member 1)
    void addMedicine(Medicine* med) {
        medTable.insert(med);
        cout << "[Inventory] Added Medicine: " << med->name << " (ID: " << med->id << ", Barcode: " << med->barcode << ")" << endl;
    }

    void scanBarcode(string barcode) {
        cout << "\n[Barcode Scanner] Scanning code: " << barcode << endl;
        Medicine* m = medTable.searchByBarcode(barcode);
        if (m != nullptr) {
            cout << "   -> Success! Details: ";
            m->display();
            cout << endl;
        } else {
            cout << "   -> Error: Medicine with barcode " << barcode << " not found!" << endl;
        }
    }

    void searchMedicineId(int id) {
        cout << "\n[Hash Search] Looking up ID: " << id << endl;
        Medicine* m = medTable.searchById(id);
        if (m != nullptr) {
            cout << "   -> Found! Details: ";
            m->display();
            cout << endl;
        } else {
            cout << "   -> ID not found." << endl;
        }
    }

    // 2. Batch & FEFO Operations (Member 2)
    void addBatch(const Batch& batch) {
        expiryHeap.insert(batch);
        cout << "[Batch Management] Added Batch ID: " << batch.batchId 
             << " for Med ID: " << batch.medId 
             << " | Expiry Date: " << batch.expiryDate 
             << " | Qty: " << batch.quantity << endl;
    }

    void peekNextExpiryBatch() {
        cout << "\n[FEFO Min-Heap] Inspecting next expiring batch (Root):" << endl;
        Batch nextBatch = expiryHeap.getNearestExpiryBatch();
        cout << "   -> Batch ID: " << nextBatch.batchId 
             << " | Med ID: " << nextBatch.medId 
             << " | Expiry: " << nextBatch.expiryDate 
             << " | Qty: " << nextBatch.quantity << endl;
    }

    // 3. Supplier & Restock Operations (Member 3)
    void queueSupplierOrder(const Supplier& supplier) {
        restockQueue.enqueue(supplier);
        cout << "[Supplier Queue] Queued Supplier ID " << supplier.supplierId 
             << " | Name: " << supplier.name << endl;
    }

    void processNextOrder() {
        cout << "\n[Supplier Queue] Processing next restock order..." << endl;
        Supplier supplier = restockQueue.dequeue();
        if (supplier.supplierId != 0) {
            cout << "   -> Fulfilled Supplier ID " << supplier.supplierId 
                 << " | Name: " << supplier.name 
                 << " | Contact: " << supplier.contact << endl;
        } else {
            cout << "   -> No pending supplier orders." << endl;
        }
    }

    // 4. Transaction & Undo/Redo Operations (Member 4)
    void executeTransaction(int transId, string type, int medId, double amount) {
        Transaction tx(transId, type, medId, amount);
        transactionStack.recordAction(tx);
        cout << "[Transaction Stack] Recorded [" << type << "] Transaction ID: " << transId 
             << " | Med ID: " << medId << " | Amount: $" << amount << endl;
    }

    void undoLastOperation() {
        cout << "\n[Undo/Redo Stack] Performing UNDO..." << endl;
        transactionStack.undo();
    }

    void redoLastOperation() {
        cout << "\n[Undo/Redo Stack] Performing REDO..." << endl;
        transactionStack.redo();
    }
};

int main() {
    cout << "=====================================================" << endl;
    cout << "   Smart Pharmacy Inventory & Medication System     " << endl;
    cout << "=====================================================" << endl;

    PharmacyInventory pharmacy;

    //test-1
    cout << "\n--- MODULE 1: MEDICINES & BARCODES ---" << endl;
    Medicine* m1 = new Tablet(101, "8901030501", "Paracetamol", "Analgesic", 2.5, 100, 500);
    Medicine* m2 = new Tablet(102, "8901030502", "Amoxicillin", "Antibiotic", 5.0, 50, 250);
    
    pharmacy.addMedicine(m1);
    pharmacy.addMedicine(m2);
    
    pharmacy.scanBarcode("8901030501");
    pharmacy.searchMedicineId(102);

    //test-2
    cout << "\n--- MODULE 2: BATCH & FEFO EXPIRY ---" << endl;
    Batch b1(1, 101, 50, 20271231);
    Batch b2(2, 101, 30, 20261115); // Earlier expiry
    
    pharmacy.addBatch(b1);
    pharmacy.addBatch(b2);
    pharmacy.peekNextExpiryBatch();

    //test-3
    cout << "\n--- MODULE 3: SUPPLIER & RESTOCK QUEUE ---" << endl;
    Supplier s1(501, "MediCorp Pharma", "9876543210");
    Supplier s2(502, "HealthCare Supplies", "8765432109");
    
    pharmacy.queueSupplierOrder(s1);
    pharmacy.queueSupplierOrder(s2);
    pharmacy.processNextOrder();

    //test-4
    cout << "\n--- MODULE 4: TRANSACTIONS & UNDO/REDO ---" << endl;
    pharmacy.executeTransaction(1001, "SELL", 101, 50.0);
    pharmacy.executeTransaction(1002, "RESTOCK", 102, 250.0);
    
    pharmacy.undoLastOperation();
    pharmacy.redoLastOperation();

    delete m1;
    delete m2;
    return 0;
}