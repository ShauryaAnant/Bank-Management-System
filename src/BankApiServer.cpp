#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <iostream>
#include <pistache/http_headers.h>
#include <pistache/serializer/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include "../core/Database.h"
#include "../core/Admin.h"
#include "../core/BankPolicy.h"
#include "../core/Account.h"
#include "../core/Transaction.h"

using namespace Pistache;

class CorsMiddleware : public Http::Handler {
public:
    HTTP_PROTOTYPE(CorsMiddleware)

    explicit CorsMiddleware(std::shared_ptr<Rest::Router> router) : router_(std::move(router)) {}

    void onRequest(const Http::Request& request, Http::ResponseWriter response) override {
        // Set CORS headers
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::AccessControlAllowMethods>("GET, POST, PUT, DELETE, OPTIONS");
        response.headers().add<Http::Header::AccessControlAllowHeaders>("Content-Type, Authorization");

        // Handle preflight requests
        if (request.method() == Http::Method::Options) {
            response.send(Http::Code::Ok);
            return;
        }

        router_->route(request, std::move(response));
    }

private:
    std::shared_ptr<Rest::Router> router_;
};

class BankApi {
public:
    explicit BankApi(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t threads = 1) {
        auto opts = Http::Endpoint::options().threads(static_cast<int>(threads));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        auto router_ptr = std::make_shared<Rest::Router>(router);
        httpEndpoint->setHandler(std::make_shared<CorsMiddleware>(router_ptr));
        httpEndpoint->serve();
    }

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/ping", Routes::bind(&BankApi::pingHandler, this));
        Routes::Post(router, "/login", Routes::bind(&BankApi::loginHandler, this));
        Routes::Post(router, "/register", Routes::bind(&BankApi::registerHandler, this));
        Routes::Post(router, "/change-password", Routes::bind(&BankApi::changePasswordHandler, this));
        Routes::Post(router, "/accounts/:id/deposit", Routes::bind(&BankApi::depositHandler, this));
        Routes::Post(router, "/accounts/:id/withdraw", Routes::bind(&BankApi::withdrawHandler, this));
        Routes::Post(router, "/accounts/:id/transfer", Routes::bind(&BankApi::transferHandler, this));
        Routes::Post(router, "/accounts/:id/close", Routes::bind(&BankApi::closeAccountHandler, this));
        Routes::Get(router, "/accounts/:id/transactions", Routes::bind(&BankApi::getTransactionsHandler, this));
        Routes::Get(router, "/accounts/:id", Routes::bind(&BankApi::getAccountDetailsHandler, this));
        Routes::Get(router, "/accounts", Routes::bind(&BankApi::listAccountsHandler, this));
        Routes::Post(router, "/accounts", Routes::bind(&BankApi::createAccountHandler, this));
        Routes::Post(router, "/admin/login", Routes::bind(&BankApi::adminLoginHandler, this));
        Routes::Post(router, "/admin/change-password", Routes::bind(&BankApi::adminChangePasswordHandler, this));
        Routes::Get(router, "/admin/policy", Routes::bind(&BankApi::getPolicyHandler, this));
        Routes::Post(router, "/admin/policy", Routes::bind(&BankApi::setPolicyHandler, this));
        Routes::Post(router, "/admin/monthly-update", Routes::bind(&BankApi::monthlyUpdateHandler, this));
        Routes::Get(router, "/users", Routes::bind(&BankApi::listUsersHandler, this));
        Routes::Get(router, "/all-accounts", Routes::bind(&BankApi::listAllAccountsHandler, this));
        Routes::Post(router, "/logout", Routes::bind(&BankApi::logoutHandler, this));
        Routes::Get(router, "/user/details", Routes::bind(&BankApi::getUserDetailsHandler, this));
        Routes::Get(router, "/me", Routes::bind(&BankApi::meHandler, this));
    }

    void logRequest(const Pistache::Rest::Request& req, const std::string& response_body, Pistache::Http::Code code) {
        std::cout << "[API] " << req.method() << " " << req.resource() 
                  << " - " << static_cast<int>(code) 
                  << " " << response_body << std::endl;
    }

    void pingHandler(const Rest::Request& request, Http::ResponseWriter response) {
        logRequest(request, "Bank API is alive!", Http::Code::Ok);
        response.send(Http::Code::Ok, "Bank API is alive!");
    }

    void loginHandler(const Rest::Request& request, Http::ResponseWriter response) {
        // Parse JSON body
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("password")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string password = doc["password"].GetString();
        int customerId = -1;
        bool ok = Database::getInstance()->authenticate(username, password, customerId);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success");
        writer.Bool(ok);
        writer.Key("message");
        if (ok) {
            writer.String("Login successful");
        } else {
            writer.String("Invalid username or password");
        }
        writer.EndObject();
        
        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Unauthorized);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Unauthorized, responseBody);
    }

    void registerHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("password") || !doc.HasMember("name") || !doc.HasMember("phone")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string password = doc["password"].GetString();
        std::string name = doc["name"].GetString();
        std::string phone = doc["phone"].GetString();
        if (Database::getInstance()->usernameExists(username)) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Username already exists"})");
            return;
        }
        int newId = Database::getInstance()->getNextCustomerId();
        Database::getInstance()->incrementCustomerId();
        auto customer = std::make_unique<Customer>(newId, name, phone);
        bool ok = Database::getInstance()->addCustomer(std::move(customer), username, password);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Registration successful" : "Registration failed");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void changePasswordHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("oldPassword") || !doc.HasMember("newPassword")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string oldPassword = doc["oldPassword"].GetString();
        std::string newPassword = doc["newPassword"].GetString();
        int customerId = Database::getInstance()->getCustomerIdByUsername(username);
        bool ok = false;
        if (customerId != -1) {
            ok = Database::getInstance()->changePassword(customerId, oldPassword, newPassword);
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Password changed successfully" : "Password change failed");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void listAccountsHandler(const Rest::Request& request, Http::ResponseWriter response) {
        auto username = request.query().get("username");
        if (!username) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Missing username parameter"})");
            return;
        }
        int customerId = -1;
        // Authenticate user by username (no password for now, for demo)
        if (!Database::getInstance()->usernameExists(*username)) {
            response.send(Http::Code::Not_Found, R"({"success":false,"message":"User not found"})");
            return;
        }
        customerId = Database::getInstance()->getCustomerIdByUsername(*username);
        // Get accounts for this customer
        std::vector<Account*> userAccounts = Database::getInstance()->getAccountsByCustomerId(customerId);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success");
        writer.Bool(true);
        writer.Key("accounts");
        writer.StartArray();
        for (auto* acc : userAccounts) {
            writer.StartObject();
            writer.Key("accountNumber"); writer.Int(acc->getAccountNumber());
            writer.Key("type"); writer.Int(static_cast<int>(acc->getType()));
            writer.Key("balance"); writer.Double(acc->getBalance());
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void createAccountHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("type") || !doc.HasMember("initialBalance") || !doc.HasMember("pin")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON - missing required fields"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string type = doc["type"].GetString();
        double initialBalance = doc["initialBalance"].GetDouble();
        std::string pin = doc["pin"].GetString();
        
        // Validate PIN format (4 digits)
        if (pin.length() != 4 || !std::all_of(pin.begin(), pin.end(), ::isdigit)) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"PIN must be exactly 4 digits"})");
            return;
        }
        
        int customerId = Database::getInstance()->getCustomerIdByUsername(username);
        if (customerId == -1) {
            response.send(Http::Code::Not_Found, R"({"success":false,"message":"User not found"})");
            return;
        }
        std::unique_ptr<Account> newAccount;
        if (type == "SAVINGS") {
            newAccount = Database::getInstance()->createSavingsAccount(customerId, 0);
        } else if (type == "CURRENT") {
            newAccount = Database::getInstance()->createCurrentAccount(customerId, 0);
        } else if (type == "AUDITABLE_SAVINGS") {
            newAccount = Database::getInstance()->createAuditableSavingsAccount(customerId, 0);
        } else {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid account type"})");
            return;
        }
        int accountNumber = newAccount->getAccountNumber();
        Database::getInstance()->addAccount(std::move(newAccount), pin); // Store PIN instead of empty password
        // Record initial deposit as a transaction
        Account* acc = Database::getInstance()->findAccount(accountNumber);
        if (acc && initialBalance > 0) {
            auto deposit = std::make_unique<Deposit>(acc, initialBalance);
            if (deposit->execute()) {
                Database::getInstance()->addTransaction(accountNumber, std::move(deposit));
            }
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(true);
        writer.Key("accountNumber"); writer.Int(accountNumber);
        writer.Key("message"); writer.String("Account created successfully");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
        Database::getInstance()->saveAll();
    }

    void getAccountDetailsHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
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

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, acc ? Http::Code::Ok : Http::Code::Not_Found);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(acc ? Http::Code::Ok : Http::Code::Not_Found, responseBody);
    }

    void depositHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("amount")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON - missing amount"})");
            return;
        }
        double amount = doc["amount"].GetDouble();
        
        Account* acc = Database::getInstance()->findAccount(accountId);
        bool ok = false;
        if (acc) {
            auto deposit = std::make_unique<Deposit>(acc, amount);
            if (deposit->execute()) {
                ok = Database::getInstance()->addTransaction(accountId, std::move(deposit));
            }
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Deposit successful" : "Deposit failed");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void withdrawHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("amount") || !doc.HasMember("pin")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON - missing amount or PIN"})");
            return;
        }
        double amount = doc["amount"].GetDouble();
        std::string pin = doc["pin"].GetString();
        
        Account* acc = Database::getInstance()->findAccount(accountId);
        bool ok = false;
        if (acc) {
            // Verify PIN before allowing transaction
            if (!Database::verifyPassword(accountId, pin)) {
                response.send(Http::Code::Unauthorized, R"({"success":false,"message":"Invalid PIN"})");
                return;
            }
            
            auto withdrawal = std::make_unique<Withdrawal>(acc, amount);
            if (withdrawal->execute()) {
                ok = Database::getInstance()->addTransaction(accountId, std::move(withdrawal));
            }
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Withdraw successful" : "Withdraw failed");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void transferHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("toAccount") || !doc.HasMember("amount") || !doc.HasMember("pin")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON - missing required fields"})");
            return;
        }
        int toAccount = doc["toAccount"].GetInt();
        double amount = doc["amount"].GetDouble();
        std::string pin = doc["pin"].GetString();
        
        Account* fromAcc = Database::getInstance()->findAccount(accountId);
        Account* toAcc = Database::getInstance()->findAccount(toAccount);
        bool ok = false;
        if (fromAcc && toAcc) {
            // Verify PIN before allowing transaction
            if (!Database::verifyPassword(accountId, pin)) {
                response.send(Http::Code::Unauthorized, R"({"success":false,"message":"Invalid PIN"})");
                return;
            }
            
            auto transfer = std::make_unique<Transfer>(fromAcc, toAcc, amount);
            if (transfer->execute()) {
                ok = Database::getInstance()->addTransaction(accountId, std::move(transfer));
            }
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Transfer successful" : "Transfer failed");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void getTransactionsHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
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
  
        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);
  
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void listUsersHandler(const Rest::Request& request, Http::ResponseWriter response) {
        auto username = request.query().get("admin_username");
        auto password = request.query().get("admin_password");
        if (!username || !password || !Admin::authenticate(*username, *password)) {
            response.send(Http::Code::Unauthorized, R"({\"success\":false,\"message\":\"Admin authentication required\"})");
            return;
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

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void listAllAccountsHandler(const Rest::Request& request, Http::ResponseWriter response) {
        auto username = request.query().get("admin_username");
        auto password = request.query().get("admin_password");
        if (!username || !password || !Admin::authenticate(*username, *password)) {
            response.send(Http::Code::Unauthorized, R"({\"success\":false,\"message\":\"Admin authentication required\"})");
            return;
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

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void logoutHandler(const Rest::Request& request, Http::ResponseWriter response) {
        (void)request;
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(true);
        writer.Key("message"); writer.String("Logged out successfully");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void meHandler(const Rest::Request& request, Http::ResponseWriter response) {
        // This is a placeholder for a session-based "me" endpoint
        // For now, it might return a generic message
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(false);
        writer.Key("message"); writer.String("No active session");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Unauthorized);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Unauthorized, responseBody);
    }

    void closeAccountHandler(const Rest::Request& request, Http::ResponseWriter response) {
        int accountId = request.param(":id").as<int>();
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("pin")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid JSON - PIN required"})");
            return;
        }
        std::string pin = doc["pin"].GetString();
        
        // Verify PIN before allowing account closure
        if (!Database::verifyPassword(accountId, pin)) {
            response.send(Http::Code::Unauthorized, R"({"success":false,"message":"Invalid PIN"})");
            return;
        }
        
        bool ok = Database::getInstance()->removeAccount(accountId);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Account closed" : "Failed to close account");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void adminLoginHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("password")) {
            response.send(Http::Code::Bad_Request, R"({\"success\":false,\"message\":\"Invalid JSON\"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string password = doc["password"].GetString();
        bool ok = Admin::authenticate(username, password);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Admin login successful" : "Invalid admin credentials");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Unauthorized);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Unauthorized, responseBody);
    }

    void adminChangePasswordHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc["username"].IsString() || !doc.HasMember("oldPassword") || !doc.HasMember("newPassword")) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Invalid or missing fields in JSON"})");
            return;
        }
        std::string username = doc["username"].GetString();
        if (username.empty()) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Username cannot be empty"})");
            return;
        }
        std::string oldPassword = doc["oldPassword"].GetString();
        std::string newPassword = doc["newPassword"].GetString();
        bool ok = false;
        if (Admin::authenticate(username, oldPassword)) {
            Admin::setAdminPassword(newPassword);
            ok = true;
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(ok);
        writer.Key("message"); writer.String(ok ? "Admin password changed" : "Failed to change admin password");
        writer.EndObject();
        
        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, ok ? Http::Code::Ok : Http::Code::Bad_Request);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request, responseBody);
        Database::getInstance()->saveAll();
    }

    void getPolicyHandler(const Rest::Request& request, Http::ResponseWriter response) {
        auto username = request.query().get("admin_username");
        auto password = request.query().get("admin_password");
        if (!username || !password || !Admin::authenticate(*username, *password)) {
            response.send(Http::Code::Unauthorized, R"({"success":false,"message":"Admin authentication required"})");
            return;
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

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    void setPolicyHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("password")) {
            response.send(Http::Code::Bad_Request, R"({\"success\":false,\"message\":\"Invalid JSON\"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string password = doc["password"].GetString();
        if (!Admin::authenticate(username, password)) {
            response.send(Http::Code::Unauthorized, R"({\"success\":false,\"message\":\"Admin authentication required\"})");
            return;
        }
        bool changed = false;
        if (doc.HasMember("savingsInterestRate") && doc["savingsInterestRate"].IsNumber()) {
            double rate = doc["savingsInterestRate"].GetDouble();
            BankPolicy::setSavingsInterestRate(rate);
            changed = true;
        }
        if (doc.HasMember("currentAccountFee") && doc["currentAccountFee"].IsNumber()) {
            double fee = doc["currentAccountFee"].GetDouble();
            BankPolicy::setCurrentAccountFee(fee);
            changed = true;
        }
        if (doc.HasMember("auditableInterestRate") && doc["auditableInterestRate"].IsNumber()) {
            double rate = doc["auditableInterestRate"].GetDouble();
            BankPolicy::setAuditableInterestRate(rate);
            changed = true;
        }
        if (changed) BankPolicy::saveToFile("data/policy.txt");
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(true);
        writer.Key("message"); writer.String("Policy updated successfully");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
        Database::getInstance()->savePolicy();
    }

    void monthlyUpdateHandler(const Rest::Request& request, Http::ResponseWriter response) {
        rapidjson::Document doc;
        if (doc.Parse(request.body().c_str()).HasParseError() || !doc.HasMember("username") || !doc.HasMember("password")) {
            response.send(Http::Code::Bad_Request, R"({\"success\":false,\"message\":\"Invalid JSON\"})");
            return;
        }
        std::string username = doc["username"].GetString();
        std::string password = doc["password"].GetString();
        if (!Admin::authenticate(username, password)) {
            response.send(Http::Code::Unauthorized, R"({\"success\":false,\"message\":\"Admin authentication required\"})");
            return;
        }
        Admin::applyMonthlyUpdateToAllAccounts();
        Database::getInstance()->saveAll();
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("success"); writer.Bool(true);
        writer.Key("message"); writer.String("Monthly updates applied successfully");
        writer.EndObject();

        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
        Database::getInstance()->saveAll();
    }

    void getUserDetailsHandler(const Rest::Request& request, Http::ResponseWriter response) {
        auto usernameParam = request.query().get("username");
        if (!usernameParam) {
            response.send(Http::Code::Bad_Request, R"({"success":false,"message":"Missing username parameter"})");
            return;
        }
        std::string username = *usernameParam;

        Database* db = Database::getInstance();
        int customerId = db->getCustomerIdByUsername(username);
        if (customerId == -1) {
            response.send(Http::Code::Not_Found, R"({"success":false,"message":"User not found"})");
            return;
        }

        const auto& customers = db->getCustomers();
        auto it = customers.find(customerId);
        if (it == customers.end()) {
            response.send(Http::Code::Not_Found, R"({"success":false,"message":"User details not found in map"})");
            return;
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
        
        std::string responseBody = sb.GetString();
        logRequest(request, responseBody, Http::Code::Ok);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseBody);
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main() {
    Port port(9080);
    Address addr(Ipv4::any(), port);
    BankApi api(addr);
    api.init(2); // 2 threads
    std::cout << "Bank API server running on port 9080..." << std::endl;
    api.start();
    return 0;
} 
