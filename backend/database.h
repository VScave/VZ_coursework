#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

struct Student {
    int id;
    std::string name;
    std::string surname;
    std::string group_name;
};

struct Grade {
    int id;
    int student_id;
    std::string subject;
    int grade;
    int semester;
    double attendance_percent;
    double assignment_completion;
    int exam_result;
};

struct User {
    int id;
    std::string username;
    std::string email;
    std::string role;
};

class Database {
private:
    std::string connection_string;
    
public:
    Database(const std::string& conn_str);
    ~Database();
    
    // Пользователи
    bool registerUser(const std::string& username, const std::string& password, const std::string& email);
    bool registerUserWithRole(const std::string& username, const std::string& password, const std::string& email, const std::string& role);
    User* authenticateUser(const std::string& username, const std::string& password);
    bool isAdmin(int user_id);
    bool createDefaultAdmin(); // Создать тестового админа (admin/admin)
    
    // Студенты
    std::vector<Student> getAllStudents();
    bool addStudent(const std::string& name, const std::string& surname, const std::string& group_name);
    bool updateStudent(int id, const std::string& name, const std::string& surname, const std::string& group_name);
    bool deleteStudent(int id);
    
    // Оценки
    std::vector<Grade> getStudentGrades(int student_id);
    std::vector<Grade> getAllGrades();
    bool addGrade(int student_id, const std::string& subject, int grade, int semester, 
                  double attendance, double assignment, int exam_result);
    bool updateGrade(int id, const std::string& subject, int grade, int semester,
                     double attendance, double assignment, int exam_result);
    bool deleteGrade(int id);
    
    // Прогноз
    std::string predictExamSuccess(int student_id);
};

#endif

