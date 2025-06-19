#include "../include/AuditableSavingsAccount.h"
#include <chrono>
#include <iomanip>
#include <sstream>

AuditableSavingsAccount::AuditableSavingsAccount(int accNo, double initialBalance, Customer* owner,
                                               const std::string& logFileName)
    : SavingsAccount(accNo, initialBalance, owner, AccountType::AUDITABLE_SAVINGS)
    , Auditable(logFileName) {
    // Log initial account creation
    AuditEntry entry;
    entry.timestamp = getCurrentTimestamp();
    entry.action = "Account created";
    entry.amount = initialBalance;
    entry.balance = initialBalance;
    auditLog.push_back(entry);
    logAction("Account created with initial balance of $" + std::to_string(initialBalance));
}

bool AuditableSavingsAccount::deposit(double amount) {
    bool success = SavingsAccount::deposit(amount);
    if (success) {
        // Log successful deposit
        AuditEntry entry;
        entry.timestamp = getCurrentTimestamp();
        entry.action = "Deposit";
        entry.amount = amount;
        entry.balance = getBalance();
        auditLog.push_back(entry);
        logAction("Deposit of $" + std::to_string(amount) + " successful");
    } else {
        logAction("Deposit of $" + std::to_string(amount) + " failed");
    }
    return success;
}

bool AuditableSavingsAccount::withdraw(double amount) {
    bool success = SavingsAccount::withdraw(amount);
    if (success) {
        // Log successful withdrawal
        AuditEntry entry;
        entry.timestamp = getCurrentTimestamp();
        entry.action = "Withdrawal";
        entry.amount = amount;
        entry.balance = getBalance();
        auditLog.push_back(entry);
        logAction("Withdrawal of $" + std::to_string(amount) + " successful");
    } else {
        logAction("Withdrawal of $" + std::to_string(amount) + " failed");
    }
    return success;
}

void AuditableSavingsAccount::applyMonthlyUpdate() {
    double interest = calculateInterest();
    SavingsAccount::applyMonthlyUpdate();
    
    // Log monthly update
    AuditEntry entry;
    entry.timestamp = getCurrentTimestamp();
    entry.action = "Monthly Interest";
    entry.amount = interest;
    entry.balance = getBalance();
    auditLog.push_back(entry);
    
    logAction("Monthly interest of $" + std::to_string(interest) + " applied");
}

std::string AuditableSavingsAccount::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
} 