#include "../include/Database.h"
#include "../include/SavingsAccount.h"
#include "../include/CurrentAccount.h"
#include "../include/AuditableSavingsAccount.h"
#include "../include/Transaction.h"
#include "../include/BankPolicy.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// Initialize static members
Database* Database::instance = nullptr;
std::mutex Database::mutex;

Database::Database(const std::string& dataDir) : dataDir(dataDir) {
    createDataDirectory();
    loadAll();
}

Database* Database::getInstance(const std::string& dataDir) {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new Database(dataDir);
    }
    return instance;
}

void Database::createDataDirectory() {
    try {
        std::filesystem::create_directories(dataDir);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Failed to create data directory: " + std::string(e.what()));
    }
}

std::string Database::getCustomerFilePath() const {
    return dataDir + "/customers.txt";
}

std::string Database::getAccountFilePath() const {
    return dataDir + "/accounts.txt";
}

std::string Database::getTransactionFilePath() const {
    return dataDir + "/transactions.txt";
}

std::string Database::getAuthFilePath() const {
    return dataDir + "/auth.txt";
}

std::string Database::getCounterFilePath() const {
    return dataDir + "/counters.txt";
}

bool Database::addCustomer(std::unique_ptr<Customer> customer, const std::string& username, const std::string& password) {
    if (usernameExists(username)) {
        return false;
    }
    
    try {
        // Save username and password
        // std::ofstream authFile(getAuthFilePath(), std::ios::app);
        // if (!authFile.is_open()) {
        //     throw std::runtime_error("Failed to open auth file for writing");
        // }
        
        // If customer is nullptr, we're just adding authentication data
        if (customer) {
            int customerId = customer->getId();
            usernameToCustomerId[username] = customerId;
            usernamePasswords[username] = password;
            customers[customerId] = std::move(customer);
            // saveCustomer(customers[customerId].get());
            // authFile << username << ":" << password << ":" << customerId << std::endl;
        } else {
            // Just save the authentication data
            // authFile << username << ":" << password << ":" << std::endl;
        }
            return true;
    } catch (const std::exception& e) {
        if (customer) {
            customers.erase(customer->getId());
            usernameToCustomerId.erase(username);
            usernamePasswords.erase(username);
        }
        throw std::runtime_error("Failed to save customer data: " + std::string(e.what()));
    }
}

Customer* Database::findCustomer(int customerId) const {
    auto it = customers.find(customerId);
    return it != customers.end() ? it->second.get() : nullptr;
}

