#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <iostream>
#include <string>
using namespace std;

class Transaction {
public:
    int transId;
    string type;
    int medId;
    double amount;

    Transaction(int tId = 0, string t = "", int mId = 0, double amt = 0.0) {
        transId = tId;
        type = t;
        medId = mId;
        amount = amt;
    }

    void display() {
        cout << "TransID: " << transId << " | " << type 
             << " | MedID: " << medId << " | Amount: $" << amount << endl;
    }
};

// Stack Node for Undo/Redo
struct StackNode {
    Transaction data;
    StackNode* next;
    StackNode(Transaction t) {
        data = t;
        next = nullptr;
    }
};

// Dual Stack for Undo and Redo
class DualStack {
private:
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

public:
    DualStack() {
        undoTop = nullptr;
        redoTop = nullptr;
    }

    void recordAction(Transaction t) {
        push(undoTop, t);
    }

    void undo() {
        if (undoTop == nullptr) {
            cout << "Nothing to undo!" << endl;
            return;
        }
        Transaction t = pop(undoTop);
        push(redoTop, t);
        cout << "Undid Transaction ID: " << t.transId << endl;
    }

    void redo() {
        if (redoTop == nullptr) {
            cout << "Nothing to redo!" << endl;
            return;
        }
        Transaction t = pop(redoTop);
        push(undoTop, t);
        cout << "Redid Transaction ID: " << t.transId << endl;
    }
};

// Doubly Linked List Node for Transaction History
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

// Doubly Linked List for Chronological Transaction History
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
        DLLNode* temp = head;
        while (temp != nullptr) {
            temp->data.display();
            temp = temp->next;
        }
    }
};

#endif