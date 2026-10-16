#ifndef MEDICINE_H
#define MEDICINE_H

#include <string>
#include <iostream>

class Medicine {
private:
    std::string medicineId;
    std::string name;
    std::string barcode;
    std::string category;
    double unitPrice;

public:
    Medicine();
    Medicine(const std::string& id, const std::string& name, 
             const std::string& barcode, const std::string& category, 
             double price);

    // Getters
    std::string getMedicineId() const;
    std::string getName() const;
    std::string getBarcode() const;
    std::string getCategory() const;
    double getUnitPrice() const;

    // Setters with validation
    bool setMedicineId(const std::string& id);
    bool setName(const std::string& name);
    bool setBarcode(const std::string& barcode);
    bool setCategory(const std::string& category);
    bool setUnitPrice(double price);

    // Validity check
    bool isValid() const;

    // Display
    void display() const;

    // Serialization for file persistence
    std::string serialize() const;
    static Medicine deserialize(const std::string& line);
};

#endif // MEDICINE_H
