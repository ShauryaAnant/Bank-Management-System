#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Account.h"

class Customer {
private:
    int id;
    std::string name;
    std::string phone;
    std::vector<std::unique_ptr<Account>> accounts;

public:
    Customer(int id, const std::string& name, const std::string& phone);
    
    // Account management
    void addAccount(std::unique_ptr<Account> account);
    bool removeAccount(int accountNumber);
    const std::vector<std::unique_ptr<Account>>& getAccounts() const;
    Account* findAccount(int accountNumber) const;

    // Getters
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getPhone() const { return phone; }

    // Setters
    void setName(const std::string& newName) { name = newName; }
    void setPhone(const std::string& newPhone) { phone = newPhone; }
}; 