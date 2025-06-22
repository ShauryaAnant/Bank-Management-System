import React, { useState, useEffect } from 'react';
import { Box, Drawer, List, ListItem, ListItemText, ListItemIcon, Typography, Paper, Grid, CircularProgress, Alert, Dialog, DialogTitle, DialogContent, DialogActions, Button, TextField, Snackbar, TableContainer, Table, TableHead, TableRow, TableCell, TableBody, Divider, Stack, InputAdornment, ListItemButton } from '@mui/material';
import DashboardIcon from '@mui/icons-material/Dashboard';
import LockIcon from '@mui/icons-material/Lock';
import ExitToAppIcon from '@mui/icons-material/ExitToApp';
import PolicyIcon from '@mui/icons-material/Policy';
import UpdateIcon from '@mui/icons-material/Update';
import api from '../api';
import { useNavigate } from 'react-router-dom';
import sampattiLogo from '../assets/sampatti-logo.svg';

const drawerWidth = 240;

const navItems = [
  { text: 'Dashboard', icon: <DashboardIcon /> },
  { text: 'Policy Management', icon: <PolicyIcon /> },
  { text: 'Monthly Update', icon: <UpdateIcon /> },
  { text: 'Change Password', icon: <LockIcon /> },
  { text: 'Logout', icon: <ExitToAppIcon /> },
];

