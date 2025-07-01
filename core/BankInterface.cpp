#include "BankInterface.h"
#include "Database.h"
#include "Admin.h"
#include "BankPolicy.h"
#include "Account.h"
#include "Transaction.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <memory>
#include <cctype>

// For now, return a stub JSON for each function
#define NOT_IMPLEMENTED(name) \
    return std::string("{\"success\":false,\"message\":\"" name " not implemented\"}");

// User operations
std::string BankInterface::login(const std::string& username, const std::string& password) {
    int customerId = -1;
    bool ok = Database::getInstance()->authenticate(username, password, customerId);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Login successful" : "Invalid username or password");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::registerUser(const std::string& username, const std::string& password, const std::string& name, const std::string& phone) {
    if (Database::getInstance()->usernameExists(username)) {
        return R"({"success":false,"message":"Username already exists"})";
    }
    int newId = Database::getInstance()->getNextCustomerId();
    Database::getInstance()->incrementCustomerId();
    auto customer = std::make_unique<Customer>(newId, name, phone);
    bool ok = Database::getInstance()->addCustomer(std::move(customer), username, password);
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Registration successful" : "Registration failed");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    int customerId = Database::getInstance()->getCustomerIdByUsername(username);
    bool ok = false;
    if (customerId != -1) {
        ok = Database::getInstance()->changePassword(customerId, oldPassword, newPassword);
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Password changed successfully" : "Password change failed");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::getUserDetails(const std::string& username) {
    Database* db = Database::getInstance();
    int customerId = db->getCustomerIdByUsername(username);
    if (customerId == -1) {
        return R"({"success":false,"message":"User not found"})";
    }
    const auto& customers = db->getCustomers();
    auto it = customers.find(customerId);
    if (it == customers.end()) {
        return R"({"success":false,"message":"User details not found in map"})";
    }
    const auto& customer = it->second;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("user");
    writer.StartObject();
    writer.Key("id"); writer.Int(customer->getId());
    writer.Key("name"); writer.String(customer->getName().c_str());
    writer.Key("phone"); writer.String(customer->getPhone().c_str());
    writer.Key("username"); writer.String(username.c_str());
    writer.EndObject();
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::listAccounts(const std::string& username) {
    if (!Database::getInstance()->usernameExists(username)) {
        return R"({"success":false,"message":"User not found"})";
    }
    int customerId = Database::getInstance()->getCustomerIdByUsername(username);
    std::vector<Account*> userAccounts = Database::getInstance()->getAccountsByCustomerId(customerId);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("accounts"); writer.StartArray();
    for (auto* acc : userAccounts) {
        writer.StartObject();
        writer.Key("accountNumber"); writer.Int(acc->getAccountNumber());
        writer.Key("type"); writer.Int(static_cast<int>(acc->getType()));
        writer.Key("balance"); writer.Double(acc->getBalance());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::createAccount(const std::string& username, const std::string& type, double initialBalance, const std::string& pin) {
    if (pin.length() != 4 || !std::all_of(pin.begin(), pin.end(), ::isdigit)) {
        return R"({"success":false,"message":"PIN must be exactly 4 digits"})";
    }
    int customerId = Database::getInstance()->getCustomerIdByUsername(username);
    if (customerId == -1) {
        return R"({"success":false,"message":"User not found"})";
    }
    std::unique_ptr<Account> newAccount;
    if (type == "SAVINGS") {
        newAccount = Database::getInstance()->createSavingsAccount(customerId, 0);
    } else if (type == "CURRENT") {
        newAccount = Database::getInstance()->createCurrentAccount(customerId, 0);
    } else if (type == "AUDITABLE_SAVINGS") {
        newAccount = Database::getInstance()->createAuditableSavingsAccount(customerId, 0);
    } else {
        return R"({"success":false,"message":"Invalid account type"})";
    }
    int accountNumber = newAccount->getAccountNumber();
    Database::getInstance()->addAccount(std::move(newAccount), pin);
    Account* acc = Database::getInstance()->findAccount(accountNumber);
    if (acc && initialBalance > 0) {
        auto deposit = std::make_unique<Deposit>(acc, initialBalance);
        if (deposit->execute()) {
            Database::getInstance()->addTransaction(accountNumber, std::move(deposit));
        }
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("accountNumber"); writer.Int(accountNumber);
    writer.Key("message"); writer.String("Account created successfully");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::getAccountDetails(int accountId) {
    Account* acc = Database::getInstance()->findAccount(accountId);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    if (acc) {
        writer.Key("success"); writer.Bool(true);
        writer.Key("account");
        writer.StartObject();
        writer.Key("accountNumber"); writer.Int(acc->getAccountNumber());
        writer.Key("type"); writer.Int(static_cast<int>(acc->getType()));
        writer.Key("balance"); writer.Double(acc->getBalance());
        writer.EndObject();
        writer.Key("message"); writer.String("Account found");
    } else {
        writer.Key("success"); writer.Bool(false);
        writer.Key("message"); writer.String("Account not found");
    }
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::deposit(int accountId, double amount) {
    Account* acc = Database::getInstance()->findAccount(accountId);
    bool ok = false;
    if (acc) {
        auto deposit = std::make_unique<Deposit>(acc, amount);
        if (deposit->execute()) {
            ok = Database::getInstance()->addTransaction(accountId, std::move(deposit));
        }
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Deposit successful" : "Deposit failed");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::withdraw(int accountId, double amount, const std::string& pin) {
    Account* acc = Database::getInstance()->findAccount(accountId);
    bool ok = false;
    if (acc) {
        if (!Database::verifyPassword(accountId, pin)) {
            return R"({"success":false,"message":"Invalid PIN"})";
        }
        auto withdrawal = std::make_unique<Withdrawal>(acc, amount);
        if (withdrawal->execute()) {
            ok = Database::getInstance()->addTransaction(accountId, std::move(withdrawal));
        }
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Withdraw successful" : "Withdraw failed");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::transfer(int accountId, int toAccount, double amount, const std::string& pin) {
    Account* fromAcc = Database::getInstance()->findAccount(accountId);
    Account* toAcc = Database::getInstance()->findAccount(toAccount);
    bool ok = false;
    if (fromAcc && toAcc) {
        if (!Database::verifyPassword(accountId, pin)) {
            return R"({"success":false,"message":"Invalid PIN"})";
        }
        auto transfer = std::make_unique<Transfer>(fromAcc, toAcc, amount);
        if (transfer->execute()) {
            ok = Database::getInstance()->addTransaction(accountId, std::move(transfer));
        }
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Transfer successful" : "Transfer failed");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::getTransactions(int accountId) {
    Account* acc = Database::getInstance()->findAccount(accountId);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    if (!acc) {
        writer.Key("success"); writer.Bool(false);
        writer.Key("message"); writer.String("Account not found");
    } else {
        writer.Key("success"); writer.Bool(true);
        struct TransactionData {
            long long timestamp;
            int type;
            double amount;
            int fromAccount;
            int toAccount;
            double balanceAfter;
        };
        std::vector<TransactionData> transactions;
        std::ifstream file(Database::getInstance()->getTransactionFilePath());
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string acc1_str, ts_str, type_str, amt_str, acc2_str;
            std::getline(ss, acc1_str, ':');
            std::getline(ss, ts_str, ':');
            std::getline(ss, type_str, ':');
            std::getline(ss, amt_str, ':');
            if (acc1_str.empty() || ts_str.empty() || type_str.empty() || amt_str.empty()) continue;
            int acc1_id = std::stoi(acc1_str);
            if (acc1_id != accountId) continue;
            struct tm tm = {};
            std::stringstream ts_ss(ts_str);
            ts_ss >> std::get_time(&tm, "%Y-%m-%d %H-%M-%S");
            time_t epoch_time = mktime(&tm);
            int type = std::stoi(type_str);
            double amount = std::stod(amt_str);
            int from_acc = 0;
            int to_acc = 0;
            if (type == 1) { // Withdrawal
                amount = -amount;
            }
            if (type == 2) { // Transfer
                std::getline(ss, acc2_str, ':');
                if (acc2_str.empty()) continue;
                if(amount < 0) {
                    from_acc = acc1_id;
                    to_acc = std::stoi(acc2_str);
                }
                else{
                    from_acc = std::stoi(acc2_str);
                    to_acc = acc1_id;
                }
            }
            transactions.push_back({(long long)epoch_time, type, amount, from_acc, to_acc, 0.0});
        }
        std::sort(transactions.begin(), transactions.end(), [](const auto& a, const auto& b) {
            return a.timestamp > b.timestamp;
        });
        double running_balance = acc->getBalance();
        for (auto& t : transactions) {
            t.balanceAfter = running_balance;
            running_balance -= t.amount;
        }
        std::sort(transactions.begin(), transactions.end(), [](const auto& a, const auto& b) {
            return a.timestamp < b.timestamp;
        });
        writer.Key("transactions");
        writer.StartArray();
        for (const auto& t : transactions) {
            writer.StartObject();
            writer.Key("timestamp"); writer.Int64(t.timestamp);
            writer.Key("type"); writer.Int(t.type);
            writer.Key("amount"); writer.Double(t.amount);
            writer.Key("fromAccount"); writer.Int(t.fromAccount);
            writer.Key("toAccount"); writer.Int(t.toAccount);
            writer.Key("balanceAfter"); writer.Double(t.balanceAfter);
            writer.EndObject();
        }
        writer.EndArray();
    }
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::closeAccount(int accountId, const std::string& pin) {
    if (!Database::verifyPassword(accountId, pin)) {
        return R"({"success":false,"message":"Invalid PIN"})";
    }
    bool ok = Database::getInstance()->removeAccount(accountId);
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Account closed" : "Failed to close account");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::logout() {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("message"); writer.String("Logged out successfully");
    writer.EndObject();
    return sb.GetString();
}

// Admin operations
std::string BankInterface::adminLogin(const std::string& username, const std::string& password) {
    bool ok = Admin::authenticate(username, password);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Admin login successful" : "Invalid admin credentials");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::adminChangePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    if (username.empty()) {
        return R"({"success":false,"message":"Username cannot be empty"})";
    }
    bool ok = false;
    if (Admin::authenticate(username, oldPassword)) {
        Admin::setAdminPassword(newPassword);
        ok = true;
    }
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(ok);
    writer.Key("message"); writer.String(ok ? "Admin password changed" : "Failed to change admin password");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::getPolicy(const std::string& username, const std::string& password) {
    if (!Admin::authenticate(username, password)) {
        return R"({"success":false,"message":"Admin authentication required"})";
    }
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("policy");
    writer.StartObject();
    writer.Key("savingsInterestRate"); writer.Double(BankPolicy::getSavingsInterestRate());
    writer.Key("currentAccountFee"); writer.Double(BankPolicy::getCurrentAccountFee());
    writer.Key("auditableInterestRate"); writer.Double(BankPolicy::getAuditableInterestRate());
    writer.EndObject();
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::setPolicy(const std::string& username, const std::string& password, double savingsInterestRate, double currentAccountFee, double auditableInterestRate) {
    if (!Admin::authenticate(username, password)) {
        return R"({"success":false,"message":"Admin authentication required"})";
    }
    BankPolicy::setSavingsInterestRate(savingsInterestRate);
    BankPolicy::setCurrentAccountFee(currentAccountFee);
    BankPolicy::setAuditableInterestRate(auditableInterestRate);
    BankPolicy::saveToFile("data/policy.txt");
    Database::getInstance()->savePolicy();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("message"); writer.String("Policy updated successfully");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::monthlyUpdate(const std::string& username, const std::string& password) {
    if (!Admin::authenticate(username, password)) {
        return R"({"success":false,"message":"Admin authentication required"})";
    }
    Admin::applyMonthlyUpdateToAllAccounts();
    Database::getInstance()->saveAll();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("message"); writer.String("Monthly updates applied successfully");
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::listUsers(const std::string& adminUsername, const std::string& adminPassword) {
    if (!Admin::authenticate(adminUsername, adminPassword)) {
        return R"({"success":false,"message":"Admin authentication required"})";
    }
    const auto& customers = Database::getInstance()->getCustomers();
    const auto& usernameMap = Database::getInstance()->getUsernameToCustomerId();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("users"); writer.StartArray();
    for (const auto& [id, cust] : customers) {
        writer.StartObject();
        writer.Key("id"); writer.Int(id);
        writer.Key("name"); writer.String(cust->getName().c_str());
        writer.Key("phone"); writer.String(cust->getPhone().c_str());
        writer.Key("username");
        for (const auto& [uname, cid] : usernameMap) {
            if (cid == id) {
                writer.String(uname.c_str());
                break;
            }
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}

std::string BankInterface::listAllAccounts(const std::string& adminUsername, const std::string& adminPassword) {
    if (!Admin::authenticate(adminUsername, adminPassword)) {
        return R"({"success":false,"message":"Admin authentication required"})";
    }
    const auto& allAccounts = Database::getInstance()->getAccounts();
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key("success"); writer.Bool(true);
    writer.Key("accounts"); writer.StartArray();
    for (const auto& [accNum, accPtr] : allAccounts) {
        if (accPtr) {
            writer.StartObject();
            writer.Key("accountNumber"); writer.Int(accNum);
            writer.Key("type"); writer.Int(static_cast<int>(accPtr->getType()));
            writer.Key("balance"); writer.Double(accPtr->getBalance());
            writer.Key("ownerId"); writer.Int(accPtr->getOwner()->getId());
            writer.EndObject();
        }
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
} 