#pragma once

#include <string>

enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER,
    MONTHLY_UPDATE
};

class ITransaction {
public:
    virtual ~ITransaction() = default;
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual std::string getDescription() const = 0;
    virtual double getAmount() const = 0;
    virtual std::string getTimestamp() const = 0;
    virtual TransactionType getType() const = 0;
}; 