#ifndef MEDICINE_H
#define MEDICINE_H

#include <iostream>
#include <string>
using namespace std;

class Medicine {
public:
    int id;
    string barcode;
    string name;
    string category;
    double price;
    int stock;

    Medicine(int i = 0, string b = "", string n = "", string c = "", double p = 0.0, int s = 0) {
        id = i;
        barcode = b;
        name = n;
        category = c;
        price = p;
        stock = s;
    }

    virtual void display() {
        cout << "[" << id << " | Barcode: " << barcode << "] " << name 
             << " | Cat: " << category << " | $" << price << " | Stock: " << stock;
    }
};

class Tablet : public Medicine {
public:
    int mg;
    Tablet(int i, string b, string n, string c, double p, int s, int m)
        : Medicine(i, b, n, c, p, s) {
        mg = m;
    }

    void display() override {
        Medicine::display();
        cout << " | Dosage: " << mg << "mg" << endl;
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
private:
    static const int SIZE = 10;
    HashNode* table[SIZE];

    int hashFunction(int key) {
        return key % SIZE;
    }

public:
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

#endif