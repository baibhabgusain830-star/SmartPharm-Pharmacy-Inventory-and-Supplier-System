#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

// =============================================
// OOP BASE CLASS: Medicine
// Encapsulation = private data, public getters
// =============================================
class Medicine {
private:
    string id, name, category;
    double price;
public:
    Medicine() { id=""; name=""; category=""; price=0; }
    Medicine(string i, string n, string c, double p) { id=i; name=n; category=c; price=p; }
    virtual ~Medicine() {}

    string getId()       { return id;       }
    string getName()     { return name;     }
    string getCategory() { return category; }
    double getPrice()    { return price;    }

    virtual string getType() { return "General"; }  // Virtual function -> Polymorphism

    void display() {
        cout << left << setw(10) << id << setw(22) << name << setw(12) << getType() << "  Rs." << price << "\n";
    }
};

// =============================================
// OOP DERIVED CLASSES: Inheritance + Polymorphism
// Tablet, Syrup, Injection extend Medicine
// =============================================
class Tablet : public Medicine {
public:
    Tablet() {}
    Tablet(string i, string n, string c, double p) : Medicine(i, n, c, p) {}
    string getType() { return "Tablet"; }    // Function Overriding
};
class Syrup : public Medicine {
public:
    Syrup() {}
    Syrup(string i, string n, string c, double p) : Medicine(i, n, c, p) {}
    string getType() { return "Syrup"; }
};
class Injection : public Medicine {
public:
    Injection() {}
    Injection(string i, string n, string c, double p) : Medicine(i, n, c, p) {}
    string getType() { return "Injection"; }
};

// =============================================
// OOP CLASS: Batch  (Encapsulation)
// =============================================
class Batch {
private:
    string batchId, medId, expiry, supplierId;
    int quantity;
public:
    Batch() { batchId=""; medId=""; expiry=""; supplierId=""; quantity=0; }
    Batch(string b, string m, string e, string s, int q) { batchId=b; medId=m; expiry=e; supplierId=s; quantity=q; }
    string getBatchId()    { return batchId;    }
    string getMedId()      { return medId;      }
    string getExpiry()     { return expiry;     }
    string getSupplierId() { return supplierId; }
    int    getQuantity()   { return quantity;   }
    void addQuantity(int q)    { quantity += q; }   // addQuantity()
    void removeQuantity(int q) { quantity -= q; }   // removeQuantity()
};

// =============================================
// OOP CLASS: Supplier  (Encapsulation)
// =============================================
class Supplier {
private:
    string id, name, phone;
public:
    Supplier() { id=""; name=""; phone=""; }
    Supplier(string i, string n, string p) { id=i; name=n; phone=p; }
    string getId()    { return id;    }
    string getName()  { return name;  }
    string getPhone() { return phone; }
    void display() { cout << left << setw(12) << id << setw(24) << name << phone << "\n"; }
};

// =============================================
// DSA: STACK (Array-based) -> Undo Last Sale
// LIFO: Last In First Out
// =============================================
struct UndoItem { string batchId; int qty; };

class UndoStack {
private:
    UndoItem data[100]; // Array = underlying storage
    int top;            // top = -1 means empty
    string medId;
    int totalQty;
    double totalAmount;
    bool hasData;
public:
    UndoStack() { top=-1; hasData=false; totalQty=0; totalAmount=0; }

    void setup(string m, int q, double a) { medId=m; totalQty=q; totalAmount=a; top=-1; hasData=true; }

    void push(string b, int q) {  // PUSH: add on top
        top++;
        data[top].batchId = b;
        data[top].qty = q;
    }
    bool   isEmpty()         { return !hasData;    }
    void   reset()           { hasData=false; top=-1; }
    string getMedId()        { return medId;       }
    int    getTotalQty()     { return totalQty;    }
    double getTotalAmt()     { return totalAmount; }
    int    getCount()        { return top + 1;     }
    UndoItem getItem(int i)  { return data[i];     }
};

