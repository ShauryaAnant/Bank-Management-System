import axios from 'axios';

const api = axios.create({
  baseURL: 'http://localhost:9080',
});

export default api; 