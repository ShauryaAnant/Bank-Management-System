#include "../core/BankApp.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        BankApp* app = BankApp::getInstance("Sampatti Bank");
        app->run();
        delete BankApp::getInstance("Sampatti Bank");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 