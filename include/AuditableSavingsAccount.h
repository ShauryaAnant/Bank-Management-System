#pragma once

#include "SavingsAccount.h"
#include "Auditable.h"
#include <string>
#include <vector>

class AuditableSavingsAccount : public virtual SavingsAccount, public virtual Auditable {
private:
    struct AuditEntry {
        std::string timestamp;
        std::string action;
        double amount;
        double balance;
    };
    std::vector<AuditEntry> auditLog;
    std::string getCurrentTimestamp() const;

public:
    AuditableSavingsAccount(int accNo, double initialBalance, Customer* owner,
                           const std::string& logFileName = "savings_audit.log");
    
    bool deposit(double amount) override;
    bool withdraw(double amount) override;
    void applyMonthlyUpdate() override;

    // Audit log methods
    const std::vector<AuditEntry>& getAuditLog() const { return auditLog; }
    void clearAuditLog() { auditLog.clear(); }
}; 