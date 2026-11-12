#include <iostream>
#include <string>
#include <fstream>
#include "m1_batch.h"
#include "m2_medicine.h"
#include "m3_supplier.h"
#include "m4_transaction.h"

using namespace std;

struct BillItem {
    int medId;
    string name;
    int qty;
    double unitPrice;
    double subtotal;
};

class PharmacyInventory {
public:
    HashTable medTable;
    CategoryBST categoryTree;
    MinHeap expiryHeap;
    CircularQueue restockQueue;
    SupplierGraph supGraph;
    DualStack transactionStack;
    TransactionHistory historyList;
    int nextTransId;
    int nextBillNo;
    int currentDate;

    PharmacyInventory() {
        nextTransId = 1001;
        nextBillNo = 5001;
        currentDate = 20261110;
    }

    void addMedicine(Medicine* med) {
        medTable.insert(med);
        categoryTree.insert(med);
    }

    Medicine* searchMedicineBinarySearch(int targetId) {
        Medicine** arr = new Medicine*[2000];
        int count = 0;

        for (int i = 0; i < HashTable::SIZE; i++) {
            HashNode* temp = medTable.table[i];
            while (temp != nullptr && count < 2000) {
                arr[count++] = temp->med;
                temp = temp->next;
            }
        }

        if (count == 0) {
            delete[] arr;
            return nullptr;
        }

        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (arr[j]->id > arr[j + 1]->id) {
                    Medicine* temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }

        int idx = binarySearchMedicine(arr, count, targetId);
        Medicine* found = (idx != -1) ? arr[idx] : nullptr;
        delete[] arr;
        return found;
    }

    void printSingleReceipt(int billNo, int transId, Medicine* m, int qty, double totalAmount) {
        cout << "\n=======================================================" << endl;
        cout << "               SMARTPHARM RETAIL INVOICE               " << endl;
        cout << "=======================================================" << endl;
        cout << "Bill No: " << billNo << " | Trans ID: " << transId << " | Date: " << currentDate << endl;
        cout << "-------------------------------------------------------" << endl;
        cout << "Medicine: " << m->name << " (ID: " << m->id << " | Type: " << m->medType << ")" << endl;
        cout << "Unit Price: Rs." << m->price << "  x  Qty: " << qty << endl;
        cout << "-------------------------------------------------------" << endl;
        cout << "GRAND TOTAL: Rs." << totalAmount << endl;
        cout << "Status: PAID (Dispensed via FEFO)" << endl;
        cout << "=======================================================\n" << endl;


        ofstream fout("last_receipt.txt");
        fout << "========== SMARTPHARM RETAIL INVOICE ==========\n";
        fout << "Bill No: " << billNo << " | Trans ID: " << transId << " | Date: " << currentDate << "\n";
        fout << "Medicine: " << m->name << " (ID: " << m->id << ")\n";
        fout << "Unit Price: Rs." << m->price << " x Qty: " << qty << "\n";
        fout << "GRAND TOTAL: Rs." << totalAmount << "\n";
        fout << "===============================================\n";
        fout.close();
    }

    void sellSpecificMedicine(Medicine* m) {
        int qty;
        cout << "Enter Quantity to sell for " << m->name << ": ";
        cin >> qty;

        if (qty <= 0 || m->stock < qty) {
            cout << "Insufficient total stock!" << endl;
            return;
        }

        if (!expiryHeap.sellFromFEFO(m->id, qty, currentDate)) {
            cout << "Sale failed: Not enough non-expired batch stock!" << endl;
            return;
        }

        m->stock -= qty;
        double billAmount = m->price * qty;
        int currentTxId = nextTransId++;
        int currentBill = nextBillNo++;

        Transaction tx(currentTxId, "SELL", m->id, billAmount, qty);
        transactionStack.recordAction(tx);
        historyList.append(tx);

        printSingleReceipt(currentBill, currentTxId, m, qty, billAmount);

        if (m->stock < 20) {
            cout << "[Auto-Restock Alert] Stock for " << m->name << " is low (" << m->stock << "). Enqueuing supplier order!" << endl;
            restockQueue.enqueue(Supplier(599, "AutoRestock_Supplier", "9999999999"));
        }
    }

