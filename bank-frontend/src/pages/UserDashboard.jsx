import React, { useEffect, useState } from 'react';
import { Box, Typography, Drawer, List, ListItem, ListItemIcon, ListItemText, Divider, Grid, Paper, Button, Stack, CircularProgress, Alert, Dialog, DialogTitle, DialogContent, DialogActions, TextField, Select, MenuItem, Snackbar, Table, TableBody, TableCell, TableContainer, TableHead, TableRow, ListItemButton } from '@mui/material';
import DashboardIcon from '@mui/icons-material/Dashboard';
import AddCircleIcon from '@mui/icons-material/AddCircle';
import LockIcon from '@mui/icons-material/Lock';
import LogoutIcon from '@mui/icons-material/Logout';
import AccountCircleIcon from '@mui/icons-material/AccountCircle';
import api from '../api';
import { useNavigate } from 'react-router-dom';
import sampattiLogo from '../assets/sampatti-logo.svg';

const drawerWidth = 220;

const navItems = [
  { text: 'Dashboard', icon: <DashboardIcon /> },
  { text: 'My Profile', icon: <AccountCircleIcon /> },
  { text: 'Change Password', icon: <LockIcon /> },
  { text: 'Logout', icon: <LogoutIcon /> },
];

const accountTypes = [
  { value: 'SAVINGS', label: 'Savings' },
  { value: 'CURRENT', label: 'Current' },
  { value: 'AUDITABLE_SAVINGS', label: 'Auditable Savings' },
];

