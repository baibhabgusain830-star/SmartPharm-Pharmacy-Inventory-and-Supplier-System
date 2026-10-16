#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <iostream>
#include "DateUtils.h"

class Transaction {
private:
    int transactionId;
    std::string type;       // "PURCHASE", "SALE", "UNDO_SALE"
    std::string medicineId;
    std::string batchId;
    int quantity;
    double unitPrice;
    double totalAmount;
    std::string date;       // Format: YYYY-MM-DD

public:
    Transaction();
    Transaction(int id, const std::string& type, const std::string& mId, 
                const std::string& bId, int qty, double price, 
                double total, const std::string& date);

    // Getters
    int getTransactionId() const;
    std::string getType() const;
    std::string getMedicineId() const;
    std::string getBatchId() const;
    int getQuantity() const;
    double getUnitPrice() const;
    double getTotalAmount() const;
    std::string getDate() const;

    // Setters with validation
    bool setTransactionId(int id);
    bool setType(const std::string& t);
    bool setMedicineId(const std::string& mId);
    bool setBatchId(const std::string& bId);
    bool setQuantity(int qty);
    bool setUnitPrice(double price);
    bool setTotalAmount(double total);
    bool setDate(const std::string& d);

    // Validity check
    bool isValid() const;

    // Display
    void display() const;

    // Serialization
    std::string serialize() const;
    static Transaction deserialize(const std::string& line);
};

#endif // TRANSACTION_H
