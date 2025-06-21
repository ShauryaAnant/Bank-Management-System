#include "../include/BankPolicy.h"
#include <fstream>
#include <iostream>

// Initialize static members
double BankPolicy::savingsInterestRate = 0.05; // 5% default
double BankPolicy::currentAccountFee = 10.0;   // $10 default
double BankPolicy::auditableInterestRate = 0.0; // 0% default

double BankPolicy::getSavingsInterestRate() {
    return savingsInterestRate;
}
void BankPolicy::setSavingsInterestRate(double rate) {
    savingsInterestRate = rate;
}
double BankPolicy::getCurrentAccountFee() {
    return currentAccountFee;
}
void BankPolicy::setCurrentAccountFee(double fee) {
    currentAccountFee = fee;
}
double BankPolicy::getAuditableInterestRate() {
    return auditableInterestRate;
}
void BankPolicy::setAuditableInterestRate(double rate) {
    auditableInterestRate = rate;
}

void BankPolicy::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open policy file for writing: " << filename << std::endl;
        return;
    }
    file << savingsInterestRate << '\n';
    file << currentAccountFee << '\n';
    file << auditableInterestRate << '\n';
}

void BankPolicy::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Use defaults if file doesn't exist
        return;
    }
    file >> savingsInterestRate;
    file >> currentAccountFee;
    file >> auditableInterestRate;
} 