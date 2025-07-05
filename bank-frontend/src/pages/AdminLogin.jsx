import React, { useState } from 'react';
import { Box, TextField, Button, Alert, CircularProgress, InputAdornment, IconButton, Typography } from '@mui/material';
import { Visibility, VisibilityOff } from '@mui/icons-material';
import { useNavigate } from 'react-router-dom';
import api from '../api';
import AuthLayout from '../components/AuthLayout';

const AdminLogin = () => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');
    setLoading(true);
    try {
      const res = await api.post('/admin/login', { username, password });
      if (res.data.success) {
        localStorage.setItem('admin_username', username);
        localStorage.setItem('admin_password', password); // Note: Storing password is not secure
        navigate('/admin/dashboard');
      } else {
        setError(res.data.message || 'Login failed');
      }
    } catch (err) {
      setError(err.response?.data?.message || 'Server error');
    } finally {
      setLoading(false);
    }
  };

  return (
    <AuthLayout title="Admin Login">
      <Box sx={{ mb: 2, width: '100%', maxWidth: 400, background: '#e3f2fd', borderRadius: 2, p: 2, textAlign: 'center', border: '1px solid #90caf9' }}>
        <Typography variant="subtitle2" color="primary" fontWeight={600}>
          Default Admin Credentials (for testing):
        </Typography>
        <Typography variant="body2" color="text.secondary">
          Username: <b>admin</b><br />
          Password: <b>admin123</b>
        </Typography>
      </Box>
      <Box
        component="form"
        noValidate
        onSubmit={handleSubmit}
        sx={{ mt: 1, width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}
      >
        <TextField
          label="Username"
          value={username}
          onChange={e => setUsername(e.target.value)}
          fullWidth
          margin="normal"
          required
          autoFocus
          sx={{ maxWidth: 400 }}
        />
        <TextField
          label="Password"
          type={showPassword ? 'text' : 'password'}
          value={password}
          onChange={e => setPassword(e.target.value)}
          fullWidth
          margin="normal"
          required
          sx={{ maxWidth: 400 }}
          InputProps={{
            endAdornment: (
              <InputAdornment position="end">
                <IconButton
                  aria-label="toggle password visibility"
                  onClick={() => setShowPassword((show) => !show)}
                  edge="end"
                  size="large"
                >
                  {showPassword ? <VisibilityOff /> : <Visibility />}
                </IconButton>
              </InputAdornment>
            ),
          }}
        />
        {error && <Alert severity="error" sx={{ mt: 2, width: '100%', maxWidth: 400 }}>{error}</Alert>}
        <Button
          type="submit"
          variant="contained"
          color="primary"
          fullWidth
          sx={{ mt: 3, mb: 2, maxWidth: 400 }}
          disabled={loading}
          size="large"
        >
          {loading ? <CircularProgress size={24} /> : 'Login'}
        </Button>
      </Box>
    </AuthLayout>
  );
};

export default AdminLogin; 