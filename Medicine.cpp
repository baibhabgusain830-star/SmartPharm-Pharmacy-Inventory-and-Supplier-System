#include "Medicine.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

Medicine::Medicine() 
    : medicineId(""), name(""), barcode(""), category("General"), unitPrice(0.0) {}

Medicine::Medicine(const std::string& id, const std::string& name, 
                   const std::string& barcode, const std::string& category, 
                   double price)
    : medicineId(id), name(name), barcode(barcode), 
      category(category.empty() ? "General" : category), unitPrice(price) {}

std::string Medicine::getMedicineId() const {
    return medicineId;
}

std::string Medicine::getName() const {
    return name;
}

std::string Medicine::getBarcode() const {
    return barcode;
}

std::string Medicine::getCategory() const {
    return category;
}

double Medicine::getUnitPrice() const {
    return unitPrice;
}

bool Medicine::setMedicineId(const std::string& id) {
    if (!id.empty() && id.find('|') == std::string::npos) {
        medicineId = id;
        return true;
    }
    return false;
}

bool Medicine::setName(const std::string& n) {
    if (!n.empty() && n.find('|') == std::string::npos) {
        name = n;
        return true;
    }
    return false;
}

bool Medicine::setBarcode(const std::string& b) {
    if (!b.empty() && b.find('|') == std::string::npos) {
        barcode = b;
        return true;
    }
    return false;
}

bool Medicine::setCategory(const std::string& c) {
    if (!c.empty() && c.find('|') == std::string::npos) {
        category = c;
        return true;
    }
    return false;
}

bool Medicine::setUnitPrice(double price) {
    if (price >= 0.0) {
        unitPrice = price;
        return true;
    }
    return false;
}

bool Medicine::isValid() const {
    return !medicineId.empty() && !name.empty() && !barcode.empty() && 
           medicineId.find('|') == std::string::npos &&
           name.find('|') == std::string::npos &&
           barcode.find('|') == std::string::npos &&
           unitPrice >= 0.0;
}

void Medicine::display() const {
    std::cout << std::left 
              << std::setw(12) << medicineId 
              << std::setw(24) << name 
              << std::setw(16) << barcode 
              << std::setw(16) << category 
              << "Rs. " << std::fixed << std::setprecision(2) << unitPrice << "\n";
}

std::string Medicine::serialize() const {
    std::ostringstream oss;
    oss << medicineId << "|" << name << "|" << barcode << "|" << category << "|" << unitPrice;
    return oss.str();
}

Medicine Medicine::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string id, name, barcode, cat, priceStr;
    if (std::getline(iss, id, '|') &&
        std::getline(iss, name, '|') &&
        std::getline(iss, barcode, '|') &&
        std::getline(iss, cat, '|') &&
        std::getline(iss, priceStr, '|')) {
        try {
            double price = std::stod(priceStr);
            Medicine med(id, name, barcode, cat, price);
            if (med.isValid()) {
                return med;
            }
        } catch (...) {
            return Medicine();
        }
    }
    return Medicine();
}
