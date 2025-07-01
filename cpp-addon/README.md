# Bank C++ Native Addon

This directory contains the Node.js native addon that bridges the C++ bank backend with the Node.js/Express API server.

## Prerequisites
- Node.js (v14+ recommended)
- Python 3 (for node-gyp)
- C++ build tools (g++, make, or MSVC on Windows)
- `node-gyp` installed globally or locally

## Build Instructions

1. Install dependencies:
   ```sh
   npm install
   ```
2. Build the addon:
   ```sh
   npm run build
   ```
   This will produce `build/Release/bankaddon.node`.

## Usage

In your Node.js server:
```js
const bank = require('./build/Release/bankaddon.node');
// bank.login(...), bank.createAccount(...), etc.
```

## Exposed Methods
- login
- registerUser
- changePassword
- getUserDetails
- listAccounts
- createAccount
- getAccountDetails
- deposit
- withdraw
- transfer
- getTransactions
- closeAccount
- logout
- adminLogin
- adminChangePassword
- getPolicy
- setPolicy
- monthlyUpdate
- listUsers
- listAllAccounts 