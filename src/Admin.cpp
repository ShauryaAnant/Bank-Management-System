#include "../include/Admin.h"
#include "../include/BankPolicy.h"
#include "../include/Database.h"
#include <iostream>
#include <fstream>
#include <limits>

std::string Admin::adminUsername = "admin";
std::string Admin::adminPassword = "admin123";

void Admin::loadCredentials(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::getline(file, adminUsername);
        std::getline(file, adminPassword);
        if (adminUsername.empty() || adminPassword.empty()) {
            adminUsername = "admin";
            adminPassword = "admin123";
        }
    } else {
        // Save defaults if file doesn't exist
        saveCredentials(filename);
    }
}

void Admin::saveCredentials(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << adminUsername << '\n' << adminPassword << '\n';
    }
}

bool Admin::authenticate(const std::string& username, const std::string& password) {
    loadCredentials();
    return username == adminUsername && password == adminPassword;
}

void Admin::showMenu() {
    int choice = 0;
    do {
        std::cout << "\n--- Admin Portal ---\n";
        std::cout << "1. Change Savings Account Interest Rate (Current: " << BankPolicy::getSavingsInterestRate() << ")\n";
        std::cout << "2. Change Current Account Fee (Current: " << BankPolicy::getCurrentAccountFee() << ")\n";
        std::cout << "3. Change Auditable Account Interest Rate (Current: " << BankPolicy::getAuditableInterestRate() << ")\n";
        std::cout << "4. View All Accounts\n";
        std::cout << "5. Change Admin Password\n";
        std::cout << "6. Exit Admin Portal\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        switch (choice) {
            case 1:
                changeSavingsInterestRate();
                break;
            case 2:
                changeCurrentAccountFee();
                break;
            case 3:
                changeAuditableInterestRate();
                break;
            case 4:
                viewAllAccounts();
                break;
            case 5:
                changeAdminPassword();
                break;
            case 6:
                std::cout << "Exiting Admin Portal.\n";
                break;
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}

void Admin::changeAdminPassword() {
    std::string oldPass, newPass, confirmPass;
    std::cout << "Enter current admin password: ";
    std::getline(std::cin, oldPass);
    if (oldPass != adminPassword) {
        std::cout << "Incorrect current password.\n";
        return;
    }
    std::cout << "Enter new password: ";
    std::getline(std::cin, newPass);
    std::cout << "Confirm new password: ";
    std::getline(std::cin, confirmPass);
    if (newPass != confirmPass || newPass.empty()) {
        std::cout << "Passwords do not match or are empty.\n";
        return;
    }
    adminPassword = newPass;
    saveCredentials();
    std::cout << "Admin password updated successfully.\n";
}

void Admin::changeSavingsInterestRate() {
    double rate;
    std::cout << "Enter new savings account interest rate (e.g., 0.05 for 5%): ";
    std::cin >> rate;
    if (rate < 0) {
        std::cout << "Interest rate cannot be negative.\n";
        return;
    }
    BankPolicy::setSavingsInterestRate(rate);
    std::cout << "Savings account interest rate updated.\n";
}

void Admin::changeCurrentAccountFee() {
    double fee;
    std::cout << "Enter new current account monthly fee: ";
    std::cin >> fee;
    if (fee < 0) {
        std::cout << "Fee cannot be negative.\n";
        return;
    }
    BankPolicy::setCurrentAccountFee(fee);
    std::cout << "Current account fee updated.\n";
}

void Admin::changeAuditableInterestRate() {
    double rate;
    std::cout << "Enter new auditable account interest rate (e.g., 0.01 for 1%): ";
    std::cin >> rate;
    if (rate < 0) {
        std::cout << "Interest rate cannot be negative.\n";
        return;
    }
    BankPolicy::setAuditableInterestRate(rate);
    std::cout << "Auditable account interest rate updated.\n";
}

void Admin::viewAllAccounts() {
    Database* db = Database::getInstance();
    std::cout << "\n--- All Accounts ---\n";
    for (const auto& [id, customerPtr] : db->getCustomers()) {
        std::cout << "Customer ID: " << id << ", Name: " << customerPtr->getName() << "\n";
        for (const auto& accPtr : customerPtr->getAccounts()) {
            std::cout << "  Account #" << accPtr->getAccountNumber() << ", Type: " << accPtr->getTypeString() << ", Balance: " << accPtr->getBalance() << "\n";
        }
    }
} 