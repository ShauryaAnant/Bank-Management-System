import React from 'react';
import { Box, Typography, Paper, CssBaseline } from '@mui/material';
import { Link } from 'react-router-dom';
import sampattiLogo from '../assets/sampatti-logo.svg';

const AuthLayout = ({ children, title }) => (
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
      <Link to="/" style={{ textDecoration: 'none', color: 'inherit' }}>
        <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
          <img src={sampattiLogo} alt="Sampatti Bank Logo" style={{ width: 60, height: 60, marginRight: 16 }} />
          <Typography component="h1" variant="h4" fontWeight={700}>
            Sampatti Bank
          </Typography>
        </Box>
      </Link>
      <Typography variant="h6" sx={{ mt: 1, opacity: 0.8 }}>
        Your trusted partner for modern banking solutions
      </Typography>
    </Box>
    <Box
      sx={{
        flex: { xs: '1 1 100%', sm: '1 1 60%', md: '1 1 45%' },
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        p: { xs: 2, sm: 4 },
      }}
    >
      <Paper elevation={6} sx={{ p: 4, width: '100%', maxWidth: 400, borderRadius: 2 }}>
        <Box sx={{ display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
          <Typography component="h1" variant="h5" sx={{ mb: 3 }}>
            {title}
          </Typography>
          {children}
        </Box>
      </Paper>
    </Box>
  </Box>
);

export default AuthLayout; 