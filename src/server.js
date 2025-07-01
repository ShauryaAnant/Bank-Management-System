const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const path = require('path');

// Load the C++ addon
const bank = require(path.join(__dirname, '../cpp-addon/build/Release/bankaddon.node'));

const app = express();
const PORT = 9080;

app.use(cors());
app.use(bodyParser.json());

// Log every API request
app.use((req, res, next) => {
  console.log(`[API] ${req.method} ${req.originalUrl}`);
  if (Object.keys(req.body || {}).length > 0) {
    console.log('  Body:', req.body);
  }
  if (Object.keys(req.query || {}).length > 0) {
    console.log('  Query:', req.query);
  }
  next();
});

// Utility to send JSON safely
function sendJson(res, result) {
  try {
    res.json(JSON.parse(result));
  } catch (e) {
    res.status(500).json({ success: false, message: 'Internal error', error: e.toString(), raw: result });
  }
}

// User routes
app.post('/login', (req, res) => {
  const { username, password } = req.body;
  if (!username || !password) return res.status(400).json({ success: false, message: 'Missing username or password' });
  sendJson(res, bank.login(username, password));
});

app.post('/register', (req, res) => {
  const { username, password, name, phone } = req.body;
  if (!username || !password || !name || !phone) return res.status(400).json({ success: false, message: 'Missing required fields' });
  sendJson(res, bank.registerUser(username, password, name, phone));
});

app.post('/change-password', (req, res) => {
  const { username, oldPassword, newPassword } = req.body;
  if (!username || !oldPassword || !newPassword) return res.status(400).json({ success: false, message: 'Missing required fields' });
  sendJson(res, bank.changePassword(username, oldPassword, newPassword));
});

app.get('/accounts', (req, res) => {
  const { username } = req.query;
  if (!username) return res.status(400).json({ success: false, message: 'Missing username parameter' });
  sendJson(res, bank.listAccounts(username));
});

app.post('/accounts', (req, res) => {
  const { username, type, initialBalance, pin } = req.body;
  if (!username || !type || typeof initialBalance !== 'number' || !pin) return res.status(400).json({ success: false, message: 'Missing required fields' });
  sendJson(res, bank.createAccount(username, type, initialBalance, pin));
});

app.get('/accounts/:id', (req, res) => {
  const accountId = parseInt(req.params.id);
  if (isNaN(accountId)) return res.status(400).json({ success: false, message: 'Invalid account id' });
  sendJson(res, bank.getAccountDetails(accountId));
});

app.post('/accounts/:id/deposit', (req, res) => {
  const accountId = parseInt(req.params.id);
  const { amount } = req.body;
  if (isNaN(accountId) || typeof amount !== 'number') return res.status(400).json({ success: false, message: 'Invalid input' });
  sendJson(res, bank.deposit(accountId, amount));
});

app.post('/accounts/:id/withdraw', (req, res) => {
  const accountId = parseInt(req.params.id);
  const { amount, pin } = req.body;
  if (isNaN(accountId) || typeof amount !== 'number' || !pin) return res.status(400).json({ success: false, message: 'Invalid input' });
  sendJson(res, bank.withdraw(accountId, amount, pin));
});

app.post('/accounts/:id/transfer', (req, res) => {
  const accountId = parseInt(req.params.id);
  const { toAccount, amount, pin } = req.body;
  if (isNaN(accountId) || isNaN(toAccount) || typeof amount !== 'number' || !pin) return res.status(400).json({ success: false, message: 'Invalid input' });
  sendJson(res, bank.transfer(accountId, toAccount, amount, pin));
});

app.get('/accounts/:id/transactions', (req, res) => {
  const accountId = parseInt(req.params.id);
  if (isNaN(accountId)) return res.status(400).json({ success: false, message: 'Invalid account id' });
  sendJson(res, bank.getTransactions(accountId));
});

app.post('/accounts/:id/close', (req, res) => {
  const accountId = parseInt(req.params.id);
  const { pin } = req.body;
  if (isNaN(accountId) || !pin) return res.status(400).json({ success: false, message: 'Invalid input' });
  sendJson(res, bank.closeAccount(accountId, pin));
});

app.post('/logout', (req, res) => {
  sendJson(res, bank.logout());
});

app.get('/user/details', (req, res) => {
  const { username } = req.query;
  if (!username) return res.status(400).json({ success: false, message: 'Missing username parameter' });
  sendJson(res, bank.getUserDetails(username));
});

// Admin routes
app.post('/admin/login', (req, res) => {
  const { username, password } = req.body;
  if (!username || !password) return res.status(400).json({ success: false, message: 'Missing admin credentials' });
  sendJson(res, bank.adminLogin(username, password));
});

app.post('/admin/change-password', (req, res) => {
  const { username, oldPassword, newPassword } = req.body;
  if (!username || !oldPassword || !newPassword) return res.status(400).json({ success: false, message: 'Missing required fields' });
  sendJson(res, bank.adminChangePassword(username, oldPassword, newPassword));
});

app.get('/admin/policy', (req, res) => {
  const { admin_username, admin_password } = req.query;
  if (!admin_username || !admin_password) return res.status(401).json({ success: false, message: 'Admin authentication required' });
  sendJson(res, bank.getPolicy(admin_username, admin_password));
});

app.post('/admin/policy', (req, res) => {
  const { username, password, savingsInterestRate, currentAccountFee, auditableInterestRate } = req.body;
  if (!username || !password) return res.status(401).json({ success: false, message: 'Admin authentication required' });
  sendJson(res, bank.setPolicy(username, password, savingsInterestRate, currentAccountFee, auditableInterestRate));
});

app.post('/admin/monthly-update', (req, res) => {
  const { username, password } = req.body;
  if (!username || !password) return res.status(401).json({ success: false, message: 'Admin authentication required' });
  sendJson(res, bank.monthlyUpdate(username, password));
});

app.get('/users', (req, res) => {
  const { admin_username, admin_password } = req.query;
  if (!admin_username || !admin_password) return res.status(401).json({ success: false, message: 'Admin authentication required' });
  sendJson(res, bank.listUsers(admin_username, admin_password));
});

app.get('/all-accounts', (req, res) => {
  const { admin_username, admin_password } = req.query;
  if (!admin_username || !admin_password) return res.status(401).json({ success: false, message: 'Admin authentication required' });
  sendJson(res, bank.listAllAccounts(admin_username, admin_password));
});

// Health check
app.get('/ping', (req, res) => {
  res.json({ success: true, message: 'Bank API is alive!' });
});

// Start server
app.listen(PORT, () => {
  console.log(`Bank API server running on port ${PORT}...`);
}); 