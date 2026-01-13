-- Создание базы данных
CREATE DATABASE exam_predictions;

\c exam_predictions;

-- Таблица пользователей
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(20) DEFAULT 'student' CHECK (role IN ('student', 'admin'))
);

-- Таблица студентов
CREATE TABLE students (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    student_id VARCHAR(20) UNIQUE NOT NULL,
    course INTEGER,
    average_grade DECIMAL(3,2),
    attendance_rate DECIMAL(5,2),
    homework_completed INTEGER DEFAULT 0,
    homework_total INTEGER DEFAULT 0
);

-- Таблица экзаменов
CREATE TABLE exams (
    id SERIAL PRIMARY KEY,
    student_id INTEGER REFERENCES students(id),
    subject VARCHAR(100) NOT NULL,
    exam_date DATE,
    predicted_score INTEGER CHECK (predicted_score >= 0 AND predicted_score <= 100),
    actual_score INTEGER CHECK (actual_score >= 0 AND actual_score <= 100)
);

-- Вставка тестовых данных
INSERT INTO users (username, password, role) VALUES 
    ('admin', 'admin123', 'admin'),
    ('student1', 'pass123', 'student');

INSERT INTO students (name, student_id, course, average_grade, attendance_rate, homework_completed, homework_total) VALUES
    ('Иван Иванов', 'ST001', 2, 4.5, 95.0, 18, 20),
    ('Мария Петрова', 'ST002', 2, 3.8, 88.0, 15, 20),
    ('Алексей Сидоров', 'ST003', 1, 4.2, 92.0, 17, 20),
    ('Елена Козлова', 'ST004', 2, 3.5, 75.0, 12, 20),
    ('Дмитрий Смирнов', 'ST005', 1, 4.8, 98.0, 20, 20);

INSERT INTO exams (student_id, subject, exam_date, predicted_score) VALUES
    (1, 'Математика', '2024-12-20', 85),
    (2, 'Математика', '2024-12-20', 72),
    (3, 'Физика', '2024-12-21', 78),
    (4, 'Математика', '2024-12-20', 65),
    (5, 'Физика', '2024-12-21', 90);


