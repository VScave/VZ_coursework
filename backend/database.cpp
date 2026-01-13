#include "database.h"
#include "queries.h"
#include "password_hash.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <string>

Database::Database(const std::string& conn_str) : connection_string(conn_str) {}

Database::~Database() {}

bool Database::registerUserWithRole(const std::string& username, const std::string& password, const std::string& email, const std::string& role) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        // Проверка на существование пользователя по username
        pqxx::result check_username = txn.exec_params(Queries::CHECK_USER_EXISTS, username);
        if (!check_username.empty()) {
            std::cerr << "Registration with role failed: username already exists: " << username << std::endl;
            return false;
        }
        
        // Проверка на существование пользователя по email
        pqxx::result check_email = txn.exec_params(Queries::CHECK_EMAIL_EXISTS, email);
        if (!check_email.empty()) {
            std::cerr << "Registration with role failed: email already exists: " << email << std::endl;
            return false;
        }
        
        // Хешируем пароль перед сохранением
        std::string passwordHash = PasswordHash::hashPassword(password);
        
        // Используем INSERT с обработкой возможного нарушения уникальности
        // (на случай race condition между проверкой и вставкой)
        try {
            txn.exec_params(Queries::INSERT_USER_WITH_ROLE, username, passwordHash, email, role);
            txn.commit();
            std::cout << "User with role registered successfully: " << username << " (" << role << ")" << std::endl;
            return true;
        } catch (const pqxx::sql_error& e) {
            // Проверяем, является ли это нарушением уникальности (SQLSTATE 23505)
            std::string sqlstate = e.sqlstate();
            if (sqlstate == "23505") {
                // Если все же произошло нарушение уникальности (race condition),
                // откатываем транзакцию и проверяем что именно нарушено
                txn.abort();
                std::cerr << "Database unique violation (race condition): " << e.what() << std::endl;
                
                // Проверяем еще раз, что именно нарушено
                pqxx::work txn2(conn);
                pqxx::result check_username2 = txn2.exec_params(Queries::CHECK_USER_EXISTS, username);
                if (!check_username2.empty()) {
                    std::cerr << "Username exists (race condition): " << username << std::endl;
                    return false;
                }
                
                pqxx::result check_email2 = txn2.exec_params(Queries::CHECK_EMAIL_EXISTS, email);
                if (!check_email2.empty()) {
                    std::cerr << "Email exists (race condition): " << email << std::endl;
                    return false;
                }
                
                // Если не понятно что, возвращаем false
                return false;
            } else {
                // Другая SQL ошибка, пробрасываем дальше
                throw;
            }
        }
    } catch (const pqxx::sql_error& e) {
        // Проверяем, является ли это нарушением уникальности (SQLSTATE 23505)
        std::string sqlstate = e.sqlstate();
        if (sqlstate == "23505") {
            std::cerr << "Database unique violation: " << e.what() << std::endl;
            return false;
        }
        // Другая SQL ошибка
        std::cerr << "Database SQL error in registerUserWithRole: " << e.what() << " (SQLSTATE: " << sqlstate << ")" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Database error in registerUserWithRole: " << e.what() << std::endl;
        return false;
    }
}

bool Database::createDefaultAdmin() {
    // Создать тестового админа, если его еще нет
    try {
        std::cout << "Checking for default admin user..." << std::endl;
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        // Проверка существования админа по username
        pqxx::result check = txn.exec_params(Queries::CHECK_USER_EXISTS, std::string("admin"));
        if (!check.empty()) {
            // Админ уже существует, проверяем его роль
            std::cout << "Admin user already exists, checking role..." << std::endl;
            pqxx::result adminResult = txn.exec_params(Queries::GET_USER_BY_USERNAME, std::string("admin"));
            if (!adminResult.empty()) {
                std::string existingRole;
                // Проверяем, не NULL ли роль
                if (adminResult[0][3].is_null()) {
                    existingRole = "";
                    std::cout << "Existing admin role is NULL" << std::endl;
                } else {
                    existingRole = adminResult[0][3].as<std::string>();
                    std::cout << "Existing admin role: '" << existingRole << "'" << std::endl;
                }
                
                // Если роль не admin (NULL, пустая или другая), обновляем ее
                if (existingRole.empty() || existingRole != "admin") {
                    std::cout << "Updating admin role to 'admin'..." << std::endl;
                    txn.exec("UPDATE users SET role = 'admin' WHERE username = 'admin'");
                    txn.commit();
                    std::cout << "Admin role updated successfully" << std::endl;
                } else {
                    std::cout << "Admin role is correct: " << existingRole << std::endl;
                }
            }
            return true;
        }
        
        // Проверяем, может быть админ существует с другим email
        pqxx::result emailCheck = txn.exec_params(Queries::CHECK_EMAIL_EXISTS, std::string("admin@example.com"));
        if (!emailCheck.empty()) {
            std::cout << "Warning: Email admin@example.com already exists with different username" << std::endl;
        }
        
        // Создаем админа с ролью admin
        std::cout << "Creating default admin user..." << std::endl;
        std::string passwordHash = PasswordHash::hashPassword("admin");
        std::cout << "Password hash generated, length: " << passwordHash.length() << std::endl;
        
        txn.exec_params(Queries::INSERT_USER_WITH_ROLE, 
                       std::string("admin"), 
                       passwordHash, 
                       std::string("admin@example.com"),
                       std::string("admin"));
        txn.commit();
        std::cout << "Тестовый админ создан: username=admin, password=admin, role=admin" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error при создании админа: " << e.what() << std::endl;
        return false;
    }
}

