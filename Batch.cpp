#include "Batch.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <climits>
#include <stdexcept>

Batch::Batch() 
    : batchId(""), medicineId(""), expiryDate(""), quantity(0), purchasePrice(0.0) {}

Batch::Batch(const std::string& bId, const std::string& mId, 
             const std::string& expDate, int qty, double cost)
    : batchId(bId), medicineId(mId), expiryDate(expDate), quantity(qty), purchasePrice(cost) {}

std::string Batch::getBatchId() const {
    return batchId;
}

std::string Batch::getMedicineId() const {
    return medicineId;
}

std::string Batch::getExpiryDate() const {
    return expiryDate;
}

int Batch::getQuantity() const {
    return quantity;
}

double Batch::getPurchasePrice() const {
    return purchasePrice;
}

bool Batch::setBatchId(const std::string& bId) {
    if (!bId.empty() && bId.find('|') == std::string::npos) {
        batchId = bId;
        return true;
    }
    return false;
}

bool Batch::setMedicineId(const std::string& mId) {
    if (!mId.empty() && mId.find('|') == std::string::npos) {
        medicineId = mId;
        return true;
    }
    return false;
}

bool Batch::setExpiryDate(const std::string& expDate) {
    if (DateUtils::isValidDate(expDate)) {
        expiryDate = expDate;
        return true;
    }
    return false;
}

bool Batch::setQuantity(int qty) {
    if (qty >= 0) {
        quantity = qty;
        return true;
    }
    return false;
}

bool Batch::setPurchasePrice(double cost) {
    if (cost >= 0.0) {
        purchasePrice = cost;
        return true;
    }
    return false;
}

bool Batch::deductQuantity(int amount) {
    if (amount > 0 && amount <= quantity) {
        quantity -= amount;
        return true;
    }
    return false;
}

bool Batch::addQuantity(int amount) {
    if (amount <= 0) return false;
    // Prevent integer overflow
    if (INT_MAX - amount < quantity) {
        return false;
    }
    quantity += amount;
    return true;
}

bool Batch::isExpired(const std::string& currentDate) const {
    return DateUtils::isExpired(expiryDate, currentDate);
}

bool Batch::isValid() const {
    return !batchId.empty() && !medicineId.empty() && 
           batchId.find('|') == std::string::npos &&
           medicineId.find('|') == std::string::npos &&
           DateUtils::isValidDate(expiryDate) && 
           quantity >= 0 && purchasePrice >= 0.0;
}

void Batch::display() const {
    std::cout << std::left 
              << std::setw(12) << batchId 
              << std::setw(14) << medicineId 
              << std::setw(14) << expiryDate 
              << std::setw(10) << quantity 
              << "Rs. " << std::fixed << std::setprecision(2) << purchasePrice << "\n";
}

std::string Batch::serialize() const {
    std::ostringstream oss;
    oss << batchId << "|" << medicineId << "|" << expiryDate << "|" 
        << quantity << "|" << purchasePrice;
    return oss.str();
}

Batch Batch::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string bId, mId, exp, qtyStr, priceStr;
    if (std::getline(iss, bId, '|') &&
        std::getline(iss, mId, '|') &&
        std::getline(iss, exp, '|') &&
        std::getline(iss, qtyStr, '|') &&
        std::getline(iss, priceStr, '|')) {
        try {
            int qty = std::stoi(qtyStr);
            double cost = std::stod(priceStr);
            Batch b(bId, mId, exp, qty, cost);
            if (b.isValid()) {
                return b;
            }
        } catch (...) {
            return Batch();
        }
    }
    return Batch();
}