    void generateCustomerBill() {
        string customerName;
        cout << "\n--- NEW CUSTOMER BILLING COUNTER ---" << endl;
        cout << "Enter Customer Name (one_word): ";
        cin >> customerName;

        BillItem cart[50];
        int itemCount = 0;
        double grandTotal = 0.0;
        int addMore = 1;

        while (addMore == 1 && itemCount < 50) {
            int mode;
            cout << "\nAdd Item #" << (itemCount + 1) << " -> Search by (1: Barcode, 2: Medicine ID): ";
            cin >> mode;

            Medicine* m = nullptr;
            if (mode == 1) {
                string code;
                cout << "Scan/Enter Barcode: ";
                cin >> code;
                m = medTable.searchByBarcode(code);
            } else {
                int id;
                cout << "Enter Medicine ID: ";
                cin >> id;
                m = medTable.searchById(id);
            }

            if (m == nullptr) {
                cout << "Medicine not found!" << endl;
            } else {
                m->display();
                int availFEFO = expiryHeap.getValidBatchStock(m->id, currentDate);
                cout << "Available Non-Expired FEFO Stock: " << availFEFO << endl;

                int qty;
                cout << "Enter Quantity to buy: ";
                cin >> qty;

                if (qty <= 0 || qty > m->stock || qty > availFEFO) {
                    cout << "Invalid quantity or insufficient non-expired stock!" << endl;
                } else {
                   
                    expiryHeap.sellFromFEFO(m->id, qty, currentDate);
                    m->stock -= qty;

                    double sub = m->price * qty;
                    cart[itemCount].medId = m->id;
                    cart[itemCount].name = m->name;
                    cart[itemCount].qty = qty;
                    cart[itemCount].unitPrice = m->price;
                    cart[itemCount].subtotal = sub;
                    itemCount++;
                    grandTotal += sub;

                    
                    Transaction tx(nextTransId++, "SELL", m->id, sub, qty);
                    transactionStack.recordAction(tx);
                    historyList.append(tx);

                    cout << "Added " << qty << " x " << m->name << " (Rs." << sub << ") to bill." << endl;

                    if (m->stock < 20) {
                        cout << "[Auto-Restock Alert] Stock for " << m->name << " is low (" << m->stock << "). Enqueuing supplier order!" << endl;
                        restockQueue.enqueue(Supplier(599, "AutoRestock_Supplier", "9999999999"));
                    }
                }
            }

            cout << "\nAdd another medicine to this bill? (1: Yes, 0: Checkout & Print Bill): ";
            cin >> addMore;
        }

        if (itemCount == 0) {
            cout << "No items billed. Returning to menu." << endl;
            return;
        }

        int billNo = nextBillNo++;

       
        cout << "\n=======================================================" << endl;
        cout << "               SMARTPHARM FINAL INVOICE                " << endl;
        cout << "=======================================================" << endl;
        cout << "Bill No: " << billNo << " | Customer: " << customerName << " | Date: " << currentDate << endl;
        cout << "-------------------------------------------------------" << endl;
        cout << "MedID | Medicine Name | Qty | Unit Price | Subtotal" << endl;
        cout << "-------------------------------------------------------" << endl;
        for (int i = 0; i < itemCount; i++) {
            cout << cart[i].medId << " | " << cart[i].name << " | "
                 << cart[i].qty << " | Rs." << cart[i].unitPrice
                 << " | Rs." << cart[i].subtotal << endl;
        }
        cout << "-------------------------------------------------------" << endl;
        cout << "Total Items: " << itemCount << " | GRAND TOTAL: Rs." << grandTotal << endl;
        cout << "Dispensed via FEFO (First-Expire-First-Out)" << endl;
        cout << "=======================================================\n" << endl;

        ofstream fout("last_receipt.txt");
        fout << "=============== SMARTPHARM FINAL INVOICE ===============\n";
        fout << "Bill No: " << billNo << " | Customer: " << customerName << " | Date: " << currentDate << "\n";
        fout << "--------------------------------------------------------\n";
        for (int i = 0; i < itemCount; i++) {
            fout << cart[i].medId << " | " << cart[i].name << " | Qty: " << cart[i].qty
                 << " | Price: Rs." << cart[i].unitPrice << " | Subtotal: Rs." << cart[i].subtotal << "\n";
        }
        fout << "--------------------------------------------------------\n";
        fout << "GRAND TOTAL: Rs." << grandTotal << "\n";
        fout << "========================================================\n";
        fout.close();
        cout << "(Invoice copy saved to last_receipt.txt)" << endl;
    }