bool Database::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        // Проверка на существование пользователя по username
        pqxx::result check_username = txn.exec_params(Queries::CHECK_USER_EXISTS, username);
        if (!check_username.empty()) {
            std::cerr << "Registration failed: username already exists: " << username << std::endl;
            return false;
        }
        
        // Проверка на существование пользователя по email
        pqxx::result check_email = txn.exec_params(Queries::CHECK_EMAIL_EXISTS, email);
        if (!check_email.empty()) {
            std::cerr << "Registration failed: email already exists: " << email << std::endl;
            return false;
        }
        
        // Хешируем пароль перед сохранением
        std::string passwordHash = PasswordHash::hashPassword(password);
        
        // Используем INSERT с обработкой возможного нарушения уникальности
        // (на случай race condition между проверкой и вставкой)
        try {
            txn.exec_params(Queries::INSERT_USER, username, passwordHash, email);
            txn.commit();
            std::cout << "User registered successfully: " << username << std::endl;
            return true;
        } catch (const pqxx::sql_error& e) {
            // Проверяем, является ли это нарушением уникальности (SQLSTATE 23505)
            std::string sqlstate = e.sqlstate();
            if (sqlstate == "23505") {
                // Если все же произошло нарушение уникальности (race condition),
                // откатываем транзакцию и проверяем что именно нарушено
                txn.abort();
                std::cerr << "Database unique violation (race condition): " << e.what() << std::endl;
                
                // Проверяем еще раз, что именно нарушено
                pqxx::work txn2(conn);
                pqxx::result check_username2 = txn2.exec_params(Queries::CHECK_USER_EXISTS, username);
                if (!check_username2.empty()) {
                    std::cerr << "Username exists (race condition): " << username << std::endl;
                    return false;
                }
                
                pqxx::result check_email2 = txn2.exec_params(Queries::CHECK_EMAIL_EXISTS, email);
                if (!check_email2.empty()) {
                    std::cerr << "Email exists (race condition): " << email << std::endl;
                    return false;
                }
                
                // Если не понятно что, возвращаем false
                return false;
            } else {
                // Другая SQL ошибка, пробрасываем дальше
                throw;
            }
        }
    } catch (const pqxx::sql_error& e) {
        // Проверяем, является ли это нарушением уникальности (SQLSTATE 23505)
        std::string sqlstate = e.sqlstate();
        if (sqlstate == "23505") {
            std::cerr << "Database unique violation: " << e.what() << std::endl;
            return false;
        }
        // Другая SQL ошибка
        std::cerr << "Database SQL error in registerUser: " << e.what() << " (SQLSTATE: " << sqlstate << ")" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Database error in registerUser: " << e.what() << std::endl;
        return false;
    }
}

User* Database::authenticateUser(const std::string& username, const std::string& password) {
    try {
        std::cout << "Attempting authentication for username: " << username << std::endl;
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        // Получаем пользователя по имени (включая хеш пароля)
        pqxx::result result = txn.exec_params(Queries::GET_USER_BY_USERNAME, username);
        
        if (result.empty()) {
            std::cerr << "Authentication failed: user not found: " << username << std::endl;
            return nullptr;
        }
        
        std::cout << "User found in database: " << username << std::endl;
        
        // Получаем сохраненный хеш пароля (колонка 4 - password)
        std::string storedPasswordHash = result[0][4].as<std::string>();
        
        // Получаем роль, проверяя на NULL
        std::string userRole;
        if (result[0][3].is_null()) {
            std::cerr << "Warning: User role is NULL, setting to 'user' by default" << std::endl;
            userRole = "user";
        } else {
            userRole = result[0][3].as<std::string>();
        }
        
        // Если роль пустая, устанавливаем по умолчанию 'user'
        if (userRole.empty()) {
            std::cerr << "Warning: User role is empty, setting to 'user' by default" << std::endl;
            userRole = "user";
        }
        
        std::cout << "User role from DB: " << (userRole.empty() ? "(empty)" : userRole) << std::endl;
        std::cout << "Stored password hash length: " << storedPasswordHash.length() << std::endl;
        
        // Проверяем пароль
        bool passwordValid = PasswordHash::verifyPassword(password, storedPasswordHash);
        std::cout << "Password verification result: " << (passwordValid ? "SUCCESS" : "FAILED") << std::endl;
        
        if (!passwordValid) {
            std::cerr << "Authentication failed: invalid password for user: " << username << std::endl;
            return nullptr;
        }
        
        // Пароль верный, создаем объект пользователя
        User* user = new User();
        user->id = result[0][0].as<int>();
        user->username = result[0][1].as<std::string>();
        user->email = result[0][2].as<std::string>();
        user->role = result[0][3].as<std::string>();
        
        std::cout << "Authentication successful for user: " << username << " (role: " << user->role << ")" << std::endl;
        return user;
    } catch (const std::exception& e) {
        std::cerr << "Database error in authenticateUser: " << e.what() << std::endl;
        return nullptr;
    }
}

