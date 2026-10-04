# Test plan and execution record

The first seven cases can be exercised automatically with the provided API after building on Linux. Cases marked **Not executed locally** require Linux packages not available in the supplied development workspace; they are explicit rather than claimed as passing.

| ID | Description | Input | Expected result | Actual result | Status |
|---|---|---|---|---|---|
| T01 | Admin login | Valid admin credentials | Token and ADMIN role | Awaiting Linux run | Not executed locally |
| T02 | Student login | Valid student credentials | Token and STUDENT role | Awaiting Linux run | Not executed locally |
| T03 | Reject invalid login | Bad password | 400 with safe message | Route inspected | Ready |
| T04 | Book validation | Quantity 0 / bad ISBN | 400 validation error | Route inspected | Ready |
| T05 | Duplicate request | Same student/book pending | 400 duplicate message | Route inspected | Ready |
| T06 | Approval flow | Pending request + due date | Issue created, count -1, APPROVED | Route inspected | Ready |
| T07 | Return flow | Active issue after due date | RETURNED, count +1, fine | Route inspected | Ready |
| T08 | Authorization | Student calls admin endpoint | 403 response | Route inspected | Ready |
| T09 | Search/filter | Catalogue query params | Filtered active rows only | Route inspected | Ready |
| T10 | UI syntax | JavaScript modules | Parse without syntax error | Run in current workspace | Passed |

## End-to-end manual script

1. Start server and sign in as admin.
2. Add one book and one student; note their IDs.
3. Sign out; sign in as the newly added student; search and request the book.
4. Sign in as admin; approve the pending request with a due date; verify the available count falls by one and a new issued record appears.
5. Sign in as the student; verify it appears under **My Books**.
6. Sign in as admin; return the issue with a date after the due date; verify stock rises, status becomes `RETURNED`, and the fine equals late days × 5.
7. Check `activity_logs` and `logs/activity.log` for relevant events.
