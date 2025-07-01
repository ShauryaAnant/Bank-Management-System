# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./core -I./include

# Project structure
SRC_DIR = src
CORE_DIR = core
OBJ_DIR = obj
BIN_DIR = bin

# Source files
CORE_SRCS = \
	core/Account.cpp \
	core/Admin.cpp \
	core/AuditableSavingsAccount.cpp \
	core/BankApp.cpp \
	core/BankPolicy.cpp \
	core/CurrentAccount.cpp \
	core/Customer.cpp \
	core/Database.cpp \
	core/SavingsAccount.cpp \
	core/Transaction.cpp \
	core/BankInterface.cpp
SRC_SRCS = $(wildcard $(SRC_DIR)/main.cpp)
SRCS = $(CORE_SRCS) $(SRC_SRCS)
OBJS = $(patsubst $(CORE_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(CORE_DIR)/%.cpp,$(SRCS))) \
        $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS)))
TARGET = $(BIN_DIR)/bank

# API server sources and target
API_SERVER_SRC = src/BankApiServer.cpp
API_SERVER_OBJS = $(patsubst $(CORE_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(CORE_DIR)/%.cpp,$(CORE_SRCS))) \
                  $(OBJ_DIR)/BankApiServer.o
API_SERVER_TARGET = $(BIN_DIR)/bank_api_server

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Compile core sources
$(OBJ_DIR)/%.o: $(CORE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile src sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build API server
$(API_SERVER_TARGET): $(API_SERVER_OBJS)
	$(CXX) $(API_SERVER_OBJS) -o $(API_SERVER_TARGET) -lpistache -lpthread

# Compile BankApiServer
$(OBJ_DIR)/BankApiServer.o: $(API_SERVER_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run
run: $(TARGET)
	./$(TARGET)

# API server run
api_server: $(API_SERVER_TARGET)
	./$(API_SERVER_TARGET)

# Phony targets
.PHONY: all clean run api_server 