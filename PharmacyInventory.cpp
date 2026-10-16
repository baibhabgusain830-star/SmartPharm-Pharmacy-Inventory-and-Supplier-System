#include "PharmacyInventory.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <climits>
#include <sstream>

PharmacyInventory::PharmacyInventory()
    : medicineCount(0), batchCount(0), supplierCount(0),
      nextTransactionId(1001), nextRestockRequestId(1),
      systemDate("2026-10-15") {}

PharmacyInventory::~PharmacyInventory() {}

bool PharmacyInventory::setSystemDate(const std::string& newDate) {
    if (DateUtils::isValidDate(newDate)) {
        systemDate = newDate;
        std::cout << "[System] System date successfully updated to: " << systemDate << "\n";
        return true;
    }
    std::cout << "[Error] Invalid calendar date '" << newDate 
              << "'. Must be a real date in YYYY-MM-DD format.\n";
    return false;
}

std::string PharmacyInventory::getSystemDate() const {
    return systemDate;
}

int PharmacyInventory::getMedicineCount() const {
    return medicineCount;
}

int PharmacyInventory::getBatchCount() const {
    return batchCount;
}

int PharmacyInventory::getSupplierCount() const {
    return supplierCount;
}

int PharmacyInventory::getUndoStackSize() const {
    return static_cast<int>(undoStack.size());
}

int PharmacyInventory::getRestockQueueSize() const {
    return static_cast<int>(restockQueue.size());
}

int PharmacyInventory::findMedicineIndexById(const std::string& id) const {
    for (int i = 0; i < medicineCount; ++i) {
        if (medicines[i].getMedicineId() == id) {
            return i;
        }
    }
    return -1;
}

int PharmacyInventory::findMedicineIndexByBarcode(const std::string& barcode) const {
    for (int i = 0; i < medicineCount; ++i) {
        if (medicines[i].getBarcode() == barcode) {
            return i;
        }
    }
    return -1;
}

int PharmacyInventory::findBatchIndexById(const std::string& bId) const {
    for (int i = 0; i < batchCount; ++i) {
        if (batches[i].getBatchId() == bId) {
            return i;
        }
    }
    return -1;
}

int PharmacyInventory::findSupplierIndexById(const std::string& sId) const {
    for (int i = 0; i < supplierCount; ++i) {
        if (suppliers[i].getSupplierId() == sId) {
            return i;
        }
    }
    return -1;
}

bool PharmacyInventory::addMedicine(const Medicine& med) {
    if (!med.isValid()) {
        std::cout << "[Error] Cannot add medicine. One or more fields are invalid.\n";
        return false;
    }
    if (medicineCount >= MAX_MEDICINES) {
        std::cout << "[Error] Medicine storage capacity (" << MAX_MEDICINES << ") reached.\n";
        return false;
    }
    if (findMedicineIndexById(med.getMedicineId()) != -1) {
        std::cout << "[Error] Medicine with ID '" << med.getMedicineId() << "' already exists.\n";
        return false;
    }
    if (findMedicineIndexByBarcode(med.getBarcode()) != -1) {
        std::cout << "[Error] Medicine with Barcode '" << med.getBarcode() << "' already exists.\n";
        return false;
    }

    medicines[medicineCount++] = med;
    std::cout << "[Success] Medicine '" << med.getName() << "' registered successfully.\n";
    return true;
}

void PharmacyInventory::displayAllMedicines() const {
    if (medicineCount == 0) {
        std::cout << " (No medicines registered in catalog)\n";
        return;
    }
    std::cout << " ================================= MEDICINE CATALOG =================================\n";
    std::cout << std::left 
              << std::setw(12) << "Med ID" 
              << std::setw(24) << "Name" 
              << std::setw(16) << "Barcode" 
              << std::setw(16) << "Category" 
              << "Unit Price\n";
    std::cout << "------------------------------------------------------------------------------------\n";
    for (int i = 0; i < medicineCount; ++i) {
        medicines[i].display();
    }
    std::cout << "------------------------------------------------------------------------------------\n";
    std::cout << " Total Medicines: " << medicineCount << "\n";
}

