#ifndef TRANSACTION_LIST_H
#define TRANSACTION_LIST_H

#include "Transaction.h"

// Node definition for custom singly linked list
struct TransactionNode {
    Transaction data;
    TransactionNode* next;

    TransactionNode(const Transaction& txn) : data(txn), next(nullptr) {}
};

// Custom Singly Linked List managing chronological transaction history
class TransactionList {
private:
    TransactionNode* head;
    TransactionNode* tail;
    int count;

public:
    TransactionList();
    ~TransactionList();

    // Deep copy semantics
    TransactionList(const TransactionList& other);
    TransactionList& operator=(const TransactionList& other);

    // Core operations
    void insertLast(const Transaction& txn);
    void displayAll() const;
    void clear();
    int getCount() const;

    // Head accessor for file export
    const TransactionNode* getHead() const;
};

#endif // TRANSACTION_LIST_H