    void shopkeeperScanAndActWorkflow() {
        int mode;
        cout << "\n--- SHOPKEEPER SCAN / LOOKUP COUNTER ---" << endl;
        cout << "1. Scan Barcode   2. Search by ID (Hash O(1))   3. Search by ID (Binary Search): ";
        cin >> mode;

        Medicine* m = nullptr;
        if (mode == 1) {
            string code;
            cout << "Scan/Enter Barcode: ";
            cin >> code;
            m = medTable.searchByBarcode(code);
        } else if (mode == 2) {
            int id;
            cout << "Enter Medicine ID: ";
            cin >> id;
            m = medTable.searchById(id);
        } else if (mode == 3) {
            int id;
            cout << "Enter Medicine ID (Binary Search): ";
            cin >> id;
            m = searchMedicineBinarySearch(id);
        }

        if (m == nullptr) {
            cout << "Medicine not found in database!" << endl;
            return;
        }

        cout << "\n=======================================================" << endl;
        cout << "MEDICINE FOUND: ";
        m->display();
        expiryHeap.displayBatchesForMed(m->id, currentDate);
        cout << "=======================================================" << endl;

        int action;
        do {
            cout << "\nWhat would you like to do with [" << m->name << "]?" << endl;
            cout << "1. Sell & Print Bill for this Medicine (FEFO Deduction)" << endl;
            cout << "2. Add a New Stock Batch for this Medicine" << endl;
            cout << "3. Recall/Remove a Defective or Expired Batch" << endl;
            cout << "0. Return to Main Menu" << endl;
            cout << "Enter action: ";
            cin >> action;

            switch (action) {
                case 1: {
                    sellSpecificMedicine(m);
                    expiryHeap.displayBatchesForMed(m->id, currentDate);
                    break;
                }
                case 2: {
                    int bId, q, exp;
                    cout << "Enter New BatchID, Quantity, ExpiryDate(YYYYMMDD): ";
                    cin >> bId >> q >> exp;
                    if (!isValidDate(exp)) {
                        cout << "Invalid date format!" << endl;
                    } else {
                        expiryHeap.insert(Batch(bId, m->id, q, exp));
                        m->stock += q;
                        cout << "Batch added and stock updated!" << endl;
                        expiryHeap.displayBatchesForMed(m->id, currentDate);
                    }
                    break;
                }
                case 3: {
                    int bId;
                    cout << "Enter Batch ID to recall: ";
                    cin >> bId;
                    expiryHeap.recallBatch(bId);
                    expiryHeap.displayBatchesForMed(m->id, currentDate);
                    break;
                }
                case 0:
                    break;
                default:
                    cout << "Invalid action." << endl;
            }
        } while (action != 0);
    }