int PharmacyInventory::searchMedicine(const std::string& query) const {
    if (query.empty()) return -1;
    int idx = findMedicineIndexById(query);
    if (idx != -1) return idx;
    return findMedicineIndexByBarcode(query);
}

void PharmacyInventory::displayMedicineDetails(int index) const {
    if (index < 0 || index >= medicineCount) {
        std::cout << "[Error] Invalid medicine index.\n";
        return;
    }
    const Medicine& med = medicines[index];
    std::cout << "\n--- Medicine Details ---\n";
    std::cout << " ID:           " << med.getMedicineId() << "\n";
    std::cout << " Name:         " << med.getName() << "\n";
    std::cout << " Barcode:      " << med.getBarcode() << "\n";
    std::cout << " Category:     " << med.getCategory() << "\n";
    std::cout << " Selling Price:Rs. " << std::fixed << std::setprecision(2) << med.getUnitPrice() << "\n";
    std::cout << " Active Stock: " << getAvailableStock(med.getMedicineId()) 
              << " units (non-expired relative to " << systemDate << ")\n";
}

bool PharmacyInventory::addBatch(const Batch& batch) {
    if (!batch.isValid()) {
        std::cout << "[Error] Cannot add batch. Invalid batch parameters or date.\n";
        return false;
    }
    if (batch.getQuantity() <= 0) {
        std::cout << "[Error] Cannot add batch. Quantity must be strictly positive (> 0).\n";
        return false;
    }
    if (batchCount >= MAX_BATCHES) {
        std::cout << "[Error] Batch storage capacity (" << MAX_BATCHES << ") reached.\n";
        return false;
    }
    if (findMedicineIndexById(batch.getMedicineId()) == -1) {
        std::cout << "[Error] Cannot add batch. Medicine ID '" << batch.getMedicineId() << "' not found.\n";
        return false;
    }
    if (findBatchIndexById(batch.getBatchId()) != -1) {
        std::cout << "[Error] Batch ID '" << batch.getBatchId() << "' already exists.\n";
        return false;
    }

    batches[batchCount++] = batch;
    std::cout << "[Success] Batch '" << batch.getBatchId() << "' added for Medicine " 
              << batch.getMedicineId() << " (Qty: " << batch.getQuantity() 
              << ", Expiry: " << batch.getExpiryDate() << ").\n";

    Transaction purchaseTxn(nextTransactionId++, "PURCHASE", batch.getMedicineId(), 
                            batch.getBatchId(), batch.getQuantity(), 
                            batch.getPurchasePrice(), 
                            batch.getQuantity() * batch.getPurchasePrice(), 
                            systemDate);
    transactionHistory.insertLast(purchaseTxn);
    return true;
}

void PharmacyInventory::displayAllBatches() const {
    if (batchCount == 0) {
        std::cout << " (No batches found in inventory)\n";
        return;
    }
    std::cout << " ================================= BATCH REPOSITORY =================================\n";
    std::cout << std::left 
              << std::setw(12) << "Batch ID" 
              << std::setw(14) << "Med ID" 
              << std::setw(14) << "Expiry Date" 
              << std::setw(10) << "Quantity" 
              << "Cost Price  Status\n";
    std::cout << "------------------------------------------------------------------------------------\n";
    for (int i = 0; i < batchCount; ++i) {
        batches[i].display();
        bool expired = batches[i].isExpired(systemDate);
        if (expired) {
            std::cout << "   [EXPIRED on/before " << systemDate << " - CANNOT SELL]\n";
        }
    }
    std::cout << "------------------------------------------------------------------------------------\n";
    std::cout << " Expiry Rule: Batches expiring on or before " << systemDate << " are marked EXPIRED.\n";
}

