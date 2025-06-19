#include "../include/Admin.h"
#include "../include/BankPolicy.h"
#include "../include/Database.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <iomanip>

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
        std::cout << "\n┌─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─┐" << std::endl;
        std::cout << "│                                   │" << std::endl;
        std::cout << "│           Admin Portal            │" << std::endl;
        std::cout << "│                                   │" << std::endl;
        std::cout << "├───────────────────────────────────┤" << std::endl;
        std::cout << "│                                   │" << std::endl;
        std::cout << "│  1. Change Savings Interest Rate  │" << std::endl;
        std::cout << "│     (Current: " << std::fixed << std::setprecision(2) << BankPolicy::getSavingsInterestRate() * 100 << "%)";
        std::cout << std::string(8 - std::to_string(static_cast<int>(BankPolicy::getSavingsInterestRate() * 100)).length(), ' ') << "       │" << std::endl;
        std::cout << "│  2. Change Current Account Fee    │" << std::endl;
        std::cout << "│     (Current: $" << std::fixed << std::setprecision(2) << BankPolicy::getCurrentAccountFee() << ")";
        std::cout << std::string(11 - std::to_string(static_cast<int>(BankPolicy::getCurrentAccountFee())).length(), ' ') << "    │" << std::endl;
        std::cout << "│  3. Change Auditable Int. Rate    │" << std::endl;
        std::cout << "│     (Current: " << std::fixed << std::setprecision(2) << BankPolicy::getAuditableInterestRate() * 100 << "%)";
        std::cout << std::string(8 - std::to_string(static_cast<int>(BankPolicy::getAuditableInterestRate() * 100)).length(), ' ') << "       │" << std::endl;
        std::cout << "│  4. View All Accounts             │" << std::endl;
        std::cout << "│  5. Change Admin Password         │" << std::endl;
        std::cout << "│  6. Apply Monthly Update          │" << std::endl;
        std::cout << "│  7. Exit Admin Portal             │" << std::endl;
        std::cout << "│                                   │" << std::endl;
        std::cout << "└─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─x─┘" << std::endl;
        std::cout << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
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
                applyMonthlyUpdateToAllAccounts();
                break;
            case 7:
                std::cout << "Exiting Admin Portal.\n";
                break;
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 7);
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
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a valid number.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a valid number.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a valid number.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (rate < 0) {
        std::cout << "Interest rate cannot be negative.\n";
        return;
    }
    BankPolicy::setAuditableInterestRate(rate);
    std::cout << "Auditable account interest rate updated.\n";
}

