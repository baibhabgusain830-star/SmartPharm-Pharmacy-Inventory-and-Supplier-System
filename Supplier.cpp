#include "Supplier.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Supplier::Supplier() 
    : supplierId(""), name(""), contactNumber(""), email("") {}

Supplier::Supplier(const std::string& id, const std::string& name, 
                   const std::string& contact, const std::string& email)
    : supplierId(""), name(""), contactNumber(""), email("") {
    setSupplierId(id);
    setName(name);
    setContactNumber(contact);
    setEmail(email);
}

std::string Supplier::getSupplierId() const {
    return supplierId;
}

std::string Supplier::getName() const {
    return name;
}

std::string Supplier::getContactNumber() const {
    return contactNumber;
}

std::string Supplier::getEmail() const {
    return email;
}

bool Supplier::setSupplierId(const std::string& id) {
    if (!id.empty() && id.find('|') == std::string::npos) {
        supplierId = id;
        return true;
    }
    return false;
}

bool Supplier::setName(const std::string& n) {
    if (!n.empty() && n.find('|') == std::string::npos) {
        name = n;
        return true;
    }
    return false;
}

bool Supplier::setContactNumber(const std::string& contact) {
    if (!contact.empty() && contact.find('|') == std::string::npos) {
        contactNumber = contact;
        return true;
    }
    return false;
}

bool Supplier::setEmail(const std::string& em) {
    if (!em.empty() && em.find('|') == std::string::npos) {
        email = em;
        return true;
    }
    return false;
}

bool Supplier::isValid() const {
    return !supplierId.empty() && !name.empty() && !contactNumber.empty();
}

void Supplier::display() const {
    std::cout << std::left 
              << std::setw(12) << supplierId 
              << std::setw(24) << name 
              << std::setw(18) << contactNumber 
              << email << "\n";
}

std::string Supplier::serialize() const {
    std::ostringstream oss;
    oss << supplierId << "|" << name << "|" << contactNumber << "|" << email;
    return oss.str();
}

Supplier Supplier::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string id, name, contact, email;
    if (std::getline(iss, id, '|') &&
        std::getline(iss, name, '|') &&
        std::getline(iss, contact, '|') &&
        std::getline(iss, email, '|')) {
        Supplier sup(id, name, contact, email);
        if (sup.isValid()) {
            return sup;
        }
    }
    return Supplier();
}
