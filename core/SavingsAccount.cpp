#include "../include/SavingsAccount.h"
#include "../include/Customer.h"
#include "../include/Database.h"
#include "../include/Transaction.h"
#include <stdexcept>

SavingsAccount::SavingsAccount(int accNo, double initialBalance, Customer* owner,
                  AccountType type)
    : Account(accNo, initialBalance, owner, type) {
    // interestRate parameter removed, use BankPolicy
}

bool SavingsAccount::deposit(double amount) {
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

bool SavingsAccount::withdraw(double amount) {
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

void SavingsAccount::applyMonthlyUpdate() {
    double interest = calculateInterest();
    updateBalance(getBalance() + interest);
    // Record monthly interest as a transaction
    auto* db = Database::getInstance();
    auto tx = std::make_unique<MonthlyUpdateTransaction>(this, interest, "Interest");
    db->addTransaction(getAccountNumber(), std::move(tx));
}

double SavingsAccount::calculateInterest() const {
    return getBalance() * BankPolicy::getSavingsInterestRate() / 12.0;
} 