void Admin::viewAllAccounts() {
    Database* db = Database::getInstance();
    std::cout << "\nAll Accounts\n";
    // Box-drawing characters
    const char* topLeft = "┌";
    const char* topRight = "┐";
    const char* bottomLeft = "└";
    const char* bottomRight = "┘";
    const char* horizontal = "─";
    const char* vertical = "│";
    const char* tDown = "┬";
    const char* tUp = "┴";
    const char* tRight = "├";
    const char* tLeft = "┤";
    const char* cross = "┼";
    // Column widths
    const int idWidth = 12, nameWidth = 15, accNumWidth = 10, typeWidth = 18, balWidth = 12;
    // Print top border
    std::cout << topLeft;
    for (int i = 0; i < idWidth; i++) std::cout << horizontal;
    std::cout << tDown;
    for (int i = 0; i < nameWidth; i++) std::cout << horizontal;
    std::cout << tDown;
    for (int i = 0; i < accNumWidth; i++) std::cout << horizontal;
    std::cout << tDown;
    for (int i = 0; i < typeWidth; i++) std::cout << horizontal;
    std::cout << tDown;
    for (int i = 0; i < balWidth; i++) std::cout << horizontal;
    std::cout << topRight << "\n";
    // Print header row
    std::cout << vertical << std::setw(idWidth) << std::left << "Customer ID"
              << vertical << std::setw(nameWidth) << std::left << "Name"
              << vertical << std::setw(accNumWidth) << std::left << "Account #"
              << vertical << std::setw(typeWidth) << std::left << "Type"
              << vertical << std::setw(balWidth) << std::left << "Balance"
              << vertical << "\n";
    // Print separator
    std::cout << tRight;
    for (int i = 0; i < idWidth; i++) std::cout << horizontal;
    std::cout << cross;
    for (int i = 0; i < nameWidth; i++) std::cout << horizontal;
    std::cout << cross;
    for (int i = 0; i < accNumWidth; i++) std::cout << horizontal;
    std::cout << cross;
    for (int i = 0; i < typeWidth; i++) std::cout << horizontal;
    std::cout << cross;
    for (int i = 0; i < balWidth; i++) std::cout << horizontal;
    std::cout << tLeft << "\n";
    for (const auto& [id, customerPtr] : db->getCustomers()) {
        bool first = true;
        for (const auto& accPtr : customerPtr->getAccounts()) {
            std::cout << vertical << std::setw(idWidth) << std::left << (first ? std::to_string(id) : "")
                      << vertical << std::setw(nameWidth) << std::left << (first ? customerPtr->getName() : "")
                      << vertical << std::setw(accNumWidth) << std::left << accPtr->getAccountNumber()
                      << vertical << std::setw(typeWidth) << std::left << accPtr->getTypeString()
                      << vertical << std::setw(balWidth) << std::fixed << std::setprecision(2) << accPtr->getBalance()
                      << vertical << "\n";
            first = false;
        }
        if (first) { // Customer with no accounts
            std::cout << vertical << std::setw(idWidth) << std::left << id
                      << vertical << std::setw(nameWidth) << std::left << customerPtr->getName()
                      << vertical << std::setw(accNumWidth) << "-"
                      << vertical << std::setw(typeWidth) << "-"
                      << vertical << std::setw(balWidth) << "-"
                      << vertical << "\n";
        }
        // Print row separator after each customer
        std::cout << tRight;
        for (int i = 0; i < idWidth; i++) std::cout << horizontal;
        std::cout << cross;
        for (int i = 0; i < nameWidth; i++) std::cout << horizontal;
        std::cout << cross;
        for (int i = 0; i < accNumWidth; i++) std::cout << horizontal;
        std::cout << cross;
        for (int i = 0; i < typeWidth; i++) std::cout << horizontal;
        std::cout << cross;
        for (int i = 0; i < balWidth; i++) std::cout << horizontal;
        std::cout << tLeft << "\n";
    }
    // Print bottom border
    std::cout << bottomLeft;
    for (int i = 0; i < idWidth; i++) std::cout << horizontal;
    std::cout << tUp;
    for (int i = 0; i < nameWidth; i++) std::cout << horizontal;
    std::cout << tUp;
    for (int i = 0; i < accNumWidth; i++) std::cout << horizontal;
    std::cout << tUp;
    for (int i = 0; i < typeWidth; i++) std::cout << horizontal;
    std::cout << tUp;
    for (int i = 0; i < balWidth; i++) std::cout << horizontal;
    std::cout << bottomRight << "\n";
}

void Admin::applyMonthlyUpdateToAllAccounts() {
    Database* db = Database::getInstance();
    std::cout << "\nApplying monthly updates to all accounts...\n";
    int updated = 0, failed = 0;
    for (const auto& [id, customerPtr] : db->getCustomers()) {
        for (const auto& accPtr : customerPtr->getAccounts()) {
            try {
                accPtr->applyMonthlyUpdate();
                updated++;
            } catch (const std::exception& e) {
                std::cout << "Failed to update account #" << accPtr->getAccountNumber() << ": " << e.what() << std::endl;
                failed++;
            }
        }
    }
    std::cout << "Monthly updates applied to " << updated << " accounts.";
    if (failed > 0) std::cout << " (" << failed << " failed)";
    std::cout << std::endl;
} 