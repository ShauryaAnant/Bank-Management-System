// src/api/auth.js
import axios from 'axios';

const API_BASE = import.meta.env.VITE_API_BASE_URL;

export async function login(username, password) {
  const response = await axios.post(`${API_BASE}/login`, { username, password });
  return response.data;
}