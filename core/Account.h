#pragma once

#include <string>
#include <memory>
#include <vector>
#include "ITransaction.h"

class Customer;  // Forward declaration

enum class AccountType {
    SAVINGS,
    CURRENT,
    AUDITABLE_SAVINGS
};

class Account {
protected:
    int accountNumber;
    double balance;
    Customer* owner;
    // std::vector<std::unique_ptr<ITransaction>> transactionHistory;
    AccountType type;

public:
    Account(int accNo, double initialBalance, Customer* owner, AccountType type);
    virtual ~Account() = default;

    // Pure virtual methods
    virtual bool deposit(double amount) = 0;
    virtual bool withdraw(double amount) = 0;
    virtual void applyMonthlyUpdate() = 0;
    virtual double calculateInterest() const = 0;

    // Common methods
    int getAccountNumber() const { return accountNumber; }
    double getBalance() const { return balance; }
    Customer* getOwner() const { return owner; }
    AccountType getType() const { return type; }
    std::string getTypeString() const {
        switch (type) {
            case AccountType::SAVINGS:
                return "Savings";
            case AccountType::CURRENT:
                return "Current";
            case AccountType::AUDITABLE_SAVINGS:
                return "Auditable Savings";
            default:
                return "Unknown";
        }
    }
    
    // Transaction history methods
    void addTransaction(std::unique_ptr<ITransaction> transaction);
    // const std::vector<std::unique_ptr<ITransaction>>& getTransactionHistory() const;

protected:
    void updateBalance(double newBalance);
}; 
