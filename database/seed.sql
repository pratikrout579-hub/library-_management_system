INSERT OR IGNORE INTO admins (id, name, email, password_hash) VALUES
  (1, 'Priya Nair', 'admin@smartlibrary.edu', '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9');

INSERT OR IGNORE INTO students (id, student_code, name, email, password_hash, phone, department, course, study_year) VALUES
 (1,'STU2026001','Aarav Sharma','aarav.sharma@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501001','Computer Science','B.Tech',3),
 (2,'STU2026002','Meera Iyer','meera.iyer@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501002','Electronics','B.Tech',2),
 (3,'STU2026003','Kabir Khan','kabir.khan@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501003','Mechanical','B.Tech',4),
 (4,'STU2026004','Ananya Gupta','ananya.gupta@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501004','Mathematics','B.Sc',2),
 (5,'STU2026005','Rohan Das','rohan.das@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501005','Physics','B.Sc',1),
 (6,'STU2026006','Ishita Rao','ishita.rao@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501006','English','B.A.',3),
 (7,'STU2026007','Dev Patel','dev.patel@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501007','Commerce','B.Com',2),
 (8,'STU2026008','Sana Ali','sana.ali@college.edu','703b0a3d6ad75b649a28adde7d83c6251da457549263bc7ff45ec709b0a8448b','9876501008','History','B.A.',1);

INSERT OR IGNORE INTO books (id,title,author,category,isbn,publisher,publication_year,total_quantity,available_quantity) VALUES
 (1,'Clean Code','Robert C. Martin','Programming','9780132350884','Prentice Hall',2008,6,5),
 (2,'Introduction to Algorithms','Thomas H. Cormen','Programming','9780262046305','MIT Press',2022,5,4),
 (3,'Database System Concepts','Abraham Silberschatz','Database','9781260084504','McGraw Hill',2019,4,4),
 (4,'Operating System Concepts','Abraham Silberschatz','Systems','9781119800361','Wiley',2021,5,4),
 (5,'Computer Networks','Andrew S. Tanenbaum','Networks','9780132126953','Pearson',2011,3,3),
 (6,'The Pragmatic Programmer','David Thomas','Programming','9780135957059','Addison-Wesley',2019,4,4),
 (7,'Design Patterns','Erich Gamma','Programming','9780201633610','Addison-Wesley',1994,3,3),
 (8,'Artificial Intelligence: A Modern Approach','Stuart Russell','Artificial Intelligence','9780134610993','Pearson',2021,4,4),
 (9,'The Theory of Everything','Stephen Hawking','Science','9788179925911','Jaico',2010,3,3),
 (10,'A Brief History of Time','Stephen Hawking','Science','9780553380163','Bantam',1998,5,5),
 (11,'Linear Algebra Done Right','Sheldon Axler','Mathematics','9783319110790','Springer',2015,3,3),
 (12,'Introduction to Psychology','James Kalat','Psychology','9781337408271','Cengage',2016,4,4),
 (13,'The Republic','Plato','Philosophy','9780140455113','Penguin',2007,3,3),
 (14,'Pride and Prejudice','Jane Austen','Literature','9780141439518','Penguin',2002,6,6),
 (15,'The Art of War','Sun Tzu','History','9781590302255','Shambhala',2005,4,4),
 (16,'Sapiens','Yuval Noah Harari','History','9780062316097','Harper',2015,5,5);

INSERT OR IGNORE INTO issue_records (id,student_id,book_id,issued_by,issue_date,due_date,status) VALUES
 (1,2,1,1,'2026-09-20','2026-10-04','ISSUED'),
 (2,3,2,1,'2026-09-02','2026-09-16','ISSUED'),
 (3,4,3,1,'2026-08-01','2026-08-15','RETURNED');
UPDATE issue_records SET return_date='2026-08-18', fine=15 WHERE id=3;
INSERT OR IGNORE INTO book_requests (id,student_id,book_id,status,request_date) VALUES
 (1,1,4,'PENDING','2026-09-25 10:15:00'),
 (2,5,6,'PENDING','2026-09-26 12:00:00'),
 (3,6,7,'REJECTED','2026-09-20 09:00:00');
UPDATE book_requests SET processed_date='2026-09-20 10:00:00', processed_by=1, rejection_reason='Reference copy reserved for the reading room.' WHERE id=3;
INSERT OR IGNORE INTO activity_logs (actor_type,actor_id,event_type,message) VALUES
 ('ADMIN',1,'LOGIN','Admin Priya Nair signed in'),
 ('STUDENT',1,'BOOK_REQUESTED','Aarav Sharma requested Operating System Concepts'),
 ('ADMIN',1,'BOOK_ISSUED','Clean Code issued to Meera Iyer');
