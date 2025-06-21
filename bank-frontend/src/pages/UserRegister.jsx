import React, { useState } from 'react';
import { Box, Typography, Paper, TextField, Button, Alert, CircularProgress } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import api from '../api';

const UserRegister = () => {
  const [name, setName] = useState('');
  const [phone, setPhone] = useState('');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const validate = () => {
    if (!name || !phone || !username || !password || !confirmPassword) return 'All fields are required.';
    if (!/^\d{10}$/.test(phone)) return 'Phone must be 10 digits.';
    if (password !== confirmPassword) return 'Passwords do not match.';
    if (password.length < 6) return 'Password must be at least 6 characters.';
    return '';
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    const validationError = validate();
    if (validationError) {
      setError(validationError);
      return;
    }
    setError('');
    setLoading(true);
    try {
      const res = await api.post('/register', { name, phone, username, password });
      if (res.data.success) {
        localStorage.setItem('username', username);
        navigate('/user/dashboard');
      } else {
        setError(res.data.message || 'Registration failed');
      }
    } catch (err) {
      setError(err.response?.data?.message || 'Server error');
    } finally {
      setLoading(false);
    }
  };

  return (
    <Box minHeight="100vh" display="flex" alignItems="center" justifyContent="center" bgcolor="#f5f6fa">
      <Paper elevation={4} sx={{ p: 6, borderRadius: 4, minWidth: 350, textAlign: 'center' }}>
        <Typography variant="h4" fontWeight={700} mb={3} color="primary.main">
          User Registration
        </Typography>
        <form onSubmit={handleSubmit}>
          <TextField
            label="Full Name"
            value={name}
            onChange={(e) => setName(e.target.value)}
            fullWidth
            margin="normal"
            required
          />
          <TextField
            label="Phone (10 digits)"
            value={phone}
            onChange={(e) => setPhone(e.target.value)}
            fullWidth
            margin="normal"
            required
          />
          <TextField
            label="Username"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            fullWidth
            margin="normal"
            required
          />
          <TextField
            label="Password"
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            fullWidth
            margin="normal"
            required
          />
          <TextField
            label="Confirm Password"
            type="password"
            value={confirmPassword}
            onChange={(e) => setConfirmPassword(e.target.value)}
            fullWidth
            margin="normal"
            required
          />
          {error && <Alert severity="error" sx={{ mt: 2 }}>{error}</Alert>}
          <Button
            type="submit"
            variant="contained"
            color="primary"
            fullWidth
            sx={{ mt: 3 }}
            disabled={loading}
            size="large"
          >
            {loading ? <CircularProgress size={24} /> : 'Register'}
          </Button>
        </form>
      </Paper>
    </Box>
  );
};

export default UserRegister; 