int PharmacyInventory::getAvailableStock(const std::string& medId) const {
    int total = 0;
    for (int i = 0; i < batchCount; ++i) {
        if (batches[i].getMedicineId() == medId) {
            if (!batches[i].isExpired(systemDate)) {
                total += batches[i].getQuantity();
            }
        }
    }
    return total;
}

bool PharmacyInventory::addSupplier(const Supplier& sup) {
    if (!sup.isValid()) {
        std::cout << "[Error] Cannot add supplier. All fields must be non-empty.\n";
        return false;
    }
    if (supplierCount >= MAX_SUPPLIERS) {
        std::cout << "[Error] Supplier capacity (" << MAX_SUPPLIERS << ") reached.\n";
        return false;
    }
    if (findSupplierIndexById(sup.getSupplierId()) != -1) {
        std::cout << "[Error] Supplier ID '" << sup.getSupplierId() << "' already exists.\n";
        return false;
    }
    suppliers[supplierCount++] = sup;
    std::cout << "[Success] Supplier '" << sup.getName() << "' registered successfully.\n";
    return true;
}

void PharmacyInventory::displayAllSuppliers() const {
    if (supplierCount == 0) {
        std::cout << " (No suppliers registered)\n";
        return;
    }
    std::cout << " ================================= REGISTERED SUPPLIERS =================================\n";
    std::cout << std::left 
              << std::setw(12) << "Supplier ID" 
              << std::setw(24) << "Name" 
              << std::setw(18) << "Contact" 
              << "Email\n";
    std::cout << "-----------------------------------------------------------------------------------------\n";
    for (int i = 0; i < supplierCount; ++i) {
        suppliers[i].display();
    }
    std::cout << "-----------------------------------------------------------------------------------------\n";
}

bool PharmacyInventory::sellMedicineFEFO(const std::string& query, int requestedQty) {
    int medIdx = searchMedicine(query);
    if (medIdx == -1) {
        std::cout << "[Error] Medicine not found for query '" << query << "'.\n";
        return false;
    }

    const Medicine& med = medicines[medIdx];
    std::string medId = med.getMedicineId();

    if (requestedQty <= 0) {
        std::cout << "[Error] Requested quantity must be strictly positive (> 0). Given: " 
                  << requestedQty << "\n";
        return false;
    }

    int availableStock = getAvailableStock(medId);
    if (requestedQty > availableStock) {
        std::cout << "[Error] Insufficient eligible stock for '" << med.getName() << "'!\n";
        std::cout << " Non-expired available: " << availableStock 
                  << " units | Requested: " << requestedQty << " units.\n";
        return false;
    }

    // Build Candidate Min-Heap containing only non-expired batches for this medicine
    BatchMinHeap candidateHeap(MAX_BATCHES);
    for (int i = 0; i < batchCount; ++i) {
        if (batches[i].getMedicineId() == medId && 
            batches[i].getQuantity() > 0 && 
            !batches[i].isExpired(systemDate)) {
            candidateHeap.insert(batches[i]);
        }
    }

    if (candidateHeap.isEmpty()) {
        std::cout << "[Error] No eligible active batches available for FEFO dispatch.\n";
        return false;
    }

    std::cout << "\n>>> Initiating FEFO Order Dispatch for " << requestedQty << " unit(s) of '" 
              << med.getName() << "' (ID: " << medId << ") <<<\n";
    candidateHeap.displayHeap();

    // Prepare Undo Record (with capacity of MAX_BATCHES, overflow is mathematically impossible)
    SaleUndoRecord undoRec;
    undoRec.transactionId = nextTransactionId++;
    undoRec.medicineId = medId;
    undoRec.depletionCount = 0;
    undoRec.totalQuantity = requestedQty;
    undoRec.totalAmount = requestedQty * med.getUnitPrice();
    undoRec.date = systemDate;

    int remaining = requestedQty;

    // Extract earliest-expiring batches and deplete iteratively
    while (remaining > 0 && !candidateHeap.isEmpty()) {
        Batch topBatch;
        candidateHeap.extractMin(topBatch);

        int bIdx = findBatchIndexById(topBatch.getBatchId());
        if (bIdx == -1) continue;

        int availableInThisBatch = batches[bIdx].getQuantity();
        int toDeduct = (availableInThisBatch < remaining) ? availableInThisBatch : remaining;

        if (toDeduct > 0) {
            batches[bIdx].deductQuantity(toDeduct);
            remaining -= toDeduct;

            // Defensive bounds check
            if (undoRec.depletionCount < MAX_BATCHES) {
                undoRec.depletions[undoRec.depletionCount].batchId = batches[bIdx].getBatchId();
                undoRec.depletions[undoRec.depletionCount].quantity = toDeduct;
                undoRec.depletionCount++;
            }

            std::cout << " -> Depleted " << toDeduct << " unit(s) from Batch [" 
                      << batches[bIdx].getBatchId() << "] (Expiry: " 
                      << batches[bIdx].getExpiryDate() << "). Remaining in batch: " 
                      << batches[bIdx].getQuantity() << "\n";
        }
    }

    // Push undo record onto LIFO Stack
    undoStack.push(undoRec);

    // Record sale in custom Singly Linked List
    std::string batchDesc = (undoRec.depletionCount == 1 ? undoRec.depletions[0].batchId : "MULTI-BATCH");
    Transaction saleTxn(undoRec.transactionId, "SALE", medId, batchDesc,
                        requestedQty, med.getUnitPrice(), undoRec.totalAmount, systemDate);
    transactionHistory.insertLast(saleTxn);

    std::cout << "[Success] Sale Completed! Total Billed: Rs. " << std::fixed 
              << std::setprecision(2) << undoRec.totalAmount 
              << " | Transaction ID: " << undoRec.transactionId << "\n\n";

    return true;
}

