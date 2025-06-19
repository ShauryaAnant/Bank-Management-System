#pragma once

#include "Account.h"
#include "BankPolicy.h"

class SavingsAccount : public Account {
public:
    SavingsAccount(int accNo, double initialBalance, Customer* owner,
                  AccountType type = AccountType::SAVINGS);
    
    bool deposit(double amount) override;
    bool withdraw(double amount) override;
    void applyMonthlyUpdate() override;
    double calculateInterest() const override;
    
    double getInterestRate() const { return BankPolicy::getSavingsInterestRate(); }
    void setInterestRate(double newRate) { BankPolicy::setSavingsInterestRate(newRate); }
}; 