import React, { useState } from 'react';
import { Box, TextField, Button, Alert, CircularProgress, Grid } from '@mui/material';
import { useNavigate, Link } from 'react-router-dom';
import api from '../api';
import AuthLayout from '../components/AuthLayout';

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
    <AuthLayout title="Create Your Account">
      <Box component="form" noValidate onSubmit={handleSubmit} sx={{ mt: 1, width: '100%' }}>
        <Grid container spacing={2}>
            <Grid item xs={12}>
                <TextField
                    label="Full Name"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    fullWidth
                    required
                    autoFocus
                />
            </Grid>
            <Grid item xs={12}>
                <TextField
                    label="Phone (10 digits)"
                    value={phone}
                    onChange={(e) => setPhone(e.target.value)}
                    fullWidth
                    required
                />
            </Grid>
            <Grid item xs={12}>
                <TextField
                    label="Username"
                    value={username}
                    onChange={(e) => setUsername(e.target.value)}
                    fullWidth
                    required
                />
            </Grid>
            <Grid item xs={12}>
                <TextField
                    label="Password"
                    type="password"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    fullWidth
                    required
                />
            </Grid>
            <Grid item xs={12}>
                <TextField
                    label="Confirm Password"
                    type="password"
                    value={confirmPassword}
                    onChange={(e) => setConfirmPassword(e.target.value)}
                    fullWidth
                    required
                />
            </Grid>
        </Grid>
        {error && <Alert severity="error" sx={{ mt: 2, width: '100%' }}>{error}</Alert>}
        <Button
            type="submit"
            variant="contained"
            color="primary"
            fullWidth
            sx={{ mt: 3, mb: 2 }}
            disabled={loading}
            size="large"
        >
            {loading ? <CircularProgress size={24} /> : 'Register'}
        </Button>
        <Grid container justifyContent="flex-end">
              <Grid item>
                <Link to="/user/login" variant="body2">
                  Already have an account? Sign in
                </Link>
              </Grid>
        </Grid>
      </Box>
    </AuthLayout>
  );
};

export default UserRegister; 