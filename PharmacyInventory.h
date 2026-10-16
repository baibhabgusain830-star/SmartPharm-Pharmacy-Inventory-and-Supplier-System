#ifndef PHARMACY_INVENTORY_H
#define PHARMACY_INVENTORY_H

#include "Medicine.h"
#include "Batch.h"
#include "Supplier.h"
#include "Transaction.h"
#include "TransactionList.h"
#include "BatchMinHeap.h"
#include "DateUtils.h"

#include <string>
#include <stack>
#include <queue>

// Structure to record batch depletion details during a multi-batch FEFO sale
struct BatchDepletion {
    std::string batchId;
    int quantity;
};

// Undo record for reversing a multi-batch FEFO sale
// Sized to MAX_BATCHES so array overflow is mathematically impossible
struct SaleUndoRecord {
    int transactionId;
    std::string medicineId;
    BatchDepletion depletions[200]; // Sized to MAX_BATCHES
    int depletionCount;
    int totalQuantity;
    double totalAmount;
    std::string date;

    SaleUndoRecord() 
        : transactionId(0), medicineId(""), depletionCount(0), 
          totalQuantity(0), totalAmount(0.0), date("") {}
};

// Request record for FIFO restock queue
struct RestockRequest {
    int requestId;
    std::string medicineId;
    int quantity;
    std::string supplierId;
    std::string notes;

    RestockRequest() 
        : requestId(0), medicineId(""), quantity(0), supplierId(""), notes("") {}

    RestockRequest(int id, const std::string& mId, int qty, 
                   const std::string& sId, const std::string& n)
        : requestId(id), medicineId(mId), quantity(qty), supplierId(sId), notes(n) {}
};

class PharmacyInventory {
public:
    static const int MAX_MEDICINES = 100;
    static const int MAX_BATCHES = 200;
    static const int MAX_SUPPLIERS = 50;

private:
    Medicine medicines[MAX_MEDICINES];
    int medicineCount;

    Batch batches[MAX_BATCHES];
    int batchCount;

    Supplier suppliers[MAX_SUPPLIERS];
    int supplierCount;

    TransactionList transactionHistory;       // Custom Singly Linked List
    std::stack<SaleUndoRecord> undoStack;      // STL Stack for LIFO Undo
    std::queue<RestockRequest> restockQueue;   // STL Queue for FIFO Restock Requests

    int nextTransactionId;
    int nextRestockRequestId;
    std::string systemDate;                   // Default: "2026-10-15"

    // Internal helper methods
    int findMedicineIndexById(const std::string& id) const;
    int findMedicineIndexByBarcode(const std::string& barcode) const;
    int findBatchIndexById(const std::string& bId) const;
    int findSupplierIndexById(const std::string& sId) const;

public:
    PharmacyInventory();
    ~PharmacyInventory();

    // Prevent unintentional shallow copy
    PharmacyInventory(const PharmacyInventory&) = delete;
    PharmacyInventory& operator=(const PharmacyInventory&) = delete;

    // System Date management for testing expiry
    bool setSystemDate(const std::string& newDate);
    std::string getSystemDate() const;

    // Counts
    int getMedicineCount() const;
    int getBatchCount() const;
    int getSupplierCount() const;

    // Medicine operations (OOP & Linear Search)
    bool addMedicine(const Medicine& med);
    void displayAllMedicines() const;
    int searchMedicine(const std::string& query) const; // by ID or Barcode
    void displayMedicineDetails(int index) const;

    // Batch operations
    bool addBatch(const Batch& batch);
    void displayAllBatches() const;
    int getAvailableStock(const std::string& medId) const; // non-expired stock only

    // Supplier operations
    bool addSupplier(const Supplier& sup);
    void displayAllSuppliers() const;

    // Core DSA Milestone: FEFO Sale using Min-Heap
    bool sellMedicineFEFO(const std::string& query, int requestedQty);

    // Core DSA Milestone: Undo last sale using Stack
    bool undoLastSale();
    int getUndoStackSize() const;

    // Core DSA Milestone: Restock FIFO Queue
    bool enqueueRestockRequest(const std::string& medId, int qty, 
                               const std::string& supId, const std::string& notes);
    bool processNextRestockRequest(const std::string& batchId, const std::string& expDate, double cost);
    void displayRestockQueue() const;
    int getRestockQueueSize() const;

    // Core DSA Milestone: Sorting (Insertion Sort) for Inventory Audit
    void displaySortedInventoryByName();
    void displaySortedInventoryById();

    // Transaction History (Custom Singly Linked List)
    void displayTransactionHistory() const;

    // File Persistence (fstream with safe validation)
    bool loadFromFile(const std::string& medFile = "medicines.txt",
                      const std::string& batchFile = "batches.txt",
                      const std::string& supFile = "suppliers.txt",
                      const std::string& txnFile = "transactions.txt",
                      const std::string& restockFile = "restock_requests.txt",
                      const std::string& undoFile = "undo_stack.txt");

    bool saveToFile(const std::string& medFile = "medicines.txt",
                    const std::string& batchFile = "batches.txt",
                    const std::string& supFile = "suppliers.txt",
                    const std::string& txnFile = "transactions.txt",
                    const std::string& restockFile = "restock_requests.txt",
                    const std::string& undoFile = "undo_stack.txt") const;
};

#endif // PHARMACY_INVENTORY_H