bool Database::removeCustomer(int customerId) {
    auto it = customers.find(customerId);
    if (it == customers.end()) {
    return false;
}

    try {
        // Remove all associated accounts
        for (const auto& account : it->second->getAccounts()) {
            removeAccount(account->getAccountNumber());
        }
        
        // Remove from maps
        customers.erase(it);

        std::string username;
        
        // Remove from auth data
        for (auto it = usernameToCustomerId.begin(); it != usernameToCustomerId.end();) {
            if (it->second == customerId) {
                username = it->first;
                it = usernameToCustomerId.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = usernamePasswords.begin(); it != usernamePasswords.end();) {
            if (it->first == username) {
                it = usernamePasswords.erase(it);
            } else {
                ++it;
            }
        }
        
        // Save changes
        saveAll();
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to remove customer: " + std::string(e.what()));
    }
}

bool Database::addAccount(std::unique_ptr<Account> account, const std::string& password) {
    int accountNumber = account->getAccountNumber();
    try {
        accounts[accountNumber] = account.get();
        accountPasswords[accountNumber] = password;
        account->getOwner()->addAccount(std::move(account));
        // saveAccount(accounts[accountNumber]);
        return true;
    } catch (const std::exception& e) {
        accounts.erase(accountNumber);
        accountPasswords.erase(accountNumber);
        throw std::runtime_error("Failed to add account: " + std::string(e.what()));
    }
}

Account* Database::findAccount(int accountNumber) const {
    auto it = accounts.find(accountNumber);
    return it != accounts.end() ? it->second : nullptr;
}

bool Database::removeAccount(int accountNumber) {
    auto it = accounts.find(accountNumber);
    auto it2 = accountPasswords.find(accountNumber);
    if (it == accounts.end() || it2 == accountPasswords.end()) {
        return false;
    }

    try {
        // 1. Get the account and its owner before removing
        Account* account = it->second;
        Customer* owner = account->getOwner();
        
        // 2. Remove from customer's account list
        if (owner) {
            owner->removeAccount(accountNumber);
        }

        // 3. Remove from database maps
        accounts.erase(it);
        accountPasswords.erase(it2);

        // 4. Clean up transaction history
        std::ifstream transFile(getTransactionFilePath());
        std::ofstream tempFile(getTransactionFilePath() + ".tmp");
        std::string line;
        
        while (std::getline(transFile, line)) {
            std::stringstream ss(line);
            int accNum;
            ss >> accNum;
            if (accNum != accountNumber) {
                tempFile << line << std::endl;
            }
        }
        
        transFile.close();
        tempFile.close();
        std::filesystem::remove(getTransactionFilePath());
        std::filesystem::rename(getTransactionFilePath() + ".tmp", getTransactionFilePath());

        // 5. Clean up account file
        // std::ifstream accFile(getAccountFilePath());
        // std::ofstream accTempFile(getAccountFilePath() + ".tmp");
        
        // while (std::getline(accFile, line)) {
        //     std::stringstream ss(line);
        //     int accNum;
        //     ss >> accNum;
        //     if (accNum != accountNumber) {
        //         accTempFile << line << std::endl;
        //     }
        // }
        
        // accFile.close();
        // accTempFile.close();
        // std::filesystem::remove(getAccountFilePath());
        // std::filesystem::rename(getAccountFilePath() + ".tmp", getAccountFilePath());

        // 6. Clean up auth file
        // std::ifstream authFile(getAuthFilePath());
        // std::ofstream authTempFile(getAuthFilePath() + ".tmp");
        
        // while (std::getline(authFile, line)) {
        //     std::stringstream ss(line);
        //     std::string username, password;
        //     int accNum;
        //     ss >> username >> password >> accNum;
        //     if (accNum != accountNumber) {
        //         authTempFile << line << std::endl;
        //     }
        // }
        
        // authFile.close();
        // authTempFile.close();
        // std::filesystem::remove(getAuthFilePath());
        // std::filesystem::rename(getAuthFilePath() + ".tmp", getAuthFilePath());

        // 7. Save all remaining changes
        //saveAll();
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to remove account: " + std::string(e.what()));
    }
}

bool Database::addTransaction(int accountNumber, std::unique_ptr<ITransaction> transaction) {
    Account* account = findAccount(accountNumber);
    if (!account) {
        return false;
    }
    
    try {
        saveTransaction(account, transaction.get());      // Save before moving
        // account->addTransaction(std::move(transaction));  // Move after saving
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to add transaction: " + std::string(e.what()));
    }
}

void Database::getTransactions(int accountNumber, std::ostream& out) const {
    std::ifstream file(getTransactionFilePath());
    if (!file.is_open()) {
        out << "Error: Could not open transactions file." << std::endl;
        return;
    }

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
    const int dateWidth = 20;
    const int typeWidth = 15;
    const int amountWidth = 15;
    const int relatedWidth = 15;
    const int balanceWidth = 15;

    // Print header
    out << "\nAccount Statement for Account #" << accountNumber << "\n";
    
    // Print top border
    out << topLeft;
    for (int i = 0; i < dateWidth; i++) out << horizontal;
    out << tDown;
    for (int i = 0; i < typeWidth; i++) out << horizontal;
    out << tDown;
    for (int i = 0; i < amountWidth; i++) out << horizontal;
    out << tDown;
    for (int i = 0; i < relatedWidth; i++) out << horizontal;
    out << tDown;
    for (int i = 0; i < balanceWidth; i++) out << horizontal;
    out << topRight << "\n";

    // Print header row
    out << vertical << std::left << std::setw(dateWidth) << "Date & Time"
        << vertical << std::setw(typeWidth) << "Type"
        << vertical << std::setw(amountWidth) << "Amount"
        << vertical << std::setw(relatedWidth) << "Related Account"
        << vertical << std::setw(balanceWidth) << "Balance"
        << vertical << "\n";

    // Print separator
    out << tRight;
    for (int i = 0; i < dateWidth; i++) out << horizontal;
    out << cross;
    for (int i = 0; i < typeWidth; i++) out << horizontal;
    out << cross;
    for (int i = 0; i < amountWidth; i++) out << horizontal;
    out << cross;
    for (int i = 0; i < relatedWidth; i++) out << horizontal;
    out << cross;
    for (int i = 0; i < balanceWidth; i++) out << horizontal;
    out << tLeft << "\n";

    std::string line;
    double runningBalance = 0.0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string accNo, timestamp, type, amount, relatedAcc;
        
        // Parse the line
        std::getline(ss, accNo, ':');
        std::getline(ss, timestamp, ':');
        std::getline(ss, type, ':');
        std::getline(ss, amount, ':');
        std::getline(ss, relatedAcc, ':');

        // Check if this transaction belongs to our account
        if (std::stoi(accNo) == accountNumber) {
            double amountValue = std::stod(amount);
            std::string transactionType;
            
            // Determine transaction type and update balance
            switch (std::stoi(type)) {
                case 0: // Deposit
                    transactionType = "Deposit";
                    runningBalance += amountValue;
                    break;
                case 1: // Withdrawal
                    transactionType = "Withdrawal";
                    runningBalance -= amountValue;
                    break;
                case 2: // Transfer
                    if (amountValue > 0) {
                        transactionType = "Transfer In";
                        runningBalance += amountValue;
                        relatedAcc = "From " + relatedAcc;
                    } else {
                        transactionType = "Transfer Out";
                        runningBalance += amountValue; // amount is already negative
                        relatedAcc = "To " + relatedAcc;
                    }
                    break;
                default:
                    transactionType = "Unknown";
                    break;
            }

            // Print transaction details
            out << vertical << std::left 
                << std::setw(dateWidth) << timestamp 
                << vertical << std::setw(typeWidth) << transactionType 
                << vertical << std::setw(amountWidth) << std::fixed << std::setprecision(2) << std::abs(amountValue);
            
            // Print related account for transfers
            if (std::stoi(type) == 2) {
                out << vertical << std::setw(relatedWidth) << relatedAcc;
            } else {
                out << vertical << std::setw(relatedWidth) << "-";
            }
            
            out << vertical << std::setw(balanceWidth) << std::fixed << std::setprecision(2) << runningBalance 
                << vertical << "\n";
        }
    }

    // Print bottom border
    out << bottomLeft;
    for (int i = 0; i < dateWidth; i++) out << horizontal;
    out << tUp;
    for (int i = 0; i < typeWidth; i++) out << horizontal;
    out << tUp;
    for (int i = 0; i < amountWidth; i++) out << horizontal;
    out << tUp;
    for (int i = 0; i < relatedWidth; i++) out << horizontal;
    out << tUp;
    for (int i = 0; i < balanceWidth; i++) out << horizontal;
    out << bottomRight << "\n";

    file.close();
}

