#ifndef MEDICINE_H
#define MEDICINE_H

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

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

#endif