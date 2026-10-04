# API documentation

All responses follow `{ success, message, data }`; failures also include `error`. Protected endpoints require `Authorization: Bearer <token>`.

| Area | Routes |
|---|---|
| Authentication | `POST /api/auth/login`, `POST /api/auth/logout`, `GET /api/auth/me` |
| Public catalogue | `GET /api/books`, `GET /api/books/:id` |
| Admin books | `GET/POST /api/admin/books`, `PUT/DELETE /api/admin/books/:id` |
| Admin students | `GET/POST /api/admin/students`, `GET/PUT /api/admin/students/:id`, `PATCH /api/admin/students/:id/status` |
| Requests | `POST/GET /api/student/requests`, `GET /api/admin/requests`, `PATCH /api/admin/requests/:id/approve`, `PATCH /api/admin/requests/:id/reject` |
| Issues | `GET/POST /api/admin/issues`, `PATCH /api/admin/issues/:id/return`, `GET /api/student/issues`, `GET /api/student/history` |
| Reporting/profile | `GET /api/admin/reports`, `GET /api/student/dashboard`, `GET /api/student/profile` |

`GET /api/books` accepts `search`, `category`, and `availability=AVAILABLE|UNAVAILABLE`. The server applies filtering instead of returning the complete database unnecessarily.