bool Database::authenticate(const std::string& username, const std::string& password, int& customerId) const {
    auto it = usernameToCustomerId.find(username);
    if (it == usernameToCustomerId.end()) {
        return false;
    }
    
    customerId = it->second;

    auto it2 = usernamePasswords.find(username);
    if (it2 == usernamePasswords.end()){
        return false;
    }

    return password == it2->second;
}

bool Database::changePassword(int customerId, const std::string& oldPassword, const std::string& newPassword) {
    try {
        std::ifstream file(getAuthFilePath());
        if (!file.is_open()) {
            return false;
        }
        
        std::vector<std::string> lines;
        std::string line;
        bool found = false;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string username, storedPassword, storedId;
            std::getline(ss, username, ':');
            std::getline(ss, storedPassword, ':');
            std::getline(ss, storedId);
            
            if (std::stoi(storedId) == customerId) {
                if (storedPassword != oldPassword) {
                    return false;
                }
                lines.push_back(username + ":" + newPassword + ":" + storedId);
                found = true;
            } else {
                lines.push_back(line);
            }
        }

        if (!found) {
            return false;
        }

        std::ofstream outFile(getAuthFilePath());
        for (const auto& l : lines) {
            outFile << l << std::endl;
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Database::usernameExists(const std::string& username) const {
    return usernameToCustomerId.find(username) != usernameToCustomerId.end();
}

void Database::saveAll() {
    try {
        saveCustomer(nullptr); // Save all customers
        saveAccount(nullptr);  // Save all accounts
        saveAuthData();
        saveCounters();
        savePolicy();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save all data: " + std::string(e.what()));
    }
}

void Database::loadAll() {
    try {
        loadCustomers();
        loadAccounts();
        // loadTransactions();
        loadAuthData();
        loadCounters();
        loadPolicy();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load data: " + std::string(e.what()));
    }
}

void Database::saveCustomer(const Customer* customer) {
    std::ofstream file(getCustomerFilePath());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open customer file for writing");
    }
    
    for (const auto& [id, cust] : customers) {
        if (!customer || cust.get() == customer) {
            file << id << ":" << cust->getName() << ":" << cust->getPhone() << std::endl;
        }
    }
}

void Database::saveAccount(const Account* account) {
    std::ofstream file(getAccountFilePath());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open account file for writing");
    }
    
    for (const auto& [number, acc] : accounts) {
        if (!account || acc == account) {
            file << number << ":" << acc->getBalance() << ":" 
                 << acc->getOwner()->getId() << ":" << static_cast<int>(acc->getType()) << std::endl;
        }
    }
}