// =============================================
// DSA: CIRCULAR QUEUE (Array-based) -> Restock
// FIFO: First In First Out
// =============================================
struct Request { int id; string medId, supplierId; int qty; };

class RestockQueue {
private:
    Request data[30];  // Fixed-size circular array
    int front, rear, count, nextId;
public:
    RestockQueue() { front=0; rear=0; count=0; nextId=1; }
    bool isFull()   { return count >= 30; }
    bool isEmpty()  { return count == 0;  }
    int  getCount() { return count;       }

    void enqueue(string mId, string sId, int qty) {  // Add at REAR
        data[rear].id = nextId++;
        data[rear].medId = mId;
        data[rear].supplierId = sId;
        data[rear].qty = qty;
        rear = (rear + 1) % 30;  // Circular: wrap to 0 after 29
        count++;
    }
    Request getFront() { return data[front]; }

    void dequeue() {                   // Remove from FRONT
        front = (front + 1) % 30;
        count--;
    }
};

// =============================================
// DSA: LINKED LIST (Singly) -> Transaction Log
// Nodes connected by pointers
// =============================================
struct TxnNode {
    int id, qty;
    double total;
    string type, medId, batchId, date;
    TxnNode* next;  // Pointer to next node
};

class TransactionList {
private:
    TxnNode* head;  // head = NULL means empty list
    int nextId;
public:
    TransactionList() { head=NULL; nextId=1001; }