bool Database::isAdmin(int user_id) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        pqxx::result result = txn.exec_params(Queries::GET_USER_ROLE, user_id);
        
        if (!result.empty() && result[0][0].as<std::string>() == "admin") {
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Student> Database::getAllStudents() {
    std::vector<Student> students;
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        pqxx::result result = txn.exec(Queries::GET_ALL_STUDENTS);
        
        for (auto row : result) {
            Student student;
            student.id = row[0].as<int>();
            student.name = row[1].as<std::string>();
            student.surname = row[2].as<std::string>();
            student.group_name = row[3].as<std::string>();
            students.push_back(student);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
    return students;
}

bool Database::addStudent(const std::string& name, const std::string& surname, const std::string& group_name) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::INSERT_STUDENT, name, surname, group_name);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::updateStudent(int id, const std::string& name, const std::string& surname, const std::string& group_name) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::UPDATE_STUDENT, name, surname, group_name, id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteStudent(int id) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::DELETE_STUDENT, id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Grade> Database::getStudentGrades(int student_id) {
    std::vector<Grade> grades;
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        pqxx::result result = txn.exec_params(Queries::GET_STUDENT_GRADES, student_id);
        
        for (auto row : result) {
            Grade grade;
            grade.id = row[0].as<int>();
            grade.student_id = row[1].as<int>();
            grade.subject = row[2].as<std::string>();
            grade.grade = row[3].as<int>();
            grade.semester = row[4].as<int>();
            grade.attendance_percent = row[5].as<double>();
            grade.assignment_completion = row[6].as<double>();
            grade.exam_result = row[7].is_null() ? 0 : row[7].as<int>();
            grades.push_back(grade);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
    return grades;
}

std::vector<Grade> Database::getAllGrades() {
    std::vector<Grade> grades;
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        pqxx::result result = txn.exec(Queries::GET_ALL_GRADES);
        
        for (auto row : result) {
            Grade grade;
            grade.id = row[0].as<int>();
            grade.student_id = row[1].as<int>();
            grade.subject = row[2].as<std::string>();
            grade.grade = row[3].as<int>();
            grade.semester = row[4].as<int>();
            grade.attendance_percent = row[5].as<double>();
            grade.assignment_completion = row[6].as<double>();
            grade.exam_result = row[7].is_null() ? 0 : row[7].as<int>();
            grades.push_back(grade);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
    return grades;
}

bool Database::addGrade(int student_id, const std::string& subject, int grade, int semester,
                        double attendance, double assignment, int exam_result) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::INSERT_GRADE, student_id, subject, grade, semester, attendance, assignment, exam_result);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::updateGrade(int id, const std::string& subject, int grade, int semester,
                           double attendance, double assignment, int exam_result) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::UPDATE_GRADE, subject, grade, semester, attendance, assignment, exam_result, id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteGrade(int id) {
    try {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
        
        txn.exec_params(Queries::DELETE_GRADE, id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::predictExamSuccess(int student_id) {
    try {
        std::vector<Grade> grades = getStudentGrades(student_id);
        
        if (grades.empty()) {
            return "Недостаточно данных для прогноза";
        }
        
        double avgGrade = 0.0;
        double avgAttendance = 0.0;
        double avgAssignment = 0.0;
        int count = 0;
        
        for (const auto& grade : grades) {
            avgGrade += grade.grade;
            avgAttendance += grade.attendance_percent;
            avgAssignment += grade.assignment_completion;
            count++;
        }
        
        if (count > 0) {
            avgGrade /= count;
            avgAttendance /= count;
            avgAssignment /= count;
        }
        
        // Простая формула прогноза: средние оценки * веса посещаемости и выполнения заданий
        double prediction_score = (avgGrade * 0.5) + (avgAttendance / 20.0 * 0.25) + (avgAssignment / 20.0 * 0.25);
        
        std::string result;
        if (prediction_score >= 4.5) {
            result = "Отлично (5) - вероятность: " + std::to_string((int)(prediction_score * 20)) + "%";
        } else if (prediction_score >= 3.5) {
            result = "Хорошо (4) - вероятность: " + std::to_string((int)(prediction_score * 20)) + "%";
        } else if (prediction_score >= 2.5) {
            result = "Удовлетворительно (3) - вероятность: " + std::to_string((int)(prediction_score * 18)) + "%";
        } else {
            result = "Неудовлетворительно (2) - вероятность: " + std::to_string((int)((5.0 - prediction_score) * 15)) + "%";
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Prediction error: " << e.what() << std::endl;
        return "Ошибка при расчете прогноза";
    }
}

