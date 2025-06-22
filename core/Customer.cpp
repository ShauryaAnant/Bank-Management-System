#include "Customer.h"
#include <algorithm>
#include <stdexcept>

Customer::Customer(int id, const std::string& name, const std::string& phone)
    : id(id), name(name), phone(phone) {
    if (name.empty()) {
        throw std::invalid_argument("Name cannot be empty");
    }
    if (phone.empty()) {
        throw std::invalid_argument("Phone number cannot be empty");
    }
}

void Customer::addAccount(std::unique_ptr<Account> account) {
    if (!account) {
        throw std::invalid_argument("Account cannot be null");
    }
    accounts.push_back(std::move(account));
}

bool Customer::removeAccount(int accountNumber) {
    auto it = std::find_if(accounts.begin(), accounts.end(),
        [accountNumber](const auto& account) {
            return account->getAccountNumber() == accountNumber;
        });
    
    if (it != accounts.end()) {
        accounts.erase(it);
        return true;
    }
    return false;
}

const std::vector<std::unique_ptr<Account>>& Customer::getAccounts() const {
    return accounts;
}

Account* Customer::findAccount(int accountNumber) const {
    auto it = std::find_if(accounts.begin(), accounts.end(),
        [accountNumber](const auto& account) {
            return account->getAccountNumber() == accountNumber;
        });
    
    return it != accounts.end() ? it->get() : nullptr;
} 