bool PharmacyInventory::undoLastSale() {
    if (undoStack.empty()) {
        std::cout << "[Warning] Undo stack is empty. No recent sale to reverse.\n";
        return false;
    }

    SaleUndoRecord topRec = undoStack.top();
    undoStack.pop();

    std::cout << "\n<<< Reversing Sale Transaction ID: " << topRec.transactionId 
              << " for Medicine ID: " << topRec.medicineId << " >>>\n";

    for (int i = 0; i < topRec.depletionCount; ++i) {
        int bIdx = findBatchIndexById(topRec.depletions[i].batchId);
        if (bIdx != -1) {
            batches[bIdx].addQuantity(topRec.depletions[i].quantity);
            std::cout << " -> Restored " << topRec.depletions[i].quantity 
                      << " unit(s) back into Batch [" << batches[bIdx].getBatchId() 
                      << "] (Current Batch Qty: " << batches[bIdx].getQuantity() << ")\n";
        } else {
            std::cout << "[Warning] Original Batch [" << topRec.depletions[i].batchId 
                      << "] not found during undo restoration.\n";
        }
    }

    // Record reversal consistently in linked-list audit trail
    Transaction undoTxn(nextTransactionId++, "UNDO_SALE", topRec.medicineId, 
                        "RESTORED", topRec.totalQuantity, 0.0, -topRec.totalAmount, systemDate);
    transactionHistory.insertLast(undoTxn);

    std::cout << "[Success] Undo complete. Reversal recorded under Transaction ID: " 
              << undoTxn.getTransactionId() << ".\n\n";
    return true;
}

bool PharmacyInventory::enqueueRestockRequest(const std::string& medId, int qty, 
                                            const std::string& supId, const std::string& notes) {
    if (findMedicineIndexById(medId) == -1) {
        std::cout << "[Error] Cannot queue restock. Medicine ID '" << medId << "' not found.\n";
        return false;
    }
    if (findSupplierIndexById(supId) == -1) {
        std::cout << "[Error] Cannot queue restock. Supplier ID '" << supId << "' not found.\n";
        return false;
    }
    if (qty <= 0) {
        std::cout << "[Error] Restock quantity must be positive (> 0).\n";
        return false;
    }

    RestockRequest req(nextRestockRequestId++, medId, qty, supId, notes);
    restockQueue.push(req);
    std::cout << "[Success] Restock Request #" << req.requestId 
              << " enqueued (FIFO order). Pending requests in queue: " 
              << restockQueue.size() << "\n";
    return true;
}