    void loadAllCSV() {
        ifstream fin("medicines.csv");
        if (fin) {
            string type, b, n, c;
            int id, s, extra;
            double p;
            while (fin >> type >> id >> b >> n >> c >> p >> s >> extra) {
                if (type == "TABLET") addMedicine(new Tablet(id, b, n, c, p, s, extra));
                else if (type == "SYRUP") addMedicine(new Syrup(id, b, n, c, p, s, extra));
                else addMedicine(new Injection(id, b, n, c, p, s));
            }
            fin.close();
        } else {
            addMedicine(new Tablet(101, "890101", "Dolo_650", "Analgesic", 30.5, 120, 650));
            addMedicine(new Tablet(102, "890102", "Augmentin_625", "Antibiotic", 201.0, 85, 625));
            addMedicine(new Syrup(103, "890103", "Ascoril_LS", "Antitussive", 118.0, 45, 100));
            addMedicine(new Injection(104, "890104", "Human_Mixtard", "Hormone", 450.0, 25));

            expiryHeap.insert(Batch(1, 101, 60, 20270515));
            expiryHeap.insert(Batch(2, 101, 60, 20261201));
            expiryHeap.insert(Batch(3, 102, 85, 20261125));
            expiryHeap.insert(Batch(4, 103, 45, 20261105));

            restockQueue.enqueue(Supplier(501, "Cipla_Distributors", "9876543210"));
            supGraph.addLink(501, 101);
            supGraph.addLink(501, 102);
        }

        expiryHeap.loadBatchesCSV();
        restockQueue.loadSuppliersCSV();
        historyList.loadTransactionsCSV();

        if (supGraph.vertexCount == 0) {
            supGraph.addLink(501, 101);
            supGraph.addLink(501, 102);
        }
    }

    void saveAllCSV() {
        medTable.saveMedicinesCSV();
        expiryHeap.saveBatchesCSV();
        restockQueue.saveSuppliersCSV();
        historyList.saveTransactionsCSV();
        cout << "All records saved to CSV files successfully!" << endl;
    }

    void undoLastSale() {
        Transaction t = transactionStack.undo();
        if (t.transId != 0 && t.type == "SELL") {
            Medicine* m = medTable.searchById(t.medId);
            if (m != nullptr) m->stock += t.quantity;
            expiryHeap.restoreBatchStock(t.medId, t.quantity);
            cout << "Restored " << t.quantity << " units back to Medicine ID " << t.medId << endl;
        }
    }

    void redoLastSale() {
        Transaction t = transactionStack.redo();
        if (t.transId != 0 && t.type == "SELL") {
            Medicine* m = medTable.searchById(t.medId);
            if (m != nullptr && m->stock >= t.quantity) {
                m->stock -= t.quantity;
                expiryHeap.sellFromFEFO(t.medId, t.quantity, currentDate);
                cout << "Re-applied sale of " << t.quantity << " units for Medicine ID " << t.medId << endl;
            }
        }
    }
};

