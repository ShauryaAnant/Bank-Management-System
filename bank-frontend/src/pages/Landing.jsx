import React from 'react';
import { Box, Button, Typography, Paper, Stack } from '@mui/material';
import { useNavigate } from 'react-router-dom';

const Landing = () => {
  const navigate = useNavigate();
  return (
    <Box minHeight="100vh" display="flex" alignItems="center" justifyContent="center" bgcolor="#f5f6fa">
      <Paper elevation={4} sx={{ p: 6, borderRadius: 4, minWidth: 350, textAlign: 'center' }}>
        <Typography variant="h4" fontWeight={700} mb={3} color="primary.main">
          Welcome to ABC Bank
        </Typography>
        <Typography variant="subtitle1" mb={4} color="text.secondary">
          Please choose an option to continue
        </Typography>
        <Stack spacing={3}>
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
  );
};

export default Landing; 