bool PharmacyInventory::processNextRestockRequest(const std::string& batchId, 
                                                 const std::string& expDate, 
                                                 double cost) {
    if (restockQueue.empty()) {
        std::cout << "[Warning] Restock queue is empty. No requests to process.\n";
        return false;
    }

    // Inspect front request without popping yet (Bug B fix: atomicity)
    const RestockRequest& req = restockQueue.front();

    // Validate incoming parameters
    if (batchId.empty() || batchId.find('|') != std::string::npos) {
        std::cout << "[Error] Invalid Batch ID. Request #" << req.requestId 
                  << " remains in queue.\n";
        return false;
    }
    if (findBatchIndexById(batchId) != -1) {
        std::cout << "[Error] Batch ID '" << batchId << "' already exists. Request #" 
                  << req.requestId << " remains in queue.\n";
        return false;
    }
    if (!DateUtils::isValidDate(expDate)) {
        std::cout << "[Error] Invalid expiry date '" << expDate 
                  << "'. Request #" << req.requestId << " remains in queue.\n";
        return false;
    }
    if (cost < 0.0) {
        std::cout << "[Error] Purchase cost cannot be negative. Request #" 
                  << req.requestId << " remains in queue.\n";
        return false;
    }
    if (batchCount >= MAX_BATCHES) {
        std::cout << "[Error] Batch storage capacity reached. Request #" 
                  << req.requestId << " remains in queue.\n";
        return false;
    }
    if (findMedicineIndexById(req.medicineId) == -1) {
        std::cout << "[Error] Associated Medicine ID '" << req.medicineId 
                  << "' no longer exists in catalog.\n";
        return false;
    }

    std::cout << "\n>>> Processing Next Restock Request #" << req.requestId << " (FIFO) <<<\n";
    std::cout << " Medicine ID:  " << req.medicineId << "\n";
    std::cout << " Ordered Qty:  " << req.quantity << "\n";
    std::cout << " Supplier ID:  " << req.supplierId << "\n";
    std::cout << " Notes:        " << req.notes << "\n";

    Batch newBatch(batchId, req.medicineId, expDate, req.quantity, cost);
    batches[batchCount++] = newBatch;

    Transaction purchaseTxn(nextTransactionId++, "PURCHASE", req.medicineId, 
                            batchId, req.quantity, cost, 
                            req.quantity * cost, systemDate);
    transactionHistory.insertLast(purchaseTxn);

    int finishedId = req.requestId;
    // Pop from queue ONLY after completely successful creation and logging
    restockQueue.pop();

    std::cout << "[Success] Restock request #" << finishedId 
              << " fulfilled with Batch '" << batchId 
              << "'. Remaining in queue: " << restockQueue.size() << "\n\n";
    return true;
}

void PharmacyInventory::displayRestockQueue() const {
    if (restockQueue.empty()) {
        std::cout << " (No pending restock requests in queue)\n";
        return;
    }

    std::cout << " --- Pending Restock Requests (FIFO Queue) ---\n";
    std::cout << " Total pending requests: " << restockQueue.size() << "\n";
    const RestockRequest& frontReq = restockQueue.front();
    std::cout << " Next in line for fulfillment (Front of Queue):\n";
    std::cout << "   Request ID:  #" << frontReq.requestId << "\n";
    std::cout << "   Medicine ID: " << frontReq.medicineId << "\n";
    std::cout << "   Quantity:    " << frontReq.quantity << "\n";
    std::cout << "   Supplier ID: " << frontReq.supplierId << "\n";
    std::cout << "   Notes:       " << frontReq.notes << "\n";
}

