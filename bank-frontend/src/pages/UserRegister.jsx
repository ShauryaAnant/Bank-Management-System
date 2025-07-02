import React, { useState } from 'react';
import { Box, TextField, Button, Alert, CircularProgress, Grid, InputAdornment, IconButton, Typography } from '@mui/material';
import { useNavigate, Link } from 'react-router-dom';
import { Visibility, VisibilityOff } from '@mui/icons-material';
import api from '../api';
import AuthLayout from '../components/AuthLayout';

const UserRegister = () => {
  const [name, setName] = useState('');
  const [phone, setPhone] = useState('');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [showConfirmPassword, setShowConfirmPassword] = useState(false);
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
      <Box
        component="form"
        noValidate
        onSubmit={handleSubmit}
        sx={{ mt: 1, width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}
      >
        <Grid container spacing={2} maxWidth={400} justifyContent="center" alignItems="center" direction="column">
          <Grid item xs={12} sx={{ width: '100%' }}>
            <TextField
              label="Full Name"
              value={name}
              onChange={(e) => setName(e.target.value)}
              fullWidth
              required
              autoFocus
            />
          </Grid>
          <Grid item xs={12} sx={{ width: '100%' }}>
            <TextField
              label="Phone (10 digits)"
              value={phone}
              onChange={(e) => setPhone(e.target.value)}
              fullWidth
              required
            />
          </Grid>
          <Grid item xs={12} sx={{ width: '100%' }}>
            <TextField
              label="Username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              fullWidth
              required
            />
          </Grid>
          <Grid item xs={12} sx={{ width: '100%' }}>
            <TextField
              label="Password"
              type={showPassword ? 'text' : 'password'}
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              fullWidth
              required
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
          </Grid>
          <Grid item xs={12} sx={{ width: '100%' }}>
            <TextField
              label="Confirm Password"
              type={showConfirmPassword ? 'text' : 'password'}
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              fullWidth
              required
              InputProps={{
                endAdornment: (
                  <InputAdornment position="end">
                    <IconButton
                      aria-label="toggle confirm password visibility"
                      onClick={() => setShowConfirmPassword((show) => !show)}
                      edge="end"
                      size="large"
                    >
                      {showConfirmPassword ? <VisibilityOff /> : <Visibility />}
                    </IconButton>
                  </InputAdornment>
                ),
              }}
            />
          </Grid>
        </Grid>
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
          {loading ? <CircularProgress size={24} /> : 'Register'}
        </Button>
        <Box sx={{ width: '100%', display: 'flex', justifyContent: 'center', mt: 1 }}>
          <Typography variant="body2">
            Already have an account?{' '}
            <Link to="/user/login" style={{ textDecoration: 'none', color: '#1976d2', fontWeight: 600 }}>
              Sign in
            </Link>
          </Typography>
        </Box>
      </Box>
    </AuthLayout>
  );
};

export default UserRegister; 