-- В Docker база данных exam_prediction уже создана автоматически через переменную окружения POSTGRES_DB
-- Поэтому команды CREATE DATABASE и \c не нужны

-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    role VARCHAR(20) DEFAULT 'user' CHECK (role IN ('user', 'admin')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица студентов
CREATE TABLE IF NOT EXISTS students (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    surname VARCHAR(100) NOT NULL,
    group_name VARCHAR(50) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица оценок и данных для прогноза
CREATE TABLE IF NOT EXISTS student_grades (
    id SERIAL PRIMARY KEY,
    student_id INTEGER REFERENCES students(id) ON DELETE CASCADE,
    subject VARCHAR(100) NOT NULL,
    grade INTEGER CHECK (grade >= 1 AND grade <= 5),
    semester INTEGER NOT NULL,
    attendance_percent DECIMAL(5,2) DEFAULT 0,
    assignment_completion DECIMAL(5,2) DEFAULT 0,
    exam_result INTEGER CHECK (exam_result >= 1 AND exam_result <= 5),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Вставка тестовых данных
-- ПРИМЕЧАНИЕ: Пароли автоматически хешируются при регистрации через приложение
-- Для создания тестовых пользователей используйте интерфейс регистрации или выполните:
-- 
-- Регистрация через веб-интерфейс (рекомендуется):
-- 1. Откройте http://localhost:8080/register
-- 2. Зарегистрируйте пользователя с username='admin', password='admin', email='admin@example.com'
-- 3. Затем обновите роль через SQL: UPDATE users SET role='admin' WHERE username='admin';
--
-- Или создайте пользователей вручную с хешированными паролями
-- (используйте утилиту backend/generate_test_hashes для генерации хешей)

-- Вставка тестовых студентов
INSERT INTO students (name, surname, group_name) VALUES
    ('Иван', 'Иванов', 'ИТ-21'),
    ('Мария', 'Петрова', 'ИТ-21'),
    ('Алексей', 'Сидоров', 'ИТ-22'),
    ('Елена', 'Козлова', 'ИТ-22');

-- Вставка тестовых оценок
INSERT INTO student_grades (student_id, subject, grade, semester, attendance_percent, assignment_completion, exam_result) VALUES
    (1, 'Математика', 4, 1, 85.5, 90.0, 4),
    (1, 'Программирование', 5, 1, 95.0, 100.0, 5),
    (1, 'Базы данных', 4, 2, 80.0, 85.0, 4),
    (2, 'Математика', 3, 1, 70.0, 75.0, 3),
    (2, 'Программирование', 4, 1, 85.0, 90.0, 4),
    (3, 'Математика', 5, 1, 100.0, 100.0, 5),
    (3, 'Программирование', 5, 1, 98.0, 100.0, 5),
    (4, 'Математика', 2, 1, 60.0, 65.0, 2),
    (4, 'Программирование', 3, 1, 70.0, 72.0, 3);

