#pragma once
#include <string>

class Admin {
public:
    static bool authenticate(const std::string& username, const std::string& password);
    static void showMenu();
    static void loadCredentials(const std::string& filename = "data/admin.txt");
    static void saveCredentials(const std::string& filename = "data/admin.txt");
    static void applyMonthlyUpdateToAllAccounts();
    static void setAdminPassword(const std::string& newPassword);
private:
    static std::string adminUsername;
    static std::string adminPassword;
    static void changeSavingsInterestRate();
    static void changeCurrentAccountFee();
    static void changeAuditableInterestRate();
    static void viewAllAccounts();
    static void changeAdminPassword();
}; 