#include "AuditableSavingsAccount.h"
#include "Database.h"
#include "Transaction.h"
#include <string>

AuditableSavingsAccount::AuditableSavingsAccount(int accNo, double initialBalance, Customer* owner)
    : SavingsAccount(accNo, initialBalance, owner, AccountType::AUDITABLE_SAVINGS) {
    // No audit log
}

bool AuditableSavingsAccount::deposit(double amount) {
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

bool AuditableSavingsAccount::withdraw(double amount) {
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

void AuditableSavingsAccount::applyMonthlyUpdate() {
    // Calculate interest using auditable rate
    double interest = getBalance() * BankPolicy::getAuditableInterestRate() / 12.0;
    updateBalance(getBalance() + interest);
    // Record monthly interest as a transaction
    auto* db = Database::getInstance();
    auto tx = std::make_unique<MonthlyUpdateTransaction>(this, interest, "Interest");
    db->addTransaction(getAccountNumber(), std::move(tx));
} 