void Database::saveTransaction(const Account* account, const ITransaction* transaction) {
    try {
        // Ensure the data directory exists
        createDataDirectory();
        
        // Open file in append mode
        std::ofstream file(getTransactionFilePath(), std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open transaction file for writing");
        }

        if (static_cast<int>(transaction->getType()) == 2) {
            saveTransferTransaction(transaction);
            return;
        }
        
        file << account->getAccountNumber() << ":" 
             << transaction->getTimestamp() << ":"
             << static_cast<int>(transaction->getType()) << ":"
             << transaction->getAmount() << std::endl;
             
        if (!file.good()) {
            throw std::runtime_error("Failed to write transaction data");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save transaction: " + std::string(e.what()));
    }
}

void Database::saveTransferTransaction(const ITransaction* transaction) {
    std::ofstream file(getTransactionFilePath(), std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open transaction file for writing");
    }
    const Transfer* transfer = dynamic_cast<const Transfer*>(transaction);

    //save in format accountNumber:timestamp:type:amount:toAccountNumber
    file << transfer->getFromAccount() << ":"
         << transaction->getTimestamp() << ":"
         << static_cast<int>(transaction->getType()) << ":"
         << (-1 * transaction->getAmount()) << ":"
         << transfer->getToAccount() << std::endl;

    file << transfer->getToAccount() << ":"
         << transaction->getTimestamp() << ":"
         << static_cast<int>(transaction->getType()) << ":"
         << transaction->getAmount() << ":"
         << transfer->getFromAccount() << std::endl;
}

void Database::loadCustomers() {
    std::ifstream file(getCustomerFilePath());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id, name, phone;
        std::getline(ss, id, ':');
        std::getline(ss, name, ':');
        std::getline(ss, phone);
        
        auto customer = std::make_unique<Customer>(std::stoi(id), name, phone);
        customers[customer->getId()] = std::move(customer);
    }
}

