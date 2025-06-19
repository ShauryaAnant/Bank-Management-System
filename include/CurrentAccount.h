#pragma once

#include "Account.h"
#include "BankPolicy.h"

class CurrentAccount : public Account {
public:
    CurrentAccount(int accNo, double initialBalance, Customer* owner);
    
    bool deposit(double amount) override;
    bool withdraw(double amount) override;
    void applyMonthlyUpdate() override;
    double calculateInterest() const override; // Returns 0 as current accounts don't earn interest
    
    double getMaintenanceFee() const { return BankPolicy::getCurrentAccountFee(); }
    void setMaintenanceFee(double newFee) { BankPolicy::setCurrentAccountFee(newFee); }
}; 