const AdminDashboard = () => {
  const [selected, setSelected] = useState(0);
  const [stats, setStats] = useState({
    users: 0,
    totalAccounts: 0,
    savingsAccounts: 0,
    currentAccounts: 0,
    auditableSavingsAccounts: 0,
    totalBalance: 0
  });
  const [users, setUsers] = useState([]);
  const [accounts, setAccounts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  
  // Dialog states
  const [pwOpen, setPwOpen] = useState(false);
  const [logoutOpen, setLogoutOpen] = useState(false);
  const [policyOpen, setPolicyOpen] = useState(false);
  const [muOpen, setMuOpen] = useState(false);
  
  // Change Password state
  const [oldPw, setOldPw] = useState('');
  const [newPw, setNewPw] = useState('');
  const [pwLoading, setPwLoading] = useState(false);
  const [pwError, setPwError] = useState('');
  
  // Policy state
  const [policy, setPolicy] = useState({});
  const [policyLoading, setPolicyLoading] = useState(false);
  const [policyError, setPolicyError] = useState('');
  
  // Monthly Update state
  const [muLoading, setMuLoading] = useState(false);
  const [muError, setMuError] = useState('');
  
  const [snackbar, setSnackbar] = useState({ open: false, message: '', severity: 'success' });
  const navigate = useNavigate();

  const getAdminCredentials = () => {
    const username = localStorage.getItem('admin_username');
    const password = localStorage.getItem('admin_password');
    if (!username || !password) {
      navigate('/admin/login');
      return null;
    }
    return { username, password };
  }

  const fetchData = async () => {
    setLoading(true);
    setError('');
    
    const creds = getAdminCredentials();
    if (!creds) {
      setLoading(false);
      return;
    }

    try {
      const apiParams = { params: { admin_username: creds.username, admin_password: creds.password } };
      const usersRes = await api.get('/users', apiParams);
      const accountsRes = await api.get('/all-accounts', apiParams);
      
      if (usersRes.data.success) setUsers(usersRes.data.users || []);
      if (accountsRes.data.success) setAccounts(accountsRes.data.accounts || []);

      if (usersRes.data.success && accountsRes.data.success) {
        const accs = accountsRes.data.accounts || [];
        setStats({
          users: (usersRes.data.users || []).length,
          totalAccounts: accs.length,
          savingsAccounts: accs.filter(a => a.type === 0).length,
          currentAccounts: accs.filter(a => a.type === 1).length,
          auditableSavingsAccounts: accs.filter(a => a.type === 2).length,
          totalBalance: accs.reduce((sum, acc) => sum + acc.balance, 0),
        });
      }
    } catch (err) {
      setError('Failed to fetch dashboard data.');
      if (err.response && err.response.status === 401) {
        setError('Authentication failed. Please log in again.');
        navigate('/admin/login');
      }
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const openPolicyDialog = async () => {
    setPolicyOpen(true);
    setPolicyLoading(true);
    setPolicyError('');

    const creds = getAdminCredentials();
    if (!creds) return;
    
    try {
      const res = await api.get('/admin/policy', { 
        params: { admin_username: creds.username, admin_password: creds.password }
      });
      if (res.data.success) {
        setPolicy(res.data.policy);
      } else {
        setPolicyError('Failed to fetch policy.');
      }
    } catch (err) {
      setPolicyError(err.response?.data?.message || 'Server error.');
    } finally {
      setPolicyLoading(false);
    }
  };

  const handlePolicySubmit = async (e) => {
    e.preventDefault();
    setPolicyLoading(true);
    setPolicyError('');

    const creds = getAdminCredentials();
    if (!creds) return;

    try {
      console.log('Submitting policy:', policy);
      const res = await api.post('/admin/policy', { 
        ...policy, 
        username: creds.username, 
        password: creds.password,
      });
      console.log('Policy response:', res.data);
      if (res.data.success) {
        setSnackbar({ open: true, message: 'Policy updated successfully!', severity: 'success' });
        setPolicyOpen(false);
      } else {
        setPolicyError(res.data.message || 'Failed to update policy.');
      }
    } catch (err) {
      console.error('Policy submission error:', err);
      setPolicyError(err.response?.data?.message || 'Server error');
    } finally {
      setPolicyLoading(false);
    }
  };
  
  const handleMonthlyUpdate = async () => {
    setMuLoading(true);
    setMuError('');

    const creds = getAdminCredentials();
    if (!creds) return;

    try {
      const res = await api.post('/admin/monthly-update', { 
        username: creds.username, 
        password: creds.password
      });
      if (res.data.success) {
        setSnackbar({ open: true, message: 'Monthly update applied!', severity: 'success' });
        setMuOpen(false);
        fetchData(); // Refresh data after update
      } else {
        setMuError(res.data.message || 'Failed to apply update.');
      }
    } catch (err) {
      setMuError(err.response?.data?.message || 'Server error');
    } finally {
      setMuLoading(false);
    }
  };

  const handlePwSubmit = async (e) => {
    e.preventDefault();
    setPwLoading(true);
    setPwError('');

    const creds = getAdminCredentials();
    if (!creds) return;

    try {
      const res = await api.post('/admin/change-password', {
        username: creds.username,
        oldPassword: oldPw,
        newPassword: newPw,
      });
      if (res.data.success) {
        localStorage.setItem('admin_password', newPw);
        setSnackbar({ open: true, message: 'Password changed successfully!', severity: 'success' });
        setPwOpen(false);
      } else {
        setPwError(res.data.message || 'Failed to change password.');
      }
    } catch (err) {
      setPwError(err.response?.data?.message || 'Server error');
    } finally {
      setPwLoading(false);
    }
  };

  const handleLogout = () => {
    localStorage.removeItem('admin_username');
    localStorage.removeItem('admin_password');
    navigate('/');
  };
  
  return (
    <Box sx={{ display: 'flex' }}>
      <Drawer
        variant="permanent"
        sx={{
          width: drawerWidth,
          flexShrink: 0,
          [`& .MuiDrawer-paper`]: {
            width: drawerWidth,
            boxSizing: 'border-box',
            background: 'linear-gradient(180deg, #1976d2 30%, #00bfae 90%)',
            color: 'white',
            border: 'none',
          },
        }}
      >
        <Box sx={{ p: 2, display: 'flex', alignItems: 'center', justifyContent: 'center', flexDirection: 'column' }}>
          <img src={sampattiLogo} alt="Sampatti Bank Logo" style={{ width: 40, height: 40, marginBottom: 8 }} />
          <Typography variant="h6" component="div" fontWeight="bold">Sampatti Bank</Typography>
          <Typography variant="body2" color="rgba(255,255,255,0.7)">Admin Panel</Typography>
        </Box>
        <List sx={{ p: 1 }}>
          {navItems.map((item, index) => (
            <ListItem key={item.text} disablePadding sx={{ my: 0.5 }}>
              <ListItemButton
                selected={selected === index && item.text === 'Dashboard'}
                onClick={() => {
                  if (item.text === 'Dashboard') setSelected(index);
                  else if (item.text === 'Policy Management') openPolicyDialog();
                  else if (item.text === 'Monthly Update') setMuOpen(true);
                  else if (item.text === 'Change Password') setPwOpen(true);
                  else if (item.text === 'Logout') setLogoutOpen(true);
                }}
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
                <ListItemIcon sx={{ color: 'white', minWidth: 40 }}>{item.icon}</ListItemIcon>
                <ListItemText primary={item.text} />
              </ListItemButton>
            </ListItem>
          ))}
        </List>
      </Drawer>
      <Box component="main" sx={{ flexGrow: 1, p: 3, backgroundColor: '#F3F4F6', minHeight: '100vh' }}>
        <Typography variant="h4" gutterBottom>Dashboard</Typography>
        {loading ? <CircularProgress /> : error ? <Alert severity="error">{error}</Alert> : (
        <>
            <Grid container spacing={2} mb={3}>
                <Grid item xs={6} md={2}><Paper sx={{ p: 2, textAlign: 'center' }}><Typography variant="h6">{stats.users}</Typography><Typography>Users</Typography></Paper></Grid>
                <Grid item xs={6} md={2}><Paper sx={{ p: 2, textAlign: 'center' }}><Typography variant="h6">{stats.totalAccounts}</Typography><Typography>Total Accounts</Typography></Paper></Grid>
                <Grid item xs={6} md={2}><Paper sx={{ p: 2, textAlign: 'center' }}><Typography variant="h6">{stats.savingsAccounts}</Typography><Typography>Savings</Typography></Paper></Grid>
                <Grid item xs={6} md={2}><Paper sx={{ p: 2, textAlign: 'center' }}><Typography variant="h6">{stats.currentAccounts}</Typography><Typography>Current</Typography></Paper></Grid>
                <Grid item xs={6} md={2}><Paper sx={{ p: 2, textAlign: 'center' }}><Typography variant="h6">{stats.auditableSavingsAccounts}</Typography><Typography>Auditable</Typography></Paper></Grid>
                <Grid item xs={12} md={2}><Paper sx={{ p: 2, textAlign: 'center', bgcolor: 'primary.main', color: 'white' }}><Typography variant="h6">${stats.totalBalance.toFixed(2)}</Typography><Typography>Total Balance</Typography></Paper></Grid>
            </Grid>
            
            <Divider sx={{ my: 3 }} />

            <Grid container spacing={3}>
                <Grid item xs={12} md={6}>
                    <Typography variant="h5" gutterBottom>Users</Typography>
                    <TableContainer component={Paper}>
                        <Table size="small">
                            <TableHead>
                                <TableRow>
                                    <TableCell>ID</TableCell>
                                    <TableCell>Username</TableCell>
                                    <TableCell>Name</TableCell>
                                </TableRow>
                            </TableHead>
                            <TableBody>
                                {users.map(u => (
                                    <TableRow key={u.id}>
                                        <TableCell>{u.id}</TableCell>
                                        <TableCell>{u.username}</TableCell>
                                        <TableCell>{u.name}</TableCell>
                                    </TableRow>
                                ))}
                            </TableBody>
                        </Table>
                    </TableContainer>
                </Grid>
                <Grid item xs={12} md={6}>
                    <Typography variant="h5" gutterBottom>Accounts</Typography>
                    <TableContainer component={Paper}>
                        <Table size="small">
                            <TableHead>
                                <TableRow>
                                    <TableCell>Acc #</TableCell>
                                    <TableCell>Owner ID</TableCell>
                                    <TableCell>Type</TableCell>
                                    <TableCell align="right">Balance</TableCell>
                                </TableRow>
                            </TableHead>
                            <TableBody>
                                {accounts.map(a => (
                                    <TableRow key={a.accountNumber}>
                                        <TableCell>{a.accountNumber}</TableCell>
                                        <TableCell>{a.ownerId}</TableCell>
                                        <TableCell>{['Sav', 'Cur', 'Aud'][a.type]}</TableCell>
                                        <TableCell align="right">${a.balance.toFixed(2)}</TableCell>
                                    </TableRow>
                                ))}
                            </TableBody>
                        </Table>
                    </TableContainer>
                </Grid>
            </Grid>
        </>
        )}
      </Box>

      {/* DIALOGS */}
      <Dialog 
        open={policyOpen} 
        onClose={() => setPolicyOpen(false)}
        maxWidth="sm"
        fullWidth
        PaperProps={{
          sx: { position: 'relative', zIndex: 1300 }
        }}
      >
        <form onSubmit={handlePolicySubmit}>
          <DialogTitle>Bank Policy Management</DialogTitle>
          <DialogContent>
              {policyLoading ? <CircularProgress /> : policyError ? <Alert severity="error">{policyError}</Alert> : (
                  <Stack spacing={2} pt={1}>
                      <TextField 
                        label="Savings Interest Rate" 
                        name="savingsInterestRate" 
                        value={policy.savingsInterestRate != null ? policy.savingsInterestRate * 100 : ''} 
                        onChange={(e) => setPolicy({...policy, savingsInterestRate: e.target.value === '' ? null : parseFloat(e.target.value) / 100})} 
                        type="number" 
                        fullWidth 
                        InputProps={{
                            endAdornment: <InputAdornment position="end">%</InputAdornment>
                        }}
                      />
                      <TextField 
                        label="Current Account Fee" 
                        name="currentAccountFee" 
                        value={policy.currentAccountFee != null ? policy.currentAccountFee : ''} 
                        onChange={(e) => setPolicy({...policy, currentAccountFee: e.target.value === '' ? null : parseFloat(e.target.value)})} 
                        type="number" 
                        fullWidth
                        InputProps={{
                            startAdornment: <InputAdornment position="start">$</InputAdornment>
                        }}
                      />
                      <TextField 
                        label="Auditable Interest Rate" 
                        name="auditableInterestRate" 
                        value={policy.auditableInterestRate != null ? policy.auditableInterestRate * 100 : ''} 
                        onChange={(e) => setPolicy({...policy, auditableInterestRate: e.target.value === '' ? null : parseFloat(e.target.value) / 100})} 
                        type="number" 
                        fullWidth 
                        InputProps={{
                            endAdornment: <InputAdornment position="end">%</InputAdornment>
                        }}
                      />
                  </Stack>
              )}
          </DialogContent>
          <DialogActions>
              <Button onClick={() => setPolicyOpen(false)}>Cancel</Button>
              <Button type="submit" disabled={policyLoading}>Save</Button>
          </DialogActions>
        </form>
      </Dialog>
      
      <Dialog 
        open={muOpen} 
        onClose={() => setMuOpen(false)}
        maxWidth="sm"
        PaperProps={{
          sx: { position: 'relative', zIndex: 1300 }
        }}
      >
          <DialogTitle>Confirm Monthly Update</DialogTitle>
          <DialogContent><Typography>Apply interest and fees to all accounts?</Typography>{muError && <Alert severity="error" sx={{mt:2}}>{muError}</Alert>}</DialogContent>
          <DialogActions>
              <Button onClick={() => setMuOpen(false)}>Cancel</Button>
              <Button onClick={handleMonthlyUpdate} disabled={muLoading}>{muLoading ? <CircularProgress size={24}/> : "Apply"}</Button>
          </DialogActions>
      </Dialog>

      <Dialog 
        open={pwOpen} 
        onClose={() => setPwOpen(false)}
        maxWidth="sm"
        PaperProps={{
          sx: { position: 'relative', zIndex: 1300 }
        }}
      >
        <form onSubmit={handlePwSubmit}>
          <DialogTitle>Change Admin Password</DialogTitle>
          <DialogContent>
            <TextField autoFocus margin="dense" label="Old Password" type="password" fullWidth variant="standard" value={oldPw} onChange={(e) => setOldPw(e.target.value)} />
            <TextField margin="dense" label="New Password" type="password" fullWidth variant="standard" value={newPw} onChange={(e) => setNewPw(e.target.value)} />
            {pwError && <Alert severity="error" sx={{ mt: 2 }}>{pwError}</Alert>}
          </DialogContent>
          <DialogActions>
            <Button onClick={() => setPwOpen(false)}>Cancel</Button>
            <Button type="submit" disabled={pwLoading}>{pwLoading ? <CircularProgress size={24} /> : 'Change'}</Button>
          </DialogActions>
        </form>
      </Dialog>
      
      <Dialog 
        open={logoutOpen} 
        onClose={() => setLogoutOpen(false)}
        maxWidth="sm"
        PaperProps={{
          sx: { position: 'relative', zIndex: 1300 }
        }}
      >
        <DialogTitle>Confirm Logout</DialogTitle>
        <DialogContent><Typography>Are you sure you want to log out?</Typography></DialogContent>
        <DialogActions>
          <Button onClick={() => setLogoutOpen(false)}>Cancel</Button>
          <Button onClick={handleLogout} color="primary">Logout</Button>
        </DialogActions>
      </Dialog>

      <Snackbar open={snackbar.open} autoHideDuration={6000} onClose={() => setSnackbar({ ...snackbar, open: false })} message={snackbar.message} />
    </Box>
  );
};

export default AdminDashboard; 