#include "node_modules/node-addon-api/napi.h"
#include "../core/BankInterface.h"

// Helper to wrap a C++ string result as a JS string
Napi::Value wrapResult(Napi::Env env, const std::string& result) {
    return Napi::String::New(env, result);
}

Napi::Value login(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::login(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Value registerUser(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 4 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString() || !info[3].IsString())
        Napi::TypeError::New(env, "Expected 4 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::registerUser(info[0].As<Napi::String>(), info[1].As<Napi::String>(), info[2].As<Napi::String>(), info[3].As<Napi::String>()));
}

Napi::Value changePassword(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString())
        Napi::TypeError::New(env, "Expected 3 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::changePassword(info[0].As<Napi::String>(), info[1].As<Napi::String>(), info[2].As<Napi::String>()));
}

Napi::Value getUserDetails(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsString())
        Napi::TypeError::New(env, "Expected 1 string argument").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::getUserDetails(info[0].As<Napi::String>()));
}

Napi::Value listAccounts(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsString())
        Napi::TypeError::New(env, "Expected 1 string argument").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::listAccounts(info[0].As<Napi::String>()));
}

Napi::Value createAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 4 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber() || !info[3].IsString())
        Napi::TypeError::New(env, "Expected (string, string, number, string)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::createAccount(info[0].As<Napi::String>(), info[1].As<Napi::String>(), info[2].As<Napi::Number>().DoubleValue(), info[3].As<Napi::String>()));
}

Napi::Value getAccountDetails(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsNumber())
        Napi::TypeError::New(env, "Expected 1 number argument").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::getAccountDetails(info[0].As<Napi::Number>().Int32Value()));
}

Napi::Value deposit(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsNumber())
        Napi::TypeError::New(env, "Expected (number, number)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::deposit(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().DoubleValue()));
}

Napi::Value withdraw(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsString())
        Napi::TypeError::New(env, "Expected (number, number, string)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::withdraw(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().DoubleValue(), info[2].As<Napi::String>()));
}

Napi::Value transfer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 4 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsString())
        Napi::TypeError::New(env, "Expected (number, number, number, string)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::transfer(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::Number>().Int32Value(), info[2].As<Napi::Number>().DoubleValue(), info[3].As<Napi::String>()));
}

Napi::Value getTransactions(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsNumber())
        Napi::TypeError::New(env, "Expected 1 number argument").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::getTransactions(info[0].As<Napi::Number>().Int32Value()));
}

Napi::Value closeAccount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected (number, string)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::closeAccount(info[0].As<Napi::Number>().Int32Value(), info[1].As<Napi::String>()));
}

Napi::Value logout(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return wrapResult(env, BankInterface::logout());
}

Napi::Value adminLogin(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::adminLogin(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Value adminChangePassword(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString())
        Napi::TypeError::New(env, "Expected 3 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::adminChangePassword(info[0].As<Napi::String>(), info[1].As<Napi::String>(), info[2].As<Napi::String>()));
}

Napi::Value getPolicy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::getPolicy(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Value setPolicy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 5 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber() || !info[3].IsNumber() || !info[4].IsNumber())
        Napi::TypeError::New(env, "Expected (string, string, number, number, number)").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::setPolicy(info[0].As<Napi::String>(), info[1].As<Napi::String>(), info[2].As<Napi::Number>().DoubleValue(), info[3].As<Napi::Number>().DoubleValue(), info[4].As<Napi::Number>().DoubleValue()));
}

Napi::Value monthlyUpdate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::monthlyUpdate(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Value listUsers(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::listUsers(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Value listAllAccounts(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString())
        Napi::TypeError::New(env, "Expected 2 string arguments").ThrowAsJavaScriptException();
    return wrapResult(env, BankInterface::listAllAccounts(info[0].As<Napi::String>(), info[1].As<Napi::String>()));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("login", Napi::Function::New(env, login));
    exports.Set("registerUser", Napi::Function::New(env, registerUser));
    exports.Set("changePassword", Napi::Function::New(env, changePassword));
    exports.Set("getUserDetails", Napi::Function::New(env, getUserDetails));
    exports.Set("listAccounts", Napi::Function::New(env, listAccounts));
    exports.Set("createAccount", Napi::Function::New(env, createAccount));
    exports.Set("getAccountDetails", Napi::Function::New(env, getAccountDetails));
    exports.Set("deposit", Napi::Function::New(env, deposit));
    exports.Set("withdraw", Napi::Function::New(env, withdraw));
    exports.Set("transfer", Napi::Function::New(env, transfer));
    exports.Set("getTransactions", Napi::Function::New(env, getTransactions));
    exports.Set("closeAccount", Napi::Function::New(env, closeAccount));
    exports.Set("logout", Napi::Function::New(env, logout));
    exports.Set("adminLogin", Napi::Function::New(env, adminLogin));
    exports.Set("adminChangePassword", Napi::Function::New(env, adminChangePassword));
    exports.Set("getPolicy", Napi::Function::New(env, getPolicy));
    exports.Set("setPolicy", Napi::Function::New(env, setPolicy));
    exports.Set("monthlyUpdate", Napi::Function::New(env, monthlyUpdate));
    exports.Set("listUsers", Napi::Function::New(env, listUsers));
    exports.Set("listAllAccounts", Napi::Function::New(env, listAllAccounts));
    return exports;
}

NODE_API_MODULE(bankaddon, Init) 