void PharmacyInventory::displaySortedInventoryByName() {
    if (medicineCount == 0) {
        std::cout << " (No medicines in catalog to sort)\n";
        return;
    }

    // Work on local clone array to preserve original catalog positions
    Medicine sortedList[MAX_MEDICINES];
    for (int i = 0; i < medicineCount; ++i) {
        sortedList[i] = medicines[i];
    }

    // Insertion Sort: Best O(N), Worst/Average O(N^2), Auxiliary Space O(N)
    for (int i = 1; i < medicineCount; ++i) {
        Medicine key = sortedList[i];
        int j = i - 1;
        while (j >= 0 && sortedList[j].getName() > key.getName()) {
            sortedList[j + 1] = sortedList[j];
            j = j - 1;
        }
        sortedList[j + 1] = key;
    }

    std::cout << "\n ================== AUDIT REPORT: MEDICINES (SORTED BY NAME - INSERTION SORT) ==================\n";
    std::cout << std::left 
              << std::setw(12) << "Med ID" 
              << std::setw(24) << "Name" 
              << std::setw(16) << "Barcode" 
              << std::setw(16) << "Category" 
              << "Unit Price\n";
    std::cout << "----------------------------------------------------------------------------------------------\n";
    for (int i = 0; i < medicineCount; ++i) {
        sortedList[i].display();
    }
    std::cout << "----------------------------------------------------------------------------------------------\n";
}

void PharmacyInventory::displaySortedInventoryById() {
    if (medicineCount == 0) {
        std::cout << " (No medicines in catalog to sort)\n";
        return;
    }

    Medicine sortedList[MAX_MEDICINES];
    for (int i = 0; i < medicineCount; ++i) {
        sortedList[i] = medicines[i];
    }

    // Insertion Sort by Medicine ID
    for (int i = 1; i < medicineCount; ++i) {
        Medicine key = sortedList[i];
        int j = i - 1;
        while (j >= 0 && sortedList[j].getMedicineId() > key.getMedicineId()) {
            sortedList[j + 1] = sortedList[j];
            j = j - 1;
        }
        sortedList[j + 1] = key;
    }

    std::cout << "\n =================== AUDIT REPORT: MEDICINES (SORTED BY ID - INSERTION SORT) ===================\n";
    std::cout << std::left 
              << std::setw(12) << "Med ID" 
              << std::setw(24) << "Name" 
              << std::setw(16) << "Barcode" 
              << std::setw(16) << "Category" 
              << "Unit Price\n";
    std::cout << "----------------------------------------------------------------------------------------------\n";
    for (int i = 0; i < medicineCount; ++i) {
        sortedList[i].display();
    }
    std::cout << "----------------------------------------------------------------------------------------------\n";
}

void PharmacyInventory::displayTransactionHistory() const {
    transactionHistory.displayAll();
}

