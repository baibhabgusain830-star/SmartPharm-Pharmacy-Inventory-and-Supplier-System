#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class Batch {
public:
    int batchId;
    int medId;
    int quantity;
    int expiryDate;

    Batch(int bId = 0, int mId = 0, int q = 0, int exp = 0) {
        batchId = bId;
        medId = mId;
        quantity = q;
        expiryDate = exp;
    }

    void display() {
        cout << "BatchID: " << batchId << " | MedID: " << medId
             << " | Qty: " << quantity << " | Expiry: " << expiryDate << endl;
    }
};

bool isValidDate(int date) {
    int year = date / 10000;
    int month = (date / 100) % 100;
    int day = date % 100;
    if (year < 2020 || year > 2050) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    return true;
}

class MinHeap {
public:
    static const int MAX_BATCHES = 3000;
    Batch heap[MAX_BATCHES];
    int size;

    void swapBatch(Batch &a, Batch &b) {
        Batch temp = a;
        a = b;
        b = temp;
    }

    void rebuildHeap() {
        int oldSize = size;
        Batch* temp = new Batch[oldSize];
        for (int i = 0; i < oldSize; i++) {
            temp[i] = heap[i];
        }
        size = 0;
        for (int i = 0; i < oldSize; i++) {
            if (temp[i].quantity > 0) {
                insert(temp[i]);
            }
        }
        delete[] temp;
    }

    MinHeap() {
        size = 0;
    }

