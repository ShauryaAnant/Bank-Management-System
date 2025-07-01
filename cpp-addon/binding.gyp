{
  "targets": [
    {
      "target_name": "bankaddon",
      "sources": [
        "binding.cpp",
        "../core/BankInterface.cpp",
        "../core/Admin.cpp",
        "../core/Database.cpp",
        "../core/Transaction.cpp",
        "../core/SavingsAccount.cpp",
        "../core/Customer.cpp",
        "../core/CurrentAccount.cpp",
        "../core/BankPolicy.cpp",
        "../core/BankApp.cpp",
        "../core/AuditableSavingsAccount.cpp",
        "../core/Account.cpp"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "../core"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags_cc": ["-std=c++17", "-fexceptions", "-frtti"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS=0"],
      "libraries": []
    }
  ]
} 