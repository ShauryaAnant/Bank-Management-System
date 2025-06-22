import React from 'react';
import { Box, Button, Typography, Paper, Stack, CssBaseline } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import sampattiLogo from '../assets/sampatti-logo.svg';

const Landing = () => {
  const navigate = useNavigate();
  return (
    <Box sx={{ display: 'flex', height: '100vh', width: '100vw' }}>
      <CssBaseline />
      <Box
        sx={{
          flex: { sm: '1 1 40%', md: '1 1 55%' },
          display: { xs: 'none', sm: 'flex' },
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          background: 'linear-gradient(45deg, #1976d2 30%, #00bfae 90%)',
          color: 'white',
          p: 4,
          textAlign: 'center',
        }}
      >
        <img src={sampattiLogo} alt="Sampatti Bank Logo" style={{ width: 100, height: 100, marginBottom: 20 }} />
        <Typography component="h1" variant="h3" fontWeight={700}>
          Sampatti Bank
        </Typography>
        <Typography variant="h6" sx={{ mt: 2, opacity: 0.8 }}>
          Your trusted partner for modern banking solutions
        </Typography>
      </Box>
      <Box
        sx={{
          flex: { xs: '1 1 100%', sm: '1 1 60%', md: '1 1 45%' },
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
        }}
      >
        <Paper
          elevation={6}
          sx={{
            p: 4,
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            width: '100%',
            maxWidth: 320,
            borderRadius: 2,
          }}
        >
          <Typography component="h1" variant="h5" sx={{ mb: 4, fontWeight: 'bold' }}>
            Welcome!
          </Typography>
          <Stack spacing={3} sx={{ width: '100%' }}>
            <Button variant="contained" size="large" color="primary" onClick={() => navigate('/admin/login')}>
              Login as Admin
            </Button>
            <Button variant="contained" size="large" color="secondary" onClick={() => navigate('/user/login')}>
              Login as User
            </Button>
            <Button variant="outlined" size="large" color="primary" onClick={() => navigate('/user/register')}>
              Register as User
            </Button>
          </Stack>
        </Paper>
      </Box>
    </Box>
  );
};

export default Landing; 