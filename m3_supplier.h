#ifndef SUPPLIER_H
#define SUPPLIER_H

#include <iostream>
#include <string>
using namespace std;

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
private:
    static const int CAPACITY = 5;
    Supplier orders[CAPACITY];
    int front, rear, count;

public:
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
};

#endif