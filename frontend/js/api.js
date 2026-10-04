const API = '/api';
const token = () => localStorage.getItem('library_token') || '';

async function request(path, options = {}) {
  const response = await fetch(`${API}${path}`, { ...options, headers: { 'Content-Type': 'application/json', ...(token() ? { Authorization: `Bearer ${token()}` } : {}), ...(options.headers || {}) } });
  const payload = await response.json().catch(() => ({ success: false, message: 'The server returned an invalid response.' }));
  if (!response.ok || !payload.success) throw new Error(payload.message || 'Request failed');
  return payload.data;
}
const body = (method, data) => ({ method, body: JSON.stringify(data) });
export const api = {
  login: (email, password) => request('/auth/login', body('POST', { email, password })),
  logout: () => request('/auth/logout', { method: 'POST' }), me: () => request('/auth/me'),
  books: (params = {}, admin = false) => request(`${admin ? '/admin/books' : '/books'}?${new URLSearchParams(params)}`),
  book: id => request(`/books/${id}`), createBook: data => request('/admin/books', body('POST', data)), updateBook: (id, data) => request(`/admin/books/${id}`, body('PUT', data)), deleteBook: id => request(`/admin/books/${id}`, { method: 'DELETE' }),
  students: search => request(`/admin/students?${new URLSearchParams({ search })}`), student: id => request(`/admin/students/${id}`), createStudent: data => request('/admin/students', body('POST', data)), updateStudent: (id, data) => request(`/admin/students/${id}`, body('PUT', data)), studentStatus: (id, status) => request(`/admin/students/${id}/status`, body('PATCH', { status })),
  requests: status => request(`/admin/requests?${new URLSearchParams({ status })}`), myRequests: () => request('/student/requests'), requestBook: book_id => request('/student/requests', body('POST', { book_id })), approveRequest: (id, due_date) => request(`/admin/requests/${id}/approve`, body('PATCH', { due_date })), rejectRequest: (id, reason) => request(`/admin/requests/${id}/reject`, body('PATCH', { reason })),
  issues: status => request(`/admin/issues?${new URLSearchParams({ status })}`), issueBook: data => request('/admin/issues', body('POST', data)), returnBook: (id, return_date) => request(`/admin/issues/${id}/return`, body('PATCH', { return_date })), myBooks: () => request('/student/issues'), history: () => request('/student/history'), studentProfile: () => request('/student/profile'),
  reports: () => request('/admin/reports'), studentDashboard: () => request('/student/dashboard')
};
