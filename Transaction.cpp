#include "Transaction.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

Transaction::Transaction() 
    : transactionId(0), type(""), medicineId(""), batchId(""), 
      quantity(0), unitPrice(0.0), totalAmount(0.0), date("") {}

Transaction::Transaction(int id, const std::string& type, const std::string& mId, 
                         const std::string& bId, int qty, double price, 
                         double total, const std::string& date)
    : transactionId(0), type(""), medicineId(""), batchId(""), 
      quantity(0), unitPrice(0.0), totalAmount(0.0), date("") {
    setTransactionId(id);
    setType(type);
    setMedicineId(mId);
    setBatchId(bId);
    setQuantity(qty);
    setUnitPrice(price);
    setTotalAmount(total);
    setDate(date);
}

int Transaction::getTransactionId() const {
    return transactionId;
}

std::string Transaction::getType() const {
    return type;
}

std::string Transaction::getMedicineId() const {
    return medicineId;
}

std::string Transaction::getBatchId() const {
    return batchId;
}

int Transaction::getQuantity() const {
    return quantity;
}

double Transaction::getUnitPrice() const {
    return unitPrice;
}

double Transaction::getTotalAmount() const {
    return totalAmount;
}

std::string Transaction::getDate() const {
    return date;
}

bool Transaction::setTransactionId(int id) {
    if (id > 0) {
        transactionId = id;
        return true;
    }
    return false;
}

bool Transaction::setType(const std::string& t) {
    if (t == "PURCHASE" || t == "SALE" || t == "UNDO_SALE") {
        type = t;
        return true;
    }
    return false;
}

bool Transaction::setMedicineId(const std::string& mId) {
    if (!mId.empty() && mId.find('|') == std::string::npos) {
        medicineId = mId;
        return true;
    }
    return false;
}

bool Transaction::setBatchId(const std::string& bId) {
    if (!bId.empty() && bId.find('|') == std::string::npos) {
        batchId = bId;
        return true;
    }
    return false;
}

bool Transaction::setQuantity(int qty) {
    if (qty > 0) {
        quantity = qty;
        return true;
    }
    return false;
}

bool Transaction::setUnitPrice(double price) {
    if (price >= 0.0) {
        unitPrice = price;
        return true;
    }
    return false;
}

bool Transaction::setTotalAmount(double total) {
    // Note: total can be negative for UNDO_SALE (representing credit/refund)
    totalAmount = total;
    return true;
}

bool Transaction::setDate(const std::string& d) {
    if (DateUtils::isValidDate(d)) {
        date = d;
        return true;
    }
    return false;
}

bool Transaction::isValid() const {
    return transactionId > 0 && 
           (type == "PURCHASE" || type == "SALE" || type == "UNDO_SALE") &&
           !medicineId.empty() && !batchId.empty() && 
           quantity > 0 && unitPrice >= 0.0 && 
           DateUtils::isValidDate(date);
}

void Transaction::display() const {
    std::cout << std::left 
              << std::setw(8)  << transactionId 
              << std::setw(12) << type 
              << std::setw(12) << medicineId 
              << std::setw(12) << batchId 
              << std::setw(8)  << quantity 
              << "Rs." << std::fixed << std::setprecision(2) << std::setw(8) << unitPrice 
              << "Rs." << std::setw(10) << totalAmount 
              << date << "\n";
}

std::string Transaction::serialize() const {
    std::ostringstream oss;
    oss << transactionId << "|" << type << "|" << medicineId << "|" 
        << batchId << "|" << quantity << "|" << unitPrice << "|" 
        << totalAmount << "|" << date;
    return oss.str();
}

Transaction Transaction::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string idStr, type, mId, bId, qtyStr, priceStr, totalStr, date;
    if (std::getline(iss, idStr, '|') &&
        std::getline(iss, type, '|') &&
        std::getline(iss, mId, '|') &&
        std::getline(iss, bId, '|') &&
        std::getline(iss, qtyStr, '|') &&
        std::getline(iss, priceStr, '|') &&
        std::getline(iss, totalStr, '|') &&
        std::getline(iss, date, '|')) {
        try {
            int id = std::stoi(idStr);
            int qty = std::stoi(qtyStr);
            double price = std::stod(priceStr);
            double total = std::stod(totalStr);
            Transaction txn(id, type, mId, bId, qty, price, total, date);
            if (txn.isValid()) {
                return txn;
            }
        } catch (...) {
            return Transaction();
        }
    }
    return Transaction();
}