bool PharmacyInventory::saveToFile(const std::string& medFile,
                                   const std::string& batchFile,
                                   const std::string& supFile,
                                   const std::string& txnFile,
                                   const std::string& restockFile,
                                   const std::string& undoFile) const {
    // 1. Save Medicines
    std::ofstream mOut(medFile);
    if (!mOut.is_open()) {
        std::cout << "[Error] Failed to open file for saving: " << medFile << "\n";
        return false;
    }
    for (int i = 0; i < medicineCount; ++i) {
        mOut << medicines[i].serialize() << "\n";
    }
    mOut.close();

    // 2. Save Batches
    std::ofstream bOut(batchFile);
    if (!bOut.is_open()) {
        std::cout << "[Error] Failed to open file for saving: " << batchFile << "\n";
        return false;
    }
    for (int i = 0; i < batchCount; ++i) {
        bOut << batches[i].serialize() << "\n";
    }
    bOut.close();

    // 3. Save Suppliers
    std::ofstream sOut(supFile);
    if (!sOut.is_open()) {
        std::cout << "[Error] Failed to open file for saving: " << supFile << "\n";
        return false;
    }
    for (int i = 0; i < supplierCount; ++i) {
        sOut << suppliers[i].serialize() << "\n";
    }
    sOut.close();

    // 4. Save Transactions
    std::ofstream tOut(txnFile);
    if (!tOut.is_open()) {
        std::cout << "[Error] Failed to open file for saving: " << txnFile << "\n";
        return false;
    }
    const TransactionNode* curr = transactionHistory.getHead();
    while (curr != nullptr) {
        tOut << curr->data.serialize() << "\n";
        curr = curr->next;
    }
    tOut.close();

    // 5. Save Restock Queue
    std::ofstream rOut(restockFile);
    if (rOut.is_open()) {
        std::queue<RestockRequest> tempQ = restockQueue;
        while (!tempQ.empty()) {
            const RestockRequest& r = tempQ.front();
            rOut << r.requestId << "|" << r.medicineId << "|" 
                 << r.quantity << "|" << r.supplierId << "|" << r.notes << "\n";
            tempQ.pop();
        }
        rOut.close();
    }

    // 6. Save Undo Stack (Reversible LIFO State)
    std::ofstream uOut(undoFile);
    if (uOut.is_open()) {
        // Copy stack to temporary array to write bottom-to-top so reloading reproduces identical stack
        std::stack<SaleUndoRecord> tempStack = undoStack;
        SaleUndoRecord records[MAX_BATCHES];
        int count = 0;
        while (!tempStack.empty() && count < MAX_BATCHES) {
            records[count++] = tempStack.top();
            tempStack.pop();
        }
        // Write in reverse order (bottom first)
        for (int i = count - 1; i >= 0; --i) {
            uOut << records[i].transactionId << "|" 
                 << records[i].medicineId << "|" 
                 << records[i].totalQuantity << "|" 
                 << records[i].totalAmount << "|" 
                 << records[i].date << "|" 
                 << records[i].depletionCount;
            for (int d = 0; d < records[i].depletionCount; ++d) {
                uOut << "|" << records[i].depletions[d].batchId 
                     << ":" << records[i].depletions[d].quantity;
            }
            uOut << "\n";
        }
        uOut.close();
    }

    std::cout << "[System] All records and data structures successfully persisted to storage.\n";
    return true;
}

