#ifndef SUPPLIER_H
#define SUPPLIER_H

#include <string>
#include <iostream>

class Supplier {
private:
    std::string supplierId;
    std::string name;
    std::string contactNumber;
    std::string email;

public:
    Supplier();
    Supplier(const std::string& id, const std::string& name, 
             const std::string& contact, const std::string& email);

    // Getters
    std::string getSupplierId() const;
    std::string getName() const;
    std::string getContactNumber() const;
    std::string getEmail() const;

    // Setters with validation
    bool setSupplierId(const std::string& id);
    bool setName(const std::string& name);
    bool setContactNumber(const std::string& contact);
    bool setEmail(const std::string& email);

    // Validity check
    bool isValid() const;

    // Display
    void display() const;

    // Serialization
    std::string serialize() const;
    static Supplier deserialize(const std::string& line);
};

#endif // SUPPLIER_H
