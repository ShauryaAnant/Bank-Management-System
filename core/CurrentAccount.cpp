#include "../include/CurrentAccount.h"
#include "../include/Database.h"
#include "../include/Transaction.h"
#include <stdexcept>

CurrentAccount::CurrentAccount(int accNo, double initialBalance, Customer* owner)
    : Account(accNo, initialBalance, owner, AccountType::CURRENT) {
    // maintenanceFee parameter removed, use BankPolicy
}

bool CurrentAccount::deposit(double amount) {
    if (amount <= 0) {
        return false;
    }
    try {
        updateBalance(getBalance() + amount);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool CurrentAccount::withdraw(double amount) {
    if (amount <= 0 || amount > getBalance()) {
        return false;
    }
    try {
        updateBalance(getBalance() - amount);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void CurrentAccount::applyMonthlyUpdate() {
    double fee = BankPolicy::getCurrentAccountFee();
    if (getBalance() < fee) {
        throw std::runtime_error("Insufficient balance for maintenance fee");
    }
    updateBalance(getBalance() - fee);
    // Record monthly fee as a transaction
    auto* db = Database::getInstance();
    auto tx = std::make_unique<MonthlyUpdateTransaction>(this, -fee, "Fee");
    db->addTransaction(getAccountNumber(), std::move(tx));
}

double CurrentAccount::calculateInterest() const {
    return 0.0; // Current accounts don't earn interest
} 