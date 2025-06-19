#pragma once

#include "SavingsAccount.h"
#include <string>

class AuditableSavingsAccount : public virtual SavingsAccount {
public:
    AuditableSavingsAccount(int accNo, double initialBalance, Customer* owner);
    bool deposit(double amount) override;
    bool withdraw(double amount) override;
    void applyMonthlyUpdate() override;
}; 