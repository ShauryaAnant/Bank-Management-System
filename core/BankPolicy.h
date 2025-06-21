#pragma once
#include <string>

class BankPolicy {
public:
    // Default values
    static double getSavingsInterestRate();
    static void setSavingsInterestRate(double rate);

    static double getCurrentAccountFee();
    static void setCurrentAccountFee(double fee);

    static double getAuditableInterestRate();
    static void setAuditableInterestRate(double rate);

    static void saveToFile(const std::string& filename);
    static void loadFromFile(const std::string& filename);

private:
    static double savingsInterestRate;
    static double currentAccountFee;
    static double auditableInterestRate;
}; 