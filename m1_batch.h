#ifndef BATCH_H
#define BATCH_H

#include <iostream>
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

#endif