void Database::loadAccounts() {
    std::ifstream file(getAccountFilePath());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string number, balance, ownerId, type;
        std::getline(ss, number, ':');
        std::getline(ss, balance, ':');
        std::getline(ss, ownerId, ':');
        std::getline(ss, type);
        Customer* owner = findCustomer(std::stoi(ownerId));
        if (!owner) {
            continue;
        }
        std::unique_ptr<Account> account;
        switch (std::stoi(type)) {
            case static_cast<int>(AccountType::SAVINGS):
                account = std::make_unique<SavingsAccount>(
                    std::stoi(number),
                    std::stod(balance),
                    owner,
                    AccountType::SAVINGS
                );
                break;
            case static_cast<int>(AccountType::CURRENT):
                account = std::make_unique<CurrentAccount>(
                    std::stoi(number),
                    std::stod(balance),
                    owner
                );
                break;
            case static_cast<int>(AccountType::AUDITABLE_SAVINGS):
                account = std::make_unique<AuditableSavingsAccount>(
                    std::stoi(number),
                    std::stod(balance),
                    owner
                );
                break;
            default:
                continue;
        }
        owner->addAccount(std::move(account));
        Account* accountPtr = owner->findAccount(std::stoi(number));
        if (accountPtr) {
            accounts[accountPtr->getAccountNumber()] = accountPtr;
        }
    }
}

// void Database::loadTransactions() {
//     std::ifstream file(getTransactionFilePath());
//     if (!file.is_open()) {
//         return;
//     }

//     std::string line;
//     while (std::getline(file, line)) {
//         std::stringstream ss(line);
//         std::string accountNumber, timestamp, type, amount;
//         std::getline(ss, accountNumber, ':');
//         std::getline(ss, timestamp, ':');
//         std::getline(ss, type, ':');
//         std::getline(ss, amount);
        
//         Account* account = findAccount(std::stoi(accountNumber));
//         if (!account) {
//             continue;
//         }
        
//         std::unique_ptr<ITransaction> transaction;
//         int typeInt = std::stoi(type);
        
//         if (typeInt == static_cast<int>(TransactionType::DEPOSIT)) {
//             transaction = std::make_unique<Deposit>(account, std::stod(amount));
//         }
//         else if (typeInt == static_cast<int>(TransactionType::WITHDRAWAL)) {
//             transaction = std::make_unique<Withdrawal>(account, std::stod(amount));
//         }
//         else if (typeInt == static_cast<int>(TransactionType::TRANSFER)) {
//             std::string targetAccountStr = description.substr(description.find("to account ") + 11);
//             Account* toAccount = findAccount(std::stoi(targetAccountStr));
//             if (toAccount) {
//                 transaction = std::make_unique<Transfer>(account, toAccount, std::stod(amount));
//             }
//         }
        
//         if (transaction) {
//             account->addTransaction(std::move(transaction));
//         }
//     }
// }

void Database::saveAuthData() {
    std::ofstream file(getAuthFilePath());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open auth file for writing");
    }
    
    // Save customer auth data in form of customer :username : password  : customerId
    for (const auto& [username, customerId] : usernameToCustomerId) {
        file << "CUSTOMER:" << username << ":" << usernamePasswords[username] << ":" << customerId << std::endl;
    }
    
    // Save account authentication data
    for (const auto& [accountNumber, password] : accountPasswords) {
        file << "ACCOUNT:" << accountNumber << ":" << password << std::endl;
    }
}