    ~TransactionList() {           // Destructor: free all nodes
        TxnNode* curr = head;
        while (curr != NULL) {
            TxnNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    void add(string type, string mId, string bId, int qty, double total, string date) {
        TxnNode* node = new TxnNode();  // Create node on heap (dynamic memory)
        node->id=nextId++; node->type=type; node->medId=mId;
        node->batchId=bId; node->qty=qty; node->total=total; node->date=date;
        node->next = NULL;
        if (head == NULL) { head = node; return; }  // First node = head
        TxnNode* curr = head;
        while (curr->next != NULL) curr = curr->next;  // Go to last node
        curr->next = node;                             // Attach at end
    }

    void display() {
        if (head == NULL) { cout << "No transactions.\n"; return; }
        cout << "\n-- Transaction History --\n";
        TxnNode* curr = head;
        while (curr != NULL) {
            cout << curr->id << "  " << left << setw(10) << curr->type
                 << "  Med:" << setw(8) << curr->medId
                 << "  Qty:" << curr->qty << "  Rs." << curr->total << "\n";
            curr = curr->next;
        }
    }
};

// =============================================
// OOP: PharmacyInventory  (Composition)
// Has-a: Medicine, Batch, Supplier, Stack, Queue, LinkedList
// Abstraction: complex logic hidden inside methods
// =============================================
class PharmacyInventory {
private:
    Medicine*       meds[50];    // Pointer array -> enables Polymorphism
    int             medCount;
    Batch           batches[100];
    int             batchCount;
    Supplier        suppliers[30];
    int             supplierCount;
    UndoStack       undoStack;    // Composition: Stack
    RestockQueue    restockQueue; // Composition: Queue
    TransactionList txnList;      // Composition: Linked List
    string today;

    int findMed(string q) {  // Linear Search
        for (int i = 0; i < medCount; i++)
            if (meds[i]->getId() == q || meds[i]->getName() == q) return i;
        return -1;
    }
    int findSupplier(string id) {
        for (int i = 0; i < supplierCount; i++) if (suppliers[i].getId() == id) return i;
        return -1;
    }
    int findBatch(string id) {
        for (int i = 0; i < batchCount; i++) if (batches[i].getBatchId() == id) return i;
        return -1;
    }
    int getStock(string mId) {
        int total = 0;
        for (int i = 0; i < batchCount; i++)
            if (batches[i].getMedId() == mId && batches[i].getExpiry() > today)
                total += batches[i].getQuantity();
        return total;
    }

public:
    PharmacyInventory() {
        medCount=0; batchCount=0; supplierCount=0; today="2026-10-15";
        for (int i = 0; i < 50; i++) meds[i] = NULL;
    }
    ~PharmacyInventory() { for (int i = 0; i < medCount; i++) delete meds[i]; }

    // -- Medicine --
    void addMedicine(string id, string name, string cat, double price, int type) {
        if (findMed(id) != -1) { cout << "Duplicate ID!\n"; return; }
        if (medCount >= 50)    { cout << "Storage full!\n"; return; }
        if (price < 0)         { cout << "Price invalid!\n"; return; }
        if (type == 1)      meds[medCount++] = new Tablet(id, name, cat, price);
        else if (type == 2) meds[medCount++] = new Syrup(id, name, cat, price);
        else                meds[medCount++] = new Injection(id, name, cat, price);
        cout << "Added: " << name << " [" << meds[medCount-1]->getType() << "]\n";
    }

    void displayMedicines() {
        if (medCount == 0) { cout << "No medicines.\n"; return; }
        cout << "\n" << left << setw(10)<<"ID" << setw(22)<<"Name" << setw(12)<<"Type" << "Price\n";
        for (int i = 0; i < medCount; i++) meds[i]->display();  // Polymorphism
    }

    void sortByName() {  // Bubble Sort
        for (int i = 0; i < medCount - 1; i++)
            for (int j = 0; j < medCount - i - 1; j++)
                if (meds[j]->getName() > meds[j+1]->getName()) {
                    Medicine* temp = meds[j]; meds[j] = meds[j+1]; meds[j+1] = temp;
                }
        cout << "Sorted.\n"; displayMedicines();
    }

    void searchMedicine(string query) {  // Linear Search
        int idx = findMed(query);
        if (idx == -1) { cout << "Not found!\n"; return; }
        meds[idx]->display();
        cout << "Stock: " << getStock(meds[idx]->getId()) << " units\n";
    }

    // -- Batch --
    void addBatch(string bId, string mId, string exp, string sId, int qty) {
        if (findBatch(bId) != -1)    { cout << "Duplicate Batch ID!\n"; return; }
        if (findMed(mId) == -1)      { cout << "Medicine not found!\n"; return; }
        if (findSupplier(sId) == -1) { cout << "Supplier not found!\n"; return; }
        if (qty <= 0)                { cout << "Qty must be > 0!\n"; return; }
        batches[batchCount++] = Batch(bId, mId, exp, sId, qty);
        txnList.add("PURCHASE", mId, bId, qty, 0, today);
        cout << "Batch added: " << bId << "\n";
    }

    void displayInventory() {
        cout << "\n" << left << setw(10)<<"Batch" << setw(10)<<"Med" << setw(8)<<"Qty" << setw(14)<<"Expiry" << "Status\n";
        for (int i = 0; i < batchCount; i++) {
            cout << left << setw(10) << batches[i].getBatchId() << setw(10) << batches[i].getMedId()
                 << setw(8) << batches[i].getQuantity() << setw(14) << batches[i].getExpiry();
            if (batches[i].getExpiry() <= today)   cout << "[EXPIRED]\n";
            else if (batches[i].getQuantity() < 5) cout << "[LOW STOCK]\n";
            else                                   cout << "[ACTIVE]\n";
        }
    }

    void checkExpiry() {  // Abstraction: hides the loop logic
        cout << "\n-- Alerts --\n";
        for (int i = 0; i < batchCount; i++) {
            if (batches[i].getExpiry() <= today)
                cout << "EXPIRED : " << batches[i].getBatchId() << "\n";
            else if (batches[i].getQuantity() < 5)
                cout << "LOW     : " << batches[i].getBatchId() << " Qty=" << batches[i].getQuantity() << "\n";
        }
    }

    // -- Supplier --
    void addSupplier(string id, string name, string phone) {
        if (findSupplier(id) != -1) { cout << "Duplicate Supplier!\n"; return; }
        if (supplierCount >= 30)    { cout << "Storage full!\n"; return; }
        suppliers[supplierCount++] = Supplier(id, name, phone);
        cout << "Supplier added: " << name << "\n";
    }
    void displaySuppliers() {
        cout << "\n" << left << setw(12)<<"ID" << setw(24)<<"Name" << "Phone\n";
        for (int i = 0; i < supplierCount; i++) suppliers[i].display();
    }

    // -- FEFO Sell: First Expiry First Out (Abstraction)
    void sellMedicine(string query, int reqQty) {
        int mIdx = findMed(query);
        if (mIdx == -1) { cout << "Medicine not found!\n"; return; }
        string mId = meds[mIdx]->getId();
        int available = getStock(mId);
        if (reqQty <= 0 || reqQty > available) { cout << "Not enough stock! Available: " << available << "\n"; return; }
        undoStack.setup(mId, reqQty, reqQty * meds[mIdx]->getPrice());
        int remaining = reqQty;
        while (remaining > 0) {
            int best = -1;
            for (int i = 0; i < batchCount; i++) {
                if (batches[i].getMedId() == mId && batches[i].getQuantity() > 0 && batches[i].getExpiry() > today)
                    if (best == -1 || batches[i].getExpiry() < batches[best].getExpiry()) best = i;
            }
            if (best == -1) break;
            int take = remaining < batches[best].getQuantity() ? remaining : batches[best].getQuantity();
            batches[best].removeQuantity(take);
            remaining -= take;
            undoStack.push(batches[best].getBatchId(), take);
            cout << "  Sold " << take << " from Batch " << batches[best].getBatchId() << "\n";
        }
        txnList.add("SALE", mId, "MULTI", reqQty, undoStack.getTotalAmt(), today);
        cout << "Total: Rs." << undoStack.getTotalAmt() << "\n";
    }

    // -- Stack: Undo (LIFO)
    void undoLastSale() {
        if (undoStack.isEmpty()) { cout << "Nothing to undo.\n"; return; }
        for (int i = 0; i < undoStack.getCount(); i++) {
            int idx = findBatch(undoStack.getItem(i).batchId);
            if (idx != -1) batches[idx].addQuantity(undoStack.getItem(i).qty);
        }
        txnList.add("UNDO", undoStack.getMedId(), "-", undoStack.getTotalQty(), 0, today);
        cout << "Undo done! Restored " << undoStack.getTotalQty() << " units.\n";
        undoStack.reset();
    }

    // -- Queue: Restock (FIFO)
    void requestRestock(string mId, string sId, int qty) {
        if (findMed(mId) == -1)      { cout << "Medicine not found!\n"; return; }
        if (findSupplier(sId) == -1) { cout << "Supplier not found!\n"; return; }
        if (qty <= 0)                { cout << "Qty must be > 0!\n"; return; }
        if (restockQueue.isFull())   { cout << "Queue full!\n"; return; }
        restockQueue.enqueue(mId, sId, qty);
        cout << "Restock queued. Pending: " << restockQueue.getCount() << "\n";
    }

    void processRestock(string newBatchId, string expiry) {
        if (restockQueue.isEmpty()) { cout << "No pending requests.\n"; return; }
        Request r = restockQueue.getFront();
        if (findBatch(newBatchId) != -1) { cout << "Batch ID exists! Request kept.\n"; return; }
        batches[batchCount++] = Batch(newBatchId, r.medId, expiry, r.supplierId, r.qty);
        txnList.add("RESTOCK", r.medId, newBatchId, r.qty, 0, today);
        restockQueue.dequeue();
        cout << "Restock done for " << r.medId << "\n";
    }

    void displayTransactions() { txnList.display(); }
};

// -- No main() here. Add main() separately to run. --
// Example:
//   PharmacyInventory shop;
//   shop.addSupplier("S01", "Cipla", "9876543210");
//   shop.addMedicine("M01", "Paracetamol", "Analgesic", 15.5, 1); // 1=Tablet
//   shop.addBatch("B01", "M01", "2027-06-01", "S01", 50);
//   shop.sellMedicine("M01", 10);
//   shop.undoLastSale();
