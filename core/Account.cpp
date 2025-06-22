#include "Account.h"
#include <stdexcept>

Account::Account(int accNo, double initialBalance, Customer* owner, AccountType type)
    : accountNumber(accNo), balance(initialBalance), owner(owner), type(type) {
    if (initialBalance < 0) {
        throw std::invalid_argument("Initial balance cannot be negative");
    }
    if (!owner) {
        throw std::invalid_argument("Owner cannot be null");
    }
}

void Account::updateBalance(double newBalance) {
    if (newBalance < 0) {
        throw std::runtime_error("Balance cannot be negative");
    }
    balance = newBalance;
}