bool PharmacyInventory::loadFromFile(const std::string& medFile,
                                   const std::string& batchFile,
                                   const std::string& supFile,
                                   const std::string& txnFile,
                                   const std::string& restockFile,
                                   const std::string& undoFile) {
    // Reset all internal structures to avoid duplicate accumulation on repeated calls
    medicineCount = 0;
    batchCount = 0;
    supplierCount = 0;
    transactionHistory.clear();
    while (!restockQueue.empty()) restockQueue.pop();
    while (!undoStack.empty()) undoStack.pop();

    int maxTxnId = 1000;
    int maxReqId = 0;

    // 1. Load Medicines
    std::ifstream mIn(medFile);
    if (mIn.is_open()) {
        std::string line;
        int lineNum = 0;
        while (std::getline(mIn, line)) {
            lineNum++;
            if (line.empty()) continue;
            Medicine med = Medicine::deserialize(line);
            if (med.isValid()) {
                if (medicineCount < MAX_MEDICINES && findMedicineIndexById(med.getMedicineId()) == -1) {
                    medicines[medicineCount++] = med;
                }
            } else {
                std::cout << "[Warning] Skipped malformed record in " << medFile 
                          << " at line " << lineNum << "\n";
            }
        }
        mIn.close();
    }

    // 2. Load Batches
    std::ifstream bIn(batchFile);
    if (bIn.is_open()) {
        std::string line;
        int lineNum = 0;
        while (std::getline(bIn, line)) {
            lineNum++;
            if (line.empty()) continue;
            Batch b = Batch::deserialize(line);
            if (b.isValid()) {
                if (batchCount < MAX_BATCHES && findBatchIndexById(b.getBatchId()) == -1) {
                    batches[batchCount++] = b;
                }
            } else {
                std::cout << "[Warning] Skipped malformed record in " << batchFile 
                          << " at line " << lineNum << "\n";
            }
        }
        bIn.close();
    }

    // 3. Load Suppliers
    std::ifstream sIn(supFile);
    if (sIn.is_open()) {
        std::string line;
        int lineNum = 0;
        while (std::getline(sIn, line)) {
            lineNum++;
            if (line.empty()) continue;
            Supplier sup = Supplier::deserialize(line);
            if (sup.isValid()) {
                if (supplierCount < MAX_SUPPLIERS && findSupplierIndexById(sup.getSupplierId()) == -1) {
                    suppliers[supplierCount++] = sup;
                }
            } else {
                std::cout << "[Warning] Skipped malformed record in " << supFile 
                          << " at line " << lineNum << "\n";
            }
        }
        sIn.close();
    }

    // 4. Load Transactions
    std::ifstream tIn(txnFile);
    if (tIn.is_open()) {
        std::string line;
        int lineNum = 0;
        while (std::getline(tIn, line)) {
            lineNum++;
            if (line.empty()) continue;
            Transaction txn = Transaction::deserialize(line);
            if (txn.isValid()) {
                transactionHistory.insertLast(txn);
                if (txn.getTransactionId() > maxTxnId) {
                    maxTxnId = txn.getTransactionId();
                }
            } else {
                std::cout << "[Warning] Skipped malformed record in " << txnFile 
                          << " at line " << lineNum << "\n";
            }
        }
        tIn.close();
    }

    // 5. Load Restock Requests
    std::ifstream rIn(restockFile);
    if (rIn.is_open()) {
        std::string line;
        while (std::getline(rIn, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string rIdStr, mId, qtyStr, sId, notes;
            if (std::getline(iss, rIdStr, '|') &&
                std::getline(iss, mId, '|') &&
                std::getline(iss, qtyStr, '|') &&
                std::getline(iss, sId, '|') &&
                std::getline(iss, notes, '|')) {
                try {
                    int rId = std::stoi(rIdStr);
                    int qty = std::stoi(qtyStr);
                    if (rId > maxReqId) maxReqId = rId;
                    restockQueue.push(RestockRequest(rId, mId, qty, sId, notes));
                } catch (...) {}
            }
        }
        rIn.close();
    }

    // 6. Load Undo Stack
    std::ifstream uIn(undoFile);
    if (uIn.is_open()) {
        std::string line;
        while (std::getline(uIn, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string tIdStr, mId, tQtyStr, tAmtStr, dStr, dCountStr;
            if (std::getline(iss, tIdStr, '|') &&
                std::getline(iss, mId, '|') &&
                std::getline(iss, tQtyStr, '|') &&
                std::getline(iss, tAmtStr, '|') &&
                std::getline(iss, dStr, '|') &&
                std::getline(iss, dCountStr, '|')) {
                try {
                    SaleUndoRecord rec;
                    rec.transactionId = std::stoi(tIdStr);
                    rec.medicineId = mId;
                    rec.totalQuantity = std::stoi(tQtyStr);
                    rec.totalAmount = std::stod(tAmtStr);
                    rec.date = dStr;
                    rec.depletionCount = std::stoi(dCountStr);
                    for (int i = 0; i < rec.depletionCount; ++i) {
                        std::string pairStr;
                        if (std::getline(iss, pairStr, '|')) {
                            size_t colon = pairStr.find(':');
                            if (colon != std::string::npos) {
                                rec.depletions[i].batchId = pairStr.substr(0, colon);
                                rec.depletions[i].quantity = std::stoi(pairStr.substr(colon + 1));
                            }
                        }
                    }
                    undoStack.push(rec);
                } catch (...) {}
            }
        }
        uIn.close();
    }

    nextTransactionId = maxTxnId + 1;
    nextRestockRequestId = maxReqId + 1;

    return true;
}
