import axios from 'axios';

const API_BASE = 'http://localhost:9080';

export async function login(username, password) {
  const response = await axios.post(`${API_BASE}/login`, { username, password });
  return response.data;
} 