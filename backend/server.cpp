#include "database.h"
#include "httplib.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>

// Простая сессия (в реальном приложении использовать JWT или cookies)
std::map<std::string, User*> sessions;
std::string generateSessionId() {
    return std::to_string(rand() % 1000000);
}

std::string readFile(const std::string& path) {
    // Путь относительно корня проекта
    std::string full_path = "../" + path;
    if (path.find("/") == 0) {
        full_path = path;  // Абсолютный путь
    }
    std::ifstream file(full_path);
    if (!file.is_open()) {
        // Попробуем без префикса
        file.open(path);
        if (!file.is_open()) {
            return "";
        }
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getEnvVar(const std::string& key, const std::string& defaultValue) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultValue;
}

int main() {
    // Инициализация генератора случайных чисел
    srand(time(nullptr));
    
    // Подключение к базе данных (из переменных окружения или значения по умолчанию)
    std::string db_host = getEnvVar("DB_HOST", "localhost");
    std::string db_port = getEnvVar("DB_PORT", "5432");
    std::string db_name = getEnvVar("DB_NAME", "exam_prediction");
    std::string db_user = getEnvVar("DB_USER", "postgres");
    std::string db_password = getEnvVar("DB_PASSWORD", "postgres");
    
    std::string conn_str = "dbname=" + db_name + " user=" + db_user + 
                          " password=" + db_password + " host=" + db_host + 
                          " port=" + db_port;
    
    std::cout << "Подключение к БД: " << db_host << ":" << db_port << "/" << db_name << std::endl;
    
    Database db(conn_str);
    
    // Создать тестового админа при первом запуске (если его еще нет)
    db.createDefaultAdmin();
    
    httplib::Server svr;
    
    // Статические файлы CSS и JS (путь относительно корня проекта)
    svr.set_mount_point("/static", "./frontend");
    
    // Главная страница
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string content = readFile("frontend/index.html");
        if (content.empty()) {
            content = readFile("./frontend/index.html");
        }
        res.set_content(content, "text/html");
    });
    
    // Страница входа
    svr.Get("/login", [](const httplib::Request& req, httplib::Response& res) {
        std::string content = readFile("frontend/login.html");
        if (content.empty()) {
            content = readFile("./frontend/login.html");
        }
        res.set_content(content, "text/html");
    });
    
    // Страница регистрации
    svr.Get("/register", [](const httplib::Request& req, httplib::Response& res) {
        std::string content = readFile("frontend/register.html");
        if (content.empty()) {
            content = readFile("./frontend/register.html");
        }
        res.set_content(content, "text/html");
    });
    
    // API: Регистрация
    svr.Post("/api/register", [&db](const httplib::Request& req, httplib::Response& res) {
        auto username = req.get_param_value("username");
        auto password = req.get_param_value("password");
        auto email = req.get_param_value("email");
        
        std::cout << "Registration attempt - username: " << username 
                  << ", email: " << email << std::endl;
        
        if (username.empty() || password.empty() || email.empty()) {
            std::cerr << "Registration failed: missing required fields" << std::endl;
            res.set_content(R"({"success": false, "message": "Все поля обязательны"})", "application/json");
            return;
        }
        
        if (db.registerUser(username, password, email)) {
            std::cout << "Registration successful for: " << username << std::endl;
            res.set_content(R"({"success": true, "message": "Регистрация успешна"})", "application/json");
        } else {
            std::cerr << "Registration failed for: " << username << " (user or email already exists)" << std::endl;
            res.set_content(R"({"success": false, "message": "Пользователь с таким именем или email уже существует"})", "application/json");
        }
    });
    
    // API: Вход
    svr.Post("/api/login", [&db](const httplib::Request& req, httplib::Response& res) {
        auto username = req.get_param_value("username");
        auto password = req.get_param_value("password");
        
        std::cout << "Login attempt - username: " << username << ", password length: " << password.length() << std::endl;
        
        if (username.empty() || password.empty()) {
            std::cerr << "Login failed: empty username or password" << std::endl;
            res.set_content(R"({"success": false, "message": "Логин и пароль обязательны"})", "application/json");
            return;
        }
        
        User* user = db.authenticateUser(username, password);
        if (user) {
            std::string session_id = generateSessionId();
            sessions[session_id] = user;
            
            std::cout << "Login successful for: " << username << " (role: " << user->role << "), session_id: " << session_id << std::endl;
            
            std::string response = "{\"success\": true, \"session_id\": \"" + session_id + 
                                  "\", \"role\": \"" + user->role + "\"}";
            res.set_content(response, "application/json");
        } else {
            std::cerr << "Login failed for: " << username << " (invalid credentials)" << std::endl;
            res.set_content(R"({"success": false, "message": "Неверный логин или пароль"})", "application/json");
        }
    });
    
    // API: Получить всех студентов
    svr.Get("/api/students", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end()) {
            res.status = 401;
            res.set_content(R"({"error": "Не авторизован"})", "application/json");
            return;
        }
        
        auto students = db.getAllStudents();
        std::string json = "[";
        for (size_t i = 0; i < students.size(); i++) {
            json += "{";
            json += "\"id\": " + std::to_string(students[i].id) + ",";
            json += "\"name\": \"" + students[i].name + "\",";
            json += "\"surname\": \"" + students[i].surname + "\",";
            json += "\"group_name\": \"" + students[i].group_name + "\"";
            json += "}";
            if (i < students.size() - 1) json += ",";
        }
        json += "]";
        res.set_content(json, "application/json");
    });
    
    // API: Прогноз для студента
    svr.Get("/api/predict", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        auto student_id_str = req.get_param_value("student_id");
        
        if (sessions.find(session_id) == sessions.end()) {
            res.status = 401;
            res.set_content(R"({"error": "Не авторизован"})", "application/json");
            return;
        }
        
        int student_id = std::stoi(student_id_str);
        std::string prediction = db.predictExamSuccess(student_id);
        res.set_content("{\"prediction\": \"" + prediction + "\"}", "application/json");
    });
    
    // API: Получить все оценки
    svr.Get("/api/grades", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end()) {
            res.status = 401;
            res.set_content(R"({"error": "Не авторизован"})", "application/json");
            return;
        }
        
        auto grades = db.getAllGrades();
        std::string json = "[";
        for (size_t i = 0; i < grades.size(); i++) {
            json += "{";
            json += "\"id\": " + std::to_string(grades[i].id) + ",";
            json += "\"student_id\": " + std::to_string(grades[i].student_id) + ",";
            json += "\"subject\": \"" + grades[i].subject + "\",";
            json += "\"grade\": " + std::to_string(grades[i].grade) + ",";
            json += "\"semester\": " + std::to_string(grades[i].semester) + ",";
            json += "\"attendance_percent\": " + std::to_string(grades[i].attendance_percent) + ",";
            json += "\"assignment_completion\": " + std::to_string(grades[i].assignment_completion) + ",";
            json += "\"exam_result\": " + std::to_string(grades[i].exam_result);
            json += "}";
            if (i < grades.size() - 1) json += ",";
        }
        json += "]";
        res.set_content(json, "application/json");
    });
    
    // Админ API: Добавить студента
    svr.Post("/api/admin/students/add", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        auto name = req.get_param_value("name");
        auto surname = req.get_param_value("surname");
        auto group_name = req.get_param_value("group_name");
        
        if (db.addStudent(name, surname, group_name)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    // Админ API: Обновить студента
    svr.Post("/api/admin/students/update", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        int id = std::stoi(req.get_param_value("id"));
        auto name = req.get_param_value("name");
        auto surname = req.get_param_value("surname");
        auto group_name = req.get_param_value("group_name");
        
        if (db.updateStudent(id, name, surname, group_name)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    // Админ API: Удалить студента
    svr.Post("/api/admin/students/delete", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        int id = std::stoi(req.get_param_value("id"));
        
        if (db.deleteStudent(id)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    // Админ API: Добавить оценку
    svr.Post("/api/admin/grades/add", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        int student_id = std::stoi(req.get_param_value("student_id"));
        auto subject = req.get_param_value("subject");
        int grade = std::stoi(req.get_param_value("grade"));
        int semester = std::stoi(req.get_param_value("semester"));
        double attendance = std::stod(req.get_param_value("attendance"));
        double assignment = std::stod(req.get_param_value("assignment"));
        int exam_result = std::stoi(req.get_param_value("exam_result"));
        
        if (db.addGrade(student_id, subject, grade, semester, attendance, assignment, exam_result)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    // Админ API: Обновить оценку
    svr.Post("/api/admin/grades/update", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        int id = std::stoi(req.get_param_value("id"));
        int student_id = std::stoi(req.get_param_value("student_id"));
        auto subject = req.get_param_value("subject");
        int grade = std::stoi(req.get_param_value("grade"));
        int semester = std::stoi(req.get_param_value("semester"));
        double attendance = std::stod(req.get_param_value("attendance"));
        double assignment = std::stod(req.get_param_value("assignment"));
        int exam_result = std::stoi(req.get_param_value("exam_result"));
        
        if (db.updateGrade(id, subject, grade, semester, attendance, assignment, exam_result)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    // Админ API: Удалить оценку
    svr.Post("/api/admin/grades/delete", [&db, &sessions](const httplib::Request& req, httplib::Response& res) {
        auto session_id = req.get_param_value("session_id");
        
        if (sessions.find(session_id) == sessions.end() || sessions[session_id]->role != "admin") {
            res.status = 403;
            res.set_content(R"({"error": "Доступ запрещен"})", "application/json");
            return;
        }
        
        int id = std::stoi(req.get_param_value("id"));
        
        if (db.deleteGrade(id)) {
            res.set_content(R"({"success": true})", "application/json");
        } else {
            res.set_content(R"({"success": false})", "application/json");
        }
    });
    
    std::cout << "Server started on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}

