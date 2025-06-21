#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ITransaction.h"
#include "Account.h"  // Include full Account definition

// Function declaration
void transactionFun(int accountNumber);
void printAccountStatement(int accountNumber);
void closeAccount(int accountNumber);

class Deposit : public ITransaction {
private:
    Account* account;
    double amount;
    std::string timestamp;
    TransactionType type;

public:
    Deposit(Account* account, double amount);
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return "Deposit"; }
    double getAmount() const override { return amount; }
    std::string getTimestamp() const override { return timestamp; }
    TransactionType getType() const override { return TransactionType::DEPOSIT; }
};

class Withdrawal : public ITransaction {
private:
    Account* account;
    double amount;
    std::string timestamp;
    TransactionType type;

public:
    Withdrawal(Account* account, double amount);
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return "Withdrawal"; }
    double getAmount() const override { return amount; }
    std::string getTimestamp() const override { return timestamp; }
    TransactionType getType() const override { return TransactionType::WITHDRAWAL; }
};

class Transfer : public ITransaction {
private:
    Account* fromAccount;
    Account* toAccount;
    double amount;
    std::string timestamp;
    TransactionType type;

public:
    Transfer(Account* from, Account* to, double amount);
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return "Transfer"; }
    double getAmount() const override { return amount; }
    std::string getTimestamp() const override { return timestamp; }
    TransactionType getType() const override { return TransactionType::TRANSFER; }
    int getFromAccount() const { return fromAccount->getAccountNumber(); }
    int getToAccount() const { return toAccount->getAccountNumber(); }
};

class MonthlyUpdateTransaction : public ITransaction {
private:
    Account* account;
    double amount;
    std::string timestamp;
    std::string description; // e.g., "Interest", "Fee"
    TransactionType type;
public:
    MonthlyUpdateTransaction(Account* account, double amount, const std::string& description);
    bool execute() override { return false; } // Not used for execution
    bool undo() override { return false; } // Not used for undo
    std::string getDescription() const override { return description; }
    double getAmount() const override { return amount; }
    std::string getTimestamp() const override { return timestamp; }
    TransactionType getType() const override { return TransactionType::MONTHLY_UPDATE; }
}; 