const UserDashboard = () => {
  const [selected, setSelected] = useState(0);
  const [accounts, setAccounts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [createOpen, setCreateOpen] = useState(false);
  const [createType, setCreateType] = useState('SAVINGS');
  const [createBalance, setCreateBalance] = useState('');
  const [createPin, setCreatePin] = useState('');
  const [createLoading, setCreateLoading] = useState(false);
  const [createError, setCreateError] = useState('');
  const [snackbar, setSnackbar] = useState({ open: false, message: '', severity: 'success' });
  const navigate = useNavigate();
  // Dialog/action state
  const [actionOpen, setActionOpen] = useState(false);
  const [actionType, setActionType] = useState(''); // 'deposit' | 'withdraw' | 'transfer'
  const [actionAccount, setActionAccount] = useState(null);
  const [actionAmount, setActionAmount] = useState('');
  const [actionToAccount, setActionToAccount] = useState('');
  const [actionPin, setActionPin] = useState('');
  const [actionLoading, setActionLoading] = useState(false);
  const [actionError, setActionError] = useState('');
  // Statement
  const [statementOpen, setStatementOpen] = useState(false);
  const [statementAccount, setStatementAccount] = useState(null);
  const [statementData, setStatementData] = useState([]);
  const [statementLoading, setStatementLoading] = useState(false);
  // Close account
  const [closeOpen, setCloseOpen] = useState(false);
  const [closeAccount, setCloseAccount] = useState(null);
  const [closePin, setClosePin] = useState('');
  const [closeLoading, setCloseLoading] = useState(false);
  const [closeError, setCloseError] = useState('');
  // Change password
  const [pwOpen, setPwOpen] = useState(false);
  const [oldPw, setOldPw] = useState('');
  const [newPw, setNewPw] = useState('');
  const [pwLoading, setPwLoading] = useState(false);
  const [pwError, setPwError] = useState('');
  // Logout confirmation
  const [logoutOpen, setLogoutOpen] = useState(false);
  // Profile
  const [profileOpen, setProfileOpen] = useState(false);
  const [profileData, setProfileData] = useState(null);
  const [profileLoading, setProfileLoading] = useState(false);
  const [profileError, setProfileError] = useState('');

  const fetchAccounts = () => {
    const username = localStorage.getItem('username');
    if (!username) {
      setError('No user logged in.');
      setLoading(false);
      return;
    }
    setLoading(true);
    api.get(`/accounts?username=${encodeURIComponent(username)}`)
      .then(res => {
        if (res.data.success) {
          setAccounts(res.data.accounts || []);
        } else {
          setError(res.data.message || 'Failed to fetch accounts');
        }
      })
      .catch(err => {
        setError(err.response?.data?.message || 'Server error');
      })
      .finally(() => setLoading(false));
  };

  useEffect(() => {
    fetchAccounts();
    // eslint-disable-next-line
  }, []);

  // Create Account Handlers
  const handleCreateAccount = () => {
    setCreateOpen(true);
    setCreateType('SAVINGS');
    setCreateBalance('');
    setCreatePin('');
    setCreateError('');
  };
  const handleCreateClose = () => setCreateOpen(false);
  const handleCreateSubmit = async (e) => {
    e.preventDefault();
    setCreateError('');
    if (!createBalance || isNaN(createBalance) || Number(createBalance) < 0) {
      setCreateError('Enter a valid initial balance.');
      return;
    }
    if (!createPin || createPin.length !== 4 || !/^\d{4}$/.test(createPin)) {
      setCreateError('PIN must be exactly 4 digits.');
      return;
    }
    setCreateLoading(true);
    try {
      const username = localStorage.getItem('username');
      const res = await api.post('/accounts', {
        username,
        type: createType,
        initialBalance: Number(createBalance),
        pin: createPin,
      });
      if (res.data.success) {
        setSnackbar({ open: true, message: 'Account created!', severity: 'success' });
        setCreateOpen(false);
        fetchAccounts();
      } else {
        setCreateError(res.data.message || 'Failed to create account');
      }
    } catch (err) {
      setCreateError(err.response?.data?.message || 'Server error');
    } finally {
      setCreateLoading(false);
    }
  };

  // Account Actions
  const openAction = (type, acc, amount = '') => {
    setActionType(type);
    setActionAccount(acc);
    setActionAmount(amount);
    setActionToAccount('');
    setActionPin('');
    setActionError('');
    setActionOpen(true);
  };
  const closeAction = () => setActionOpen(false);
  const handleActionSubmit = async (e) => {
    e.preventDefault();
    setActionError('');
    if (!actionAmount || isNaN(actionAmount) || Number(actionAmount) <= 0) {
      setActionError('Enter a valid amount.');
      return;
    }
    // Only require PIN for withdraw and transfer operations
    if (actionType !== 'deposit' && (!actionPin || actionPin.length !== 4 || !/^\d{4}$/.test(actionPin))) {
      setActionError('PIN must be exactly 4 digits.');
      return;
    }
    setActionLoading(true);
    try {
      let url = '', data = {};
      if (actionType === 'deposit') {
        url = `/accounts/${actionAccount.accountNumber}/deposit`;
        data = { amount: Number(actionAmount) };
      } else if (actionType === 'withdraw') {
        url = `/accounts/${actionAccount.accountNumber}/withdraw`;
        data = { amount: Number(actionAmount), pin: actionPin };
      } else if (actionType === 'transfer') {
        if (!actionToAccount || isNaN(actionToAccount)) {
          setActionError('Enter a valid destination account number.');
          setActionLoading(false);
          return;
        }
        url = `/accounts/${actionAccount.accountNumber}/transfer`;
        data = { toAccount: Number(actionToAccount), amount: Number(actionAmount), pin: actionPin };
      }
      const res = await api.post(url, data);
      if (res.data.success) {
        setSnackbar({ open: true, message: `${actionType.charAt(0).toUpperCase() + actionType.slice(1)} successful!`, severity: 'success' });
        setActionOpen(false);
        fetchAccounts();
      } else {
        setActionError(res.data.message || 'Action failed');
      }
    } catch (err) {
      setActionError(err.response?.data?.message || 'Server error');
    } finally {
      setActionLoading(false);
    }
  };

  // Statement
  const openStatement = (acc) => {
    setStatementOpen(true);
    setStatementAccount(acc);
    setStatementData([]);
    setStatementLoading(true);
    api.get(`/accounts/${acc.accountNumber}/transactions`)
      .then(res => {
        if (res.data.success) {
          setStatementData(res.data.transactions || []);
        } else {
          setStatementData([]);
        }
      })
      .catch(() => setStatementData([]))
      .finally(() => setStatementLoading(false));
  };
  const closeStatement = () => setStatementOpen(false);

  // Close Account
  const openClose = (acc) => {
    setCloseOpen(true);
    setCloseAccount(acc);
    setClosePin('');
    setCloseError('');
  };
  const closeClose = () => setCloseOpen(false);
  const handleCloseAccount = async () => {
    if (!closePin || closePin.length !== 4 || !/^\d{4}$/.test(closePin)) {
      setCloseError('PIN must be exactly 4 digits.');
      return;
    }
    setCloseLoading(true);
    setCloseError('');
    try {
      const res = await api.post(`/accounts/${closeAccount.accountNumber}/close`, { pin: closePin });
      if (res.data.success) {
        setSnackbar({ open: true, message: 'Account closed!', severity: 'success' });
        setCloseOpen(false);
        fetchAccounts();
      } else {
        setCloseError(res.data.message || 'Failed to close account');
      }
    } catch (err) {
      setCloseError(err.response?.data?.message || 'Server error');
    } finally {
      setCloseLoading(false);
    }
  };

  // Change Password
  const openPw = () => {
    setPwOpen(true);
    setOldPw('');
    setNewPw('');
    setPwError('');
  };
  const closePw = () => setPwOpen(false);
  const handlePwSubmit = async (e) => {
    e.preventDefault();
    setPwError('');
    if (!oldPw || !newPw || newPw.length < 6) {
      setPwError('Enter valid old and new passwords (min 6 chars).');
      return;
    }
    setPwLoading(true);
    try {
      const username = localStorage.getItem('username');
      const res = await api.post('/change-password', { username, oldPassword: oldPw, newPassword: newPw });
      if (res.data.success) {
        setSnackbar({ open: true, message: 'Password changed!', severity: 'success' });
        setPwOpen(false);
      } else {
        setPwError(res.data.message || 'Failed to change password');
      }
    } catch (err) {
      setPwError(err.response?.data?.message || 'Server error');
    } finally {
      setPwLoading(false);
    }
  };

  // Profile
  const openProfileDialog = async () => {
    setProfileOpen(true);
    setProfileLoading(true);
    setProfileError('');
    try {
        const username = localStorage.getItem('username');
        if (!username) {
            setProfileError('Not logged in.');
            return;
        }
        const res = await api.get(`/user/details?username=${encodeURIComponent(username)}`);
        if (res.data.success) {
            setProfileData(res.data.user);
        } else {
            setProfileError(res.data.message || 'Failed to fetch profile.');
        }
    } catch (err) {
        setProfileError(err.response?.data?.message || 'Server error.');
    } finally {
        setProfileLoading(false);
    }
  };
  const closeProfileDialog = () => setProfileOpen(false);

  // Logout
  const handleLogout = () => {
    localStorage.removeItem('username');
    navigate('/');
  };

  const getTransactionTypeString = (typeStr) => {
    switch (String(typeStr)) {
      case '0': return 'Deposit';
      case '1': return 'Withdrawal';
      case '2': return 'Transfer';
      case '3': return 'Monthly Update';
      default: return 'Unknown';
    }
  };

  return (
    <Box sx={{ display: 'flex', minHeight: '100vh', bgcolor: '#f5f6fa' }}>
      {/* Sidebar */}
      <Drawer
        variant="permanent"
        sx={{
          width: drawerWidth,
          flexShrink: 0,
          [`& .MuiDrawer-paper`]: {
            width: drawerWidth,
            boxSizing: 'border-box',
            background: 'linear-gradient(180deg, #1976d2 30%, #00bfae 90%)',
            color: '#fff',
            border: 'none',
          },
        }}
      >
        <Box sx={{ p: 2, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
          <img src={sampattiLogo} alt="Sampatti Bank Logo" style={{ width: 40, height: 40, marginRight: 12 }} />
          <Typography variant="h6" fontWeight={700} color="#fff">Sampatti Bank</Typography>
        </Box>
        <Divider sx={{ bgcolor: 'rgba(255,255,255,0.2)' }} />
        <List sx={{ p: 1 }}>
          {navItems.map((item, index) => {
            const handleNavClick = () => {
              if (item.text === 'Dashboard') {
                setSelected(index);
              } else if (item.text === 'My Profile') {
                openProfileDialog();
              } else if (item.text === 'Change Password') {
                openPw();
              } else if (item.text === 'Logout') {
                setLogoutOpen(true);
              }
            };

            return (
              <ListItem key={item.text} disablePadding sx={{ my: 0.5 }}>
                <ListItemButton
                  selected={selected === index}
                  onClick={handleNavClick}
                  sx={{
                    borderRadius: 2,
                    '&.Mui-selected': {
                      backgroundColor: 'rgba(255, 255, 255, 0.2)',
                      '&:hover': {
                        backgroundColor: 'rgba(255, 255, 255, 0.25)',
                      },
                    },
                    '&:hover': {
                      backgroundColor: 'rgba(255, 255, 255, 0.1)',
                    },
                  }}
                >
                  <ListItemIcon sx={{ color: '#fff', minWidth: 40 }}>{item.icon}</ListItemIcon>
                  <ListItemText primary={item.text} />
                </ListItemButton>
              </ListItem>
            );
          })}
        </List>
      </Drawer>
      {/* Main Content */}
      <Box component="main" sx={{ flexGrow: 1, p: 3, width: { sm: `calc(100% - ${drawerWidth}px)` } }}>
        <Typography variant="h4" gutterBottom>Your Accounts</Typography>
        {loading && <CircularProgress />}
        {error && <Alert severity="error">{error}</Alert>}
        {!loading && !error && (
          <>
            <Button variant="contained" color="primary" sx={{ mb: 3 }} onClick={handleCreateAccount}>
              + Create New Account
            </Button>
            <Grid container spacing={3}>
              {accounts.map(acc => (
                <Grid item xs={12} sm={6} md={4} key={acc.accountNumber}>
                  <Paper elevation={2} sx={{ p: 2, textAlign: 'center' }}>
                    <Typography variant="overline">Account #{acc.accountNumber}</Typography>
                    <Typography variant="h6" color="primary.main">{acc.type === 0 ? 'Savings' : acc.type === 1 ? 'Current' : 'Auditable Savings'}</Typography>
                    <Typography variant="h4" color="text.primary" mb={2}>${acc.balance.toLocaleString()}</Typography>
                    <Stack direction="row" spacing={1} mt={2}>
                      <Button size="small" variant="outlined" onClick={() => openAction('deposit', acc)}>Deposit</Button>
                      <Button size="small" variant="outlined" onClick={() => openAction('withdraw', acc)}>Withdraw</Button>
                      <Button size="small" variant="outlined" onClick={() => openAction('transfer', acc)}>Transfer</Button>
                    </Stack>
                    <Stack direction="row" spacing={1} mt={1}>
                      <Button size="small" variant="text" onClick={() => openStatement(acc)}>Statement</Button>
                      <Button size="small" variant="text" color="error" onClick={() => openClose(acc)}>Close</Button>
                    </Stack>
                  </Paper>
                </Grid>
              ))}
            </Grid>
          </>
        )}
        {/* Create Account Dialog */}
        <Dialog open={createOpen} onClose={handleCreateClose}>
          <DialogTitle>Create New Account</DialogTitle>
          <DialogContent>
            {createError && <Alert severity="error" sx={{ mb: 2 }}>{createError}</Alert>}
            <Stack component="form" onSubmit={handleCreateSubmit} spacing={2} sx={{ pt: 1 }}>
              <Select
                value={createType}
                onChange={(e) => setCreateType(e.target.value)}
                fullWidth
              >
                {accountTypes.map((opt) => (
                  <MenuItem key={opt.value} value={opt.value}>{opt.label}</MenuItem>
                ))}
              </Select>
              <TextField
                label="Initial Balance"
                type="number"
                value={createBalance}
                onChange={(e) => setCreateBalance(e.target.value)}
                fullWidth
                required
              />
              <TextField
                label="PIN"
                type="password"
                value={createPin}
                onChange={(e) => {
                  const value = e.target.value.replace(/\D/g, '').slice(0, 4);
                  setCreatePin(value);
                }}
                fullWidth
                required
                helperText="Enter a 4-digit PIN for account security"
                inputProps={{ maxLength: 4, pattern: '[0-9]*' }}
              />
              <DialogActions>
                <Button onClick={handleCreateClose}>Cancel</Button>
                <Button type="submit" variant="contained" disabled={createLoading}>
                  {createLoading ? <CircularProgress size={24} /> : 'Create'}
                </Button>
              </DialogActions>
            </Stack>
          </DialogContent>
        </Dialog>
        {/* Account Actions (Deposit, Withdraw, Transfer) */}
        <Dialog open={actionOpen} onClose={closeAction}>
          <DialogTitle>{actionType.charAt(0).toUpperCase() + actionType.slice(1)}</DialogTitle>
          <DialogContent>
            {actionError && <Alert severity="error" sx={{ mb: 2 }}>{actionError}</Alert>}
            <Stack component="form" onSubmit={handleActionSubmit} spacing={2} sx={{ pt: 1, minWidth: 400 }}>
              <Typography>Account: {actionAccount?.accountNumber}</Typography>
              <TextField
                label="Amount"
                type="number"
                value={actionAmount}
                onChange={(e) => setActionAmount(e.target.value)}
                fullWidth
                required
                autoFocus
              />
              {actionType === 'transfer' && (
                <TextField
                  label="To Account Number"
                  type="text"
                  value={actionToAccount}
                  onChange={(e) => setActionToAccount(e.target.value)}
                  fullWidth
                  required
                />
              )}
              {actionType !== 'deposit' && (
                <TextField
                  label="PIN"
                  type="password"
                  value={actionPin}
                  onChange={(e) => {
                    const value = e.target.value.replace(/\D/g, '').slice(0, 4);
                    setActionPin(value);
                  }}
                  fullWidth
                  required
                  helperText="Enter your 4-digit account PIN"
                  inputProps={{ maxLength: 4, pattern: '[0-9]*' }}
                />
              )}
              <DialogActions>
                <Button onClick={closeAction}>Cancel</Button>
                <Button type="submit" variant="contained" disabled={actionLoading}>
                  {actionLoading ? <CircularProgress size={24} /> : 'Submit'}
                </Button>
              </DialogActions>
            </Stack>
          </DialogContent>
        </Dialog>
        {/* Statement */}
        <Dialog open={statementOpen} onClose={closeStatement} maxWidth="md" fullWidth>
          <DialogTitle>Account Statement for #{statementAccount?.accountNumber}</DialogTitle>
          <DialogContent>
            {statementLoading ? <CircularProgress /> : (
              <TableContainer component={Paper} sx={{ mt: 2 }}>
                <Table aria-label="account statement table">
                  <TableHead>
                    <TableRow>
                      <TableCell sx={{ fontWeight: 'bold' }}>Timestamp</TableCell>
                      <TableCell sx={{ fontWeight: 'bold' }}>Type</TableCell>
                      <TableCell sx={{ fontWeight: 'bold' }} align="right">Amount</TableCell>
                      <TableCell sx={{ fontWeight: 'bold' }}>From</TableCell>
                      <TableCell sx={{ fontWeight: 'bold' }}>To</TableCell>
                      <TableCell sx={{ fontWeight: 'bold' }} align="right">Balance</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {statementData.length > 0 ? statementData.map((t, index) => {
                      const isCredit = parseFloat(t.amount) >= 0;
                      return (
                        <TableRow key={index}>
                          <TableCell>{new Date(t.timestamp * 1000).toLocaleString()}</TableCell>
                          <TableCell>{getTransactionTypeString(t.type)}</TableCell>
                          <TableCell align="right" style={{ color: isCredit ? 'green' : 'red' }}>
                            {isCredit ? '+' : '-'}${Math.abs(t.amount).toFixed(2)}
                          </TableCell>
                          <TableCell>{t.fromAccount !== 0 ? t.fromAccount : '-'}</TableCell>
                          <TableCell>{t.toAccount !== 0 ? t.toAccount : '-'}</TableCell>
                          <TableCell align="right">${t.balanceAfter.toFixed(2)}</TableCell>
                        </TableRow>
                      );
                    }) : (
                      <TableRow>
                        <TableCell colSpan={6} align="center">No transactions found.</TableCell>
                      </TableRow>
                    )}
                  </TableBody>
                </Table>
              </TableContainer>
            )}
          </DialogContent>
          <DialogActions>
            <Button onClick={closeStatement}>Close</Button>
          </DialogActions>
        </Dialog>
        {/* Close Account */}
        <Dialog open={closeOpen} onClose={closeClose}>
          <DialogTitle>Close Account #{closeAccount?.accountNumber}</DialogTitle>
          <DialogContent>
            {closeAccount?.balance > 0 ? (
              <>
                <Typography>This account has a balance of ${closeAccount.balance.toFixed(2)}.</Typography>
                <Typography>To close this account, the balance must be zero. Please withdraw or transfer the funds first.</Typography>
              </>
            ) : (
              <>
                <Typography>Are you sure you want to close this account? This action is irreversible.</Typography>
                {closeError && <Alert severity="error" sx={{ mt: 2 }}>{closeError}</Alert>}
                <TextField
                  label="PIN"
                  type="password"
                  value={closePin}
                  onChange={(e) => {
                    const value = e.target.value.replace(/\D/g, '').slice(0, 4);
                    setClosePin(value);
                  }}
                  fullWidth
                  required
                  helperText="Enter your 4-digit account PIN to confirm closure"
                  inputProps={{ maxLength: 4, pattern: '[0-9]*' }}
                  sx={{ mt: 2 }}
                />
              </>
            )}
          </DialogContent>
          <DialogActions>
            <Button onClick={closeClose}>Cancel</Button>
            {closeAccount?.balance > 0 ? (
              <Stack direction="row" spacing={1}>
                <Button variant="outlined" onClick={() => {
                  closeClose();
                  openAction('withdraw', closeAccount, closeAccount.balance);
                }}>Withdraw</Button>
                <Button variant="outlined" onClick={() => {
                  closeClose();
                  openAction('transfer', closeAccount, closeAccount.balance);
                }}>Transfer</Button>
              </Stack>
            ) : (
            <Button onClick={handleCloseAccount} color="error" disabled={closeLoading}>
              {closeLoading ? <CircularProgress size={24} /> : 'Confirm Close'}
            </Button>
            )}
          </DialogActions>
        </Dialog>
        {/* Change Password */}
        <Dialog open={pwOpen} onClose={closePw}>
          <DialogTitle>Change Password</DialogTitle>
          <DialogContent>
            {pwError && <Alert severity="error" sx={{ mb: 2 }}>{pwError}</Alert>}
            <Stack component="form" onSubmit={handlePwSubmit} spacing={2} sx={{ pt: 1 }}>
              <TextField
                label="Old Password"
                type="password"
                value={oldPw}
                onChange={(e) => setOldPw(e.target.value)}
                fullWidth
                required
              />
              <TextField
                label="New Password"
                type="password"
                value={newPw}
                onChange={(e) => setNewPw(e.target.value)}
                fullWidth
                required
              />
              <DialogActions>
                <Button onClick={closePw}>Cancel</Button>
                <Button type="submit" variant="contained" disabled={pwLoading}>
                  {pwLoading ? <CircularProgress size={24} /> : 'Change'}
                </Button>
              </DialogActions>
            </Stack>
          </DialogContent>
        </Dialog>
        {/* Profile Dialog */}
        <Dialog open={profileOpen} onClose={closeProfileDialog}>
            <DialogTitle>My Profile</DialogTitle>
            <DialogContent>
                {profileLoading ? <CircularProgress /> :
                profileError ? <Alert severity="error">{profileError}</Alert> :
                profileData && (
                    <Stack spacing={1} sx={{pt: 1, minWidth: 300}}>
                        <Typography><strong>Username:</strong> {profileData.username}</Typography>
                        <Divider />
                        <Typography><strong>Full Name:</strong> {profileData.name}</Typography>
                         <Divider />
                        <Typography><strong>Phone:</strong> {profileData.phone}</Typography>
                    </Stack>
                )
                }
            </DialogContent>
            <DialogActions>
                <Button onClick={closeProfileDialog}>Close</Button>
            </DialogActions>
        </Dialog>
        {/* Logout Confirmation */}
        <Dialog open={logoutOpen} onClose={() => setLogoutOpen(false)}>
          <DialogTitle>Logout</DialogTitle>
          <DialogContent>
            <Typography>Are you sure you want to log out?</Typography>
          </DialogContent>
          <DialogActions>
            <Button onClick={() => setLogoutOpen(false)}>Cancel</Button>
            <Button onClick={handleLogout} color="primary">
              Logout
            </Button>
          </DialogActions>
        </Dialog>
        {/* Snackbar for notifications */}
        <Snackbar
          open={snackbar.open}
          autoHideDuration={6000}
          onClose={() => setSnackbar({ ...snackbar, open: false })}
          anchorOrigin={{ vertical: 'top', horizontal: 'center' }}
        >
          <Alert onClose={() => setSnackbar({ ...snackbar, open: false })} severity={snackbar.severity} sx={{ width: '100%' }}>
            {snackbar.message}
          </Alert>
        </Snackbar>
      </Box>
    </Box>
  );
};

export default UserDashboard; 