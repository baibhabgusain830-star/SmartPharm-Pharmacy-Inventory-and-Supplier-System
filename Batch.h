#ifndef BATCH_H
#define BATCH_H

#include <string>
#include <iostream>
#include "DateUtils.h"

class Batch {
private:
    std::string batchId;
    std::string medicineId;
    std::string expiryDate; // Format: YYYY-MM-DD
    int quantity;
    double purchasePrice;

public:
    Batch();
    Batch(const std::string& bId, const std::string& mId, 
          const std::string& expDate, int qty, double cost);

    // Getters
    std::string getBatchId() const;
    std::string getMedicineId() const;
    std::string getExpiryDate() const;
    int getQuantity() const;
    double getPurchasePrice() const;

    // Setters with validation
    bool setBatchId(const std::string& bId);
    bool setMedicineId(const std::string& mId);
    bool setExpiryDate(const std::string& expDate);
    bool setQuantity(int qty);
    bool setPurchasePrice(double cost);

    bool deductQuantity(int amount);
    bool addQuantity(int amount);

    // Expiry verification using normalized DateUtils calendar logic
    bool isExpired(const std::string& currentDate) const;

    // Validity check
    bool isValid() const;

    // Display
    void display() const;

    // Serialization
    std::string serialize() const;
    static Batch deserialize(const std::string& line);
};

#endif // BATCH_H