    void insert(Batch b) {
        if (size >= MAX_BATCHES) return;
        int i = size;
        heap[size++] = b;

        while (i != 0 && heap[(i - 1) / 2].expiryDate > heap[i].expiryDate) {
            swapBatch(heap[i], heap[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    Batch getNearestExpiryBatch() {
        if (size <= 0) return Batch();
        return heap[0];
    }

    Batch extractMin() {
        if (size <= 0) return Batch();
        if (size == 1) return heap[--size];

        Batch root = heap[0];
        heap[0] = heap[--size];

        int i = 0;
        while (2 * i + 1 < size) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < size && heap[left].expiryDate < heap[smallest].expiryDate)
                smallest = left;
            if (right < size && heap[right].expiryDate < heap[smallest].expiryDate)
                smallest = right;

            if (smallest != i) {
                swapBatch(heap[i], heap[smallest]);
                i = smallest;
            } else {
                break;
            }
        }
        return root;
    }

    void displayBatchesForMed(int medId, int currentDate) {
        int count = 0;
        int validQty = 0;
        int nearestExp = 99999999;
        int nearestBatchId = -1;

        cout << "\n--- Batches for Medicine ID " << medId << " ---" << endl;
        for (int i = 0; i < size; i++) {
            if (heap[i].medId == medId && heap[i].quantity > 0) {
                count++;
                cout << "   Batch ID: " << heap[i].batchId
                     << " | Qty: " << heap[i].quantity
                     << " | Expiry: " << heap[i].expiryDate;
                if (heap[i].expiryDate < currentDate) {
                    cout << " [EXPIRED - DO NOT SELL]" << endl;
                } else {
                    cout << " [VALID]" << endl;
                    validQty += heap[i].quantity;
                    if (heap[i].expiryDate < nearestExp) {
                        nearestExp = heap[i].expiryDate;
                        nearestBatchId = heap[i].batchId;
                    }
                }
            }
        }
        if (count == 0) {
            cout << "   No active batches found for this medicine!" << endl;
        } else {
            cout << "   Total Batches: " << count << " | Sellable Non-Expired Stock: " << validQty << endl;
            if (nearestBatchId != -1) {
                cout << "   Next Batch to Dispense (FEFO): Batch ID " << nearestBatchId
                     << " (Expires: " << nearestExp << ")" << endl;
            }
        }
    }

    int getValidBatchStock(int medId, int currentDate) {
        int total = 0;
        for (int i = 0; i < size; i++) {
            if (heap[i].medId == medId && heap[i].expiryDate >= currentDate) {
                total += heap[i].quantity;
            }
        }
        return total;
    }

    bool sellFromFEFO(int medId, int qtyNeeded, int currentDate) {
        if (getValidBatchStock(medId, currentDate) < qtyNeeded) {
            return false;
        }

        int remaining = qtyNeeded;
        while (remaining > 0) {
            int bestIndex = -1;
            for (int i = 0; i < size; i++) {
                if (heap[i].medId == medId && heap[i].quantity > 0 && heap[i].expiryDate >= currentDate) {
                    if (bestIndex == -1 || heap[i].expiryDate < heap[bestIndex].expiryDate) {
                        bestIndex = i;
                    }
                }
            }

            if (bestIndex == -1) break;

            int take = heap[bestIndex].quantity;
            if (take > remaining) take = remaining;

            heap[bestIndex].quantity -= take;
            remaining -= take;

            cout << "   -> Dispensed " << take << " units from Batch ID " << heap[bestIndex].batchId
                 << " (Expiry: " << heap[bestIndex].expiryDate << ")" << endl;
        }

        rebuildHeap();
        return true;
    }

    void restoreBatchStock(int medId, int qty) {
        for (int i = 0; i < size; i++) {
            if (heap[i].medId == medId) {
                heap[i].quantity += qty;
                return;
            }
        }
        insert(Batch(9999, medId, qty, 20271231));
    }

    void recallBatch(int batchId) {
        for (int i = 0; i < size; i++) {
            if (heap[i].batchId == batchId) {
                heap[i].quantity = 0;
                rebuildHeap();
                cout << "Batch " << batchId << " recalled and removed from stock." << endl;
                return;
            }
        }
        cout << "Batch ID not found." << endl;
    }

    void scanExpiryAlerts(int currentDate) {
        cout << "\n--- Expiry Alert Scan (Current Date: " << currentDate << ") ---" << endl;
        int found = 0;
        for (int i = 0; i < size; i++) {
            if (heap[i].expiryDate < currentDate) {
                cout << "[EXPIRED] ";
                heap[i].display();
                found++;
            } else if (heap[i].expiryDate - currentDate <= 300) {
                cout << "[NEAR EXPIRY] ";
                heap[i].display();
                found++;
            }
        }
        if (found == 0) cout << "All batches are safe and within valid dates." << endl;
        else cout << "Total flagged batches: " << found << endl;
    }

    void displayAll() {
        if (size == 0) {
            cout << "No batches in inventory." << endl;
            return;
        }
        for (int i = 0; i < size; i++) {
            heap[i].display();
        }
    }

    void saveBatchesCSV() {
        ofstream fout("batches.csv");
        for (int i = 0; i < size; i++) {
            fout << heap[i].batchId << " " << heap[i].medId << " "
                 << heap[i].quantity << " " << heap[i].expiryDate << endl;
        }
        fout.close();
    }

    void loadBatchesCSV() {
        ifstream fin("batches.csv");
        if (!fin) return;
        size = 0;
        int bId, mId, q, exp;
        while (fin >> bId >> mId >> q >> exp) {
            insert(Batch(bId, mId, q, exp));
        }
        fin.close();
    }
};

class Medicine {
public:
    int id;
    string barcode;
    string name;
    string category;
    double price;
    int stock;
    string medType;
    int extraVal;

    Medicine(int i = 0, string b = "", string n = "", string c = "", double p = 0.0, int s = 0) {
        id = i;
        barcode = b;
        name = n;
        category = c;
        price = p;
        stock = s;
        medType = "GENERIC";
        extraVal = 0;
    }

    virtual ~Medicine() {}

    virtual void display() {
        cout << "[" << id << " | Barcode: " << barcode << "] " << name
             << " | Cat: " << category << " | Rs." << price << " | Total Stock: " << stock;
    }
};

class Tablet : public Medicine {
public:
    int mg;
    Tablet(int i, string b, string n, string c, double p, int s, int m)
        : Medicine(i, b, n, c, p, s) {
        mg = m;
        medType = "TABLET";
        extraVal = m;
    }

    void display() override {
        Medicine::display();
        cout << " | Type: Tablet (" << mg << "mg)" << endl;
    }
};

class Syrup : public Medicine {
public:
    int ml;
    Syrup(int i, string b, string n, string c, double p, int s, int v)
        : Medicine(i, b, n, c, p, s) {
        ml = v;
        medType = "SYRUP";
        extraVal = v;
    }

    void display() override {
        Medicine::display();
        cout << " | Type: Syrup (" << ml << "ml)" << endl;
    }
};

class Injection : public Medicine {
public:
    Injection(int i, string b, string n, string c, double p, int s)
        : Medicine(i, b, n, c, p, s) {
        medType = "INJECTION";
        extraVal = 0;
    }

    void display() override {
        Medicine::display();
        cout << " | Type: Injection" << endl;
    }
};

struct HashNode {
    Medicine* med;
    HashNode* next;
    HashNode(Medicine* m) {
        med = m;
        next = nullptr;
    }
};

class HashTable {
public:
    static const int SIZE = 1009;
    HashNode* table[SIZE];

    int hashFunction(int key) {
        if (key < 0) key = -key;
        return key % SIZE;
    }

    HashTable() {
        for (int i = 0; i < SIZE; i++) table[i] = nullptr;
    }

    void insert(Medicine* m) {
        int index = hashFunction(m->id);
        HashNode* newNode = new HashNode(m);
        newNode->next = table[index];
        table[index] = newNode;
    }

    Medicine* searchById(int id) {
        int index = hashFunction(id);
        HashNode* temp = table[index];
        while (temp != nullptr) {
            if (temp->med->id == id) return temp->med;
            temp = temp->next;
        }
        return nullptr;
    }

    Medicine* searchByBarcode(string scanCode) {
        for (int i = 0; i < SIZE; i++) {
            HashNode* temp = table[i];
            while (temp != nullptr) {
                if (temp->med->barcode == scanCode) return temp->med;
                temp = temp->next;
            }
        }
        return nullptr;
    }

    void displayAll() {
        int count = 0;
        for (int i = 0; i < SIZE; i++) {
            HashNode* temp = table[i];
            while (temp != nullptr) {
                temp->med->display();
                count++;
                temp = temp->next;
            }
        }
        if (count == 0) cout << "No medicines in inventory." << endl;
        else cout << "Total Medicines Displayed: " << count << endl;
    }

    void saveMedicinesCSV() {
        ofstream fout("medicines.csv");
        for (int i = 0; i < SIZE; i++) {
            HashNode* temp = table[i];
            while (temp != nullptr) {
                Medicine* m = temp->med;
                fout << m->medType << " " << m->id << " " << m->barcode << " "
                     << m->name << " " << m->category << " " << m->price << " "
                     << m->stock << " " << m->extraVal << endl;
                temp = temp->next;
            }
        }
        fout.close();
    }
};

struct BSTNode {
    Medicine* med;
    BSTNode* left;
    BSTNode* right;
    BSTNode(Medicine* m) {
        med = m;
        left = nullptr;
        right = nullptr;
    }
};

class CategoryBST {
public:
    BSTNode* root;

    CategoryBST() {
        root = nullptr;
    }

    BSTNode* insertRec(BSTNode* node, Medicine* m) {
        if (node == nullptr) return new BSTNode(m);
        if (m->category < node->med->category)
            node->left = insertRec(node->left, m);
        else
            node->right = insertRec(node->right, m);
        return node;
    }

    void insert(Medicine* m) {
        root = insertRec(root, m);
    }

    void inOrder(BSTNode* node) {
        if (node == nullptr) return;
        inOrder(node->left);
        node->med->display();
        inOrder(node->right);
    }

    void displayOrderedByCategory() {
        if (root == nullptr) {
            cout << "No medicines in BST." << endl;
            return;
        }
        cout << "\n--- Medicines Ordered by Category (BST In-Order) ---" << endl;
        inOrder(root);
    }
};

int binarySearchMedicine(Medicine* arr[], int size, int targetId) {
    int low = 0, high = size - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (arr[mid]->id == targetId) return mid;
        else if (arr[mid]->id < targetId) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

class Supplier {
public:
    int supplierId;
    string name;
    string contact;

    Supplier(int id = 0, string n = "", string c = "") {
        supplierId = id;
        name = n;
        contact = c;
    }

    void display() {
        cout << "Supplier [" << supplierId << "]: " << name << " (" << contact << ")" << endl;
    }
};

class CircularQueue {
public:
    static const int CAPACITY = 100;
    Supplier orders[CAPACITY];
    int front, rear, count;

    CircularQueue() {
        front = 0;
        rear = -1;
        count = 0;
    }

    void enqueue(Supplier s) {
        if (count == CAPACITY) {
            cout << "Order Queue is Full!" << endl;
            return;
        }
        rear = (rear + 1) % CAPACITY;
        orders[rear] = s;
        count++;
    }

    Supplier dequeue() {
        if (count == 0) {
            cout << "Order Queue is Empty!" << endl;
            return Supplier();
        }
        Supplier s = orders[front];
        front = (front + 1) % CAPACITY;
        count--;
        return s;
    }

    void displayQueue() {
        if (count == 0) {
            cout << "Supplier queue is empty." << endl;
            return;
        }
        for (int i = 0; i < count; i++) {
            int index = (front + i) % CAPACITY;
            orders[index].display();
        }
    }

    void saveSuppliersCSV() {
        ofstream fout("suppliers.csv");
        for (int i = 0; i < count; i++) {
            int index = (front + i) % CAPACITY;
            fout << orders[index].supplierId << " " << orders[index].name << " "
                 << orders[index].contact << endl;
        }
        fout.close();
    }

    void loadSuppliersCSV() {
        ifstream fin("suppliers.csv");
        if (!fin) return;
        front = 0; rear = -1; count = 0;
        int id; string n, c;
        while (fin >> id >> n >> c) {
            enqueue(Supplier(id, n, c));
        }
        fin.close();
    }
};

struct EdgeNode {
    int medId;
    EdgeNode* next;
    EdgeNode(int m) {
        medId = m;
        next = nullptr;
    }
};

class SupplierGraph {
public:
    static const int MAX_SUPPLIERS = 100;
    int supplierIds[MAX_SUPPLIERS];
    EdgeNode* adjList[MAX_SUPPLIERS];
    int vertexCount;

    SupplierGraph() {
        vertexCount = 0;
        for (int i = 0; i < MAX_SUPPLIERS; i++) adjList[i] = nullptr;
    }

    int findOrAddSupplierVertex(int supId) {
        for (int i = 0; i < vertexCount; i++) {
            if (supplierIds[i] == supId) return i;
        }
        if (vertexCount < MAX_SUPPLIERS) {
            supplierIds[vertexCount] = supId;
            adjList[vertexCount] = nullptr;
            return vertexCount++;
        }
        return -1;
    }

    void addLink(int supId, int medId) {
        int idx = findOrAddSupplierVertex(supId);
        if (idx == -1) return;
        EdgeNode* newEdge = new EdgeNode(medId);
        newEdge->next = adjList[idx];
        adjList[idx] = newEdge;
    }

    void dfsHelper(int idx, bool visited[]) {
        visited[idx] = true;
        cout << "Supplier ID " << supplierIds[idx] << " supplies Medicine IDs: ";
        EdgeNode* temp = adjList[idx];
        while (temp != nullptr) {
            cout << temp->medId << " ";
            temp = temp->next;
        }
        cout << endl;
    }

    void runDFS() {
        if (vertexCount == 0) {
            cout << "No supplier-medicine links in Graph." << endl;
            return;
        }
        bool visited[MAX_SUPPLIERS];
        for (int i = 0; i < MAX_SUPPLIERS; i++) visited[i] = false;

        cout << "\n--- Supplier-Medicine Graph (DFS Traversal) ---" << endl;
        for (int i = 0; i < vertexCount; i++) {
            if (!visited[i]) {
                dfsHelper(i, visited);
            }
        }
    }
};

class Transaction {
public:
    int transId;
    string type;
    int medId;
    int quantity;
    double amount;

    Transaction(int tId = 0, string t = "", int mId = 0, double amt = 0.0, int q = 1) {
        transId = tId;
        type = t;
        medId = mId;
        amount = amt;
        quantity = q;
    }

    void display() {
        cout << "TransID: " << transId << " | " << type
             << " | MedID: " << medId << " | Qty: " << quantity
             << " | Amount: Rs." << amount << endl;
    }
};

struct StackNode {
    Transaction data;
    StackNode* next;
    StackNode(Transaction t) {
        data = t;
        next = nullptr;
    }
};

class DualStack {
public:
    StackNode* undoTop;
    StackNode* redoTop;

    void push(StackNode* &top, Transaction t) {
        StackNode* newNode = new StackNode(t);
        newNode->next = top;
        top = newNode;
    }

    Transaction pop(StackNode* &top) {
        if (top == nullptr) return Transaction();
        StackNode* temp = top;
        Transaction t = temp->data;
        top = top->next;
        delete temp;
        return t;
    }

    DualStack() {
        undoTop = nullptr;
        redoTop = nullptr;
    }

    void recordAction(Transaction t) {
        push(undoTop, t);
    }

    Transaction undo() {
        if (undoTop == nullptr) {
            cout << "Nothing to undo!" << endl;
            return Transaction(0, "", 0, 0.0, 0);
        }
        Transaction t = pop(undoTop);
        push(redoTop, t);
        cout << "Undid Transaction ID: " << t.transId << endl;
        return t;
    }

    Transaction redo() {
        if (redoTop == nullptr) {
            cout << "Nothing to redo!" << endl;
            return Transaction(0, "", 0, 0.0, 0);
        }
        Transaction t = pop(redoTop);
        push(undoTop, t);
        cout << "Redid Transaction ID: " << t.transId << endl;
        return t;
    }
};

struct DLLNode {
    Transaction data;
    DLLNode* prev;
    DLLNode* next;
    DLLNode(Transaction t) {
        data = t;
        prev = nullptr;
        next = nullptr;
    }
};

class TransactionHistory {
public:
    DLLNode* head;
    DLLNode* tail;

    TransactionHistory() {
        head = nullptr;
        tail = nullptr;
    }

    void append(Transaction t) {
        DLLNode* newNode = new DLLNode(t);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    void displayAll() {
        if (head == nullptr) {
            cout << "No transactions recorded." << endl;
            return;
        }
        DLLNode* temp = head;
        while (temp != nullptr) {
            temp->data.display();
            temp = temp->next;
        }
    }

    void quickSortArray(Transaction arr[], int low, int high) {
        if (low < high) {
            double pivot = arr[high].amount;
            int i = low - 1;
            for (int j = low; j < high; j++) {
                if (arr[j].amount > pivot) {
                    i++;
                    Transaction temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
            }
            Transaction temp = arr[i + 1];
            arr[i + 1] = arr[high];
            arr[high] = temp;
            int pi = i + 1;

            quickSortArray(arr, low, pi - 1);
            quickSortArray(arr, pi + 1, high);
        }
    }

    void displayProfitRankingQuickSort() {
        Transaction* arr = new Transaction[2000];
        int count = 0;
        DLLNode* temp = head;
        while (temp != nullptr && count < 2000) {
            arr[count++] = temp->data;
            temp = temp->next;
        }
        if (count == 0) {
            cout << "No transactions to rank." << endl;
            delete[] arr;
            return;
        }
        quickSortArray(arr, 0, count - 1);
        cout << "\n--- Sales & Profit Ranking (Quick Sort Highest to Lowest) ---" << endl;
        for (int i = 0; i < count; i++) {
            arr[i].display();
        }
        delete[] arr;
    }

    void saveTransactionsCSV() {
        ofstream fout("transactions.csv");
        DLLNode* temp = head;
        while (temp != nullptr) {
            fout << temp->data.transId << " " << temp->data.type << " "
                 << temp->data.medId << " " << temp->data.amount << " "
                 << temp->data.quantity << endl;
            temp = temp->next;
        }
        fout.close();
    }

    void loadTransactionsCSV() {
        ifstream fin("transactions.csv");
        if (!fin) return;
        head = tail = nullptr;
        int tId, mId, q;
        string t;
        double amt;
        while (fin >> tId >> t >> mId >> amt >> q) {
            append(Transaction(tId, t, mId, amt, q));
        }
        fin.close();
    }
};

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
                cout << " | Available Non-Expired FEFO Stock: " << availFEFO << endl;

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