int main() {
    PharmacyInventory pharmacy;
    pharmacy.loadAllCSV();

    int choice;
    do {
        cout << "\n===== SMARTPHARM: SHOPKEEPER INVENTORY SYSTEM =====" << endl;
        cout << "1. Scan Barcode / Search Medicine (Inspect Batches & Sell/Manage)" << endl;
        cout << "2. Generate Customer Bill (Multi-Item Cart & FEFO Invoice)" << endl;
        cout << "3. Add New Medicine to Catalog (Tablet / Syrup / Injection)" << endl;
        cout << "4. Display All Medicines (Hash Table) & Browse by Category (BST)" << endl;
        cout << "5. Add Medicine Batch Directly (FEFO Min-Heap)" << endl;
        cout << "6. Show Global Nearest Expiry Batch & Scan All Expiry Alerts" << endl;
        cout << "7. Display All Batches in Inventory" << endl;
        cout << "8. Undo / Redo Last Sale Transaction" << endl;
        cout << "9. Supplier Restock Queue (FIFO) & Supplier-Medicine Graph (DFS)" << endl;
        cout << "10. View Transaction History (DLL) & QuickSort Profit Ranking" << endl;
        cout << "0. Save to CSV & Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                pharmacy.shopkeeperScanAndActWorkflow();
                break;
            }
            case 2: {
                pharmacy.generateCustomerBill();
                break;
            }
            case 3: {
                int type, id, s;
                string b, n, c;
                double p;
                cout << "Select Type (1: Tablet, 2: Syrup, 3: Injection): ";
                cin >> type;
                cout << "Enter ID, Barcode, Name(one_word), Category, Price, Stock: ";
                cin >> id >> b >> n >> c >> p >> s;
                if (type == 1) {
                    int mg; cout << "Enter Dosage (mg): "; cin >> mg;
                    pharmacy.addMedicine(new Tablet(id, b, n, c, p, s, mg));
                } else if (type == 2) {
                    int ml; cout << "Enter Volume (ml): "; cin >> ml;
                    pharmacy.addMedicine(new Syrup(id, b, n, c, p, s, ml));
                } else {
                    pharmacy.addMedicine(new Injection(id, b, n, c, p, s));
                }
                cout << "Medicine added!" << endl;
                break;
            }
            case 4: {
                int sub;
                cout << "1. Display All (Hash Table)   2. Display Ordered by Category (BST): ";
                cin >> sub;
                if (sub == 1) pharmacy.medTable.displayAll();
                else pharmacy.categoryTree.displayOrderedByCategory();
                break;
            }
            case 5: {
                int bId, mId, q, exp;
                cout << "Enter BatchID, MedID, Quantity, Expiry(YYYYMMDD): ";
                cin >> bId >> mId >> q >> exp;
                if (!isValidDate(exp)) {
                    cout << "Invalid date format!" << endl;
                } else {
                    pharmacy.expiryHeap.insert(Batch(bId, mId, q, exp));
                    Medicine* m = pharmacy.medTable.searchById(mId);
                    if (m) m->stock += q;
                    cout << "Batch added to Min-Heap!" << endl;
                }
                break;
            }
            case 6: {
                cout << "\nGlobal Nearest Expiry Batch (FEFO Root): ";
                Batch b = pharmacy.expiryHeap.getNearestExpiryBatch();
                if (b.batchId != 0) b.display(); else cout << "None" << endl;
                pharmacy.expiryHeap.scanExpiryAlerts(pharmacy.currentDate);
                break;
            }
            case 7: {
                pharmacy.expiryHeap.displayAll();
                break;
            }
            case 8: {
                int sub;
                cout << "Enter 1 for Undo, 2 for Redo: ";
                cin >> sub;
                if (sub == 1) pharmacy.undoLastSale();
                else pharmacy.redoLastSale();
                break;
            }
            case 9: {
                int sub;
                cout << "1. Enqueue Supplier  2. Process Next Supplier  3. View Queue  4. Link Supplier-Med & Run DFS: ";
                cin >> sub;
                if (sub == 1) {
                    int sId; string n, c;
                    cout << "Enter SupplierID, Name(one_word), Contact: ";
                    cin >> sId >> n >> c;
                    pharmacy.restockQueue.enqueue(Supplier(sId, n, c));
                } else if (sub == 2) {
                    Supplier s = pharmacy.restockQueue.dequeue();
                    if (s.supplierId != 0) { cout << "Processed: "; s.display(); }
                } else if (sub == 3) {
                    pharmacy.restockQueue.displayQueue();
                } else if (sub == 4) {
                    int sId, mId;
                    cout << "Enter SupplierID and MedID to link: ";
                    cin >> sId >> mId;
                    pharmacy.supGraph.addLink(sId, mId);
                    pharmacy.supGraph.runDFS();
                }
                break;
            }
            case 10: {
                cout << "\n--- Chronological Transaction History (Doubly Linked List) ---" << endl;
                pharmacy.historyList.displayAll();
                pharmacy.historyList.displayProfitRankingQuickSort();
                break;
            }
            case 0: {
                pharmacy.saveAllCSV();
                cout << "Exiting SmartPharm. Goodbye!" << endl;
                break;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        }
    } while (choice != 0);

    return 0;
}