#pragma once
#include <string>

class BankInterface {
public:
    // User operations
    static std::string login(const std::string& username, const std::string& password);
    static std::string registerUser(const std::string& username, const std::string& password, const std::string& name, const std::string& phone);
    static std::string changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);
    static std::string getUserDetails(const std::string& username);
    static std::string listAccounts(const std::string& username);
    static std::string createAccount(const std::string& username, const std::string& type, double initialBalance, const std::string& pin);
    static std::string getAccountDetails(int accountId);
    static std::string deposit(int accountId, double amount);
    static std::string withdraw(int accountId, double amount, const std::string& pin);
    static std::string transfer(int accountId, int toAccount, double amount, const std::string& pin);
    static std::string getTransactions(int accountId);
    static std::string closeAccount(int accountId, const std::string& pin);
    static std::string logout();

    // Admin operations
    static std::string adminLogin(const std::string& username, const std::string& password);
    static std::string adminChangePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);
    static std::string getPolicy(const std::string& username, const std::string& password);
    static std::string setPolicy(const std::string& username, const std::string& password, double savingsInterestRate, double currentAccountFee, double auditableInterestRate);
    static std::string monthlyUpdate(const std::string& username, const std::string& password);
    static std::string listUsers(const std::string& adminUsername, const std::string& adminPassword);
    static std::string listAllAccounts(const std::string& adminUsername, const std::string& adminPassword);
}; 