void Database::loadAuthData() {
    std::ifstream file(getAuthFilePath());
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        //customer :username : password  : customerId
        std::stringstream ss(line);
        std::string type, username, password, customerId;
        std::getline(ss, type, ':');
        std::getline(ss, username, ':');
        
        if (type == "CUSTOMER") {
            std::getline(ss, password, ':');
            std::getline(ss, customerId);
            if (!username.empty() && !customerId.empty()) {
                usernameToCustomerId[username] = std::stoi(customerId);
                usernamePasswords[username] = password;  // Also store the password
            }
        } else if (type == "ACCOUNT") {
            std::getline(ss, password);
            accountPasswords[std::stoi(username)] = password;
        }
    }
}

bool Database::verifyPassword(int accountNumber, const std::string& password) {
    Database* db = getInstance();
    auto it = db->accountPasswords.find(accountNumber);
    if (it == db->accountPasswords.end()) {
    return false;
    }
    return it->second == password;
}

Account* Database::getAccount(int accountNumber) {
    Database* db = getInstance();
    return db->findAccount(accountNumber);
}

void Database::saveCounters() const {
    try {
        std::ofstream file(getCounterFilePath());
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open counter file for writing");
        }
        file << nextCustomerId << ":" << nextAccountNumber << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save counters: " + std::string(e.what()));
    }
}

void Database::loadCounters() {
    try {
        std::ifstream file(getCounterFilePath());
        if (!file.is_open()) {
            // If file doesn't exist, initialize with default values
            nextCustomerId = 1000;
            nextAccountNumber = 10000;
            return;
        }

        std::string line;
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string customerIdStr, accountNumberStr;
            if (std::getline(ss, customerIdStr, ':') && std::getline(ss, accountNumberStr)) {
                nextCustomerId = std::stoi(customerIdStr);
                nextAccountNumber = std::stoi(accountNumberStr);
            } else {
                throw std::runtime_error("Invalid counter file format");
            }
        } else {
            // If file is empty, initialize with default values
            nextCustomerId = 1000;
            nextAccountNumber = 10000;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load counters: " + std::string(e.what()));
    }
}

int Database::getNextCustomerId() {
    Database* db = getInstance();
    return db->nextCustomerId;
}

int Database::getNextAccountNumber() {
    Database* db = getInstance();
    return db->nextAccountNumber;
}

void Database::incrementCustomerId() {
    Database* db = getInstance();
    db->nextCustomerId++;
    db->saveCounters();
}

void Database::incrementAccountNumber() {
    Database* db = getInstance();
    db->nextAccountNumber++;
    db->saveCounters();
}


std::unique_ptr<Account> Database::createSavingsAccount(int customerId, double initialBalance) {
    Customer* customer = findCustomer(customerId);
    if (!customer) {
        throw std::runtime_error("Customer not found");
    }
    int accountNumber = getNextAccountNumber();
    incrementAccountNumber();
    return std::make_unique<SavingsAccount>(accountNumber, initialBalance, customer, AccountType::SAVINGS);
}

std::unique_ptr<Account> Database::createCurrentAccount(int customerId, double initialBalance) {
    Customer* customer = findCustomer(customerId);
    if (!customer) {
        throw std::runtime_error("Customer not found");
    }
    int accountNumber = getNextAccountNumber();
    incrementAccountNumber();
    return std::make_unique<CurrentAccount>(accountNumber, initialBalance, customer);
}

std::unique_ptr<Account> Database::createAuditableSavingsAccount(int customerId, double initialBalance) {
    Customer* customer = findCustomer(customerId);
    if (!customer) {
        throw std::runtime_error("Customer not found");
    }
    int accountNumber = getNextAccountNumber();
    incrementAccountNumber();
    return std::make_unique<AuditableSavingsAccount>(accountNumber, initialBalance, customer);
}

void Database::savePolicy() const {
    BankPolicy::saveToFile(dataDir + "/policy.txt");
}

void Database::loadPolicy() {
    BankPolicy::loadFromFile(dataDir + "/policy.txt");
}

const std::unordered_map<int, std::unique_ptr<Customer>>& Database::getCustomers() const {
    return customers;
}

Database::~Database() {
    try {
        saveAll();
    } catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << std::endl;
    }
} 