#include "TransactionList.h"
#include <iostream>
#include <iomanip>

TransactionList::TransactionList() 
    : head(nullptr), tail(nullptr), count(0) {}

TransactionList::~TransactionList() {
    clear();
}

TransactionList::TransactionList(const TransactionList& other) 
    : head(nullptr), tail(nullptr), count(0) {
    TransactionNode* current = other.head;
    while (current != nullptr) {
        insertLast(current->data);
        current = current->next;
    }
}

TransactionList& TransactionList::operator=(const TransactionList& other) {
    if (this != &other) {
        clear();
        TransactionNode* current = other.head;
        while (current != nullptr) {
            insertLast(current->data);
            current = current->next;
        }
    }
    return *this;
}

void TransactionList::insertLast(const Transaction& txn) {
    if (!txn.isValid()) {
        std::cerr << "[Warning] Attempted to insert invalid transaction into audit trail.\n";
        return;
    }

    TransactionNode* newNode = new TransactionNode(txn);
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    count++;
}

void TransactionList::displayAll() const {
    if (head == nullptr) {
        std::cout << " (No transactions recorded yet)\n";
        return;
    }

    std::cout << " ================================= TRANSACTION AUDIT TRAIL =================================\n";
    std::cout << std::left 
              << std::setw(8)  << "Txn ID" 
              << std::setw(12) << "Type" 
              << std::setw(12) << "Med ID" 
              << std::setw(12) << "Batch ID" 
              << std::setw(8)  << "Qty" 
              << std::setw(11) << "Unit Price" 
              << std::setw(13) << "Total Amount" 
              << "Date\n";
    std::cout << "--------------------------------------------------------------------------------------------\n";

    TransactionNode* current = head;
    while (current != nullptr) {
        current->data.display();
        current = current->next;
    }
    std::cout << "--------------------------------------------------------------------------------------------\n";
    std::cout << " Total logged transactions: " << count << "\n";
}

void TransactionList::clear() {
    TransactionNode* current = head;
    while (current != nullptr) {
        TransactionNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
}

int TransactionList::getCount() const {
    return count;
}

const TransactionNode* TransactionList::getHead() const {
    return head;
}
