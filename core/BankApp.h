#pragma once

#include "Customer.h"
#include "Account.h"
#include "Transaction.h"
#include "Database.h"
#include <memory>
#include <string>

class BankApp {
private:
    static BankApp* instance;
    std::string bankName;
    Customer* currentCustomer;
    Account* currentAccount;

    // Input validation
    bool isValidName(const std::string& name);
    bool isValidPhone(const std::string& phone);
    bool isValidUsername(const std::string& username);
    bool isValidPassword(const std::string& password);

    // Private constructor for singleton
    BankApp(const std::string& bankName);
    
    // UI helper methods
    void displayMainMenu();
    void displayCustomerMenu();
    // void displayAccountMenu();
    void listAccounts();
    
    // Customer operations
    void handleCustomerLogin();
    void handleCustomerRegistration();
    // void handleCustomerLogout();
    
    // Account operations
    void handleAccountCreation();
    void handleAccountSelection();
    // void handleBalanceInquiry();
    // void handleDeposit();
    // void handleWithdrawal();
    // void handleTransfer();
    // void handleAccountStatement();
    // void handleAccountClosure();

    void handleChangePassword();

public:
    static BankApp* getInstance(const std::string& bankName = "MyBank");
    void run();
    ~BankApp();  // Add destructor declaration
    
    // Transaction handling
    // void performTransaction(int accountNumber, const std::string& type);
}; 