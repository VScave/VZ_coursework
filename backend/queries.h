#ifndef QUERIES_H
#define QUERIES_H

#include <string>

namespace Queries {
    // Users queries
    const std::string CHECK_USER_EXISTS = "SELECT id FROM users WHERE username = $1";
    const std::string CHECK_EMAIL_EXISTS = "SELECT id FROM users WHERE email = $1";
    const std::string INSERT_USER = "INSERT INTO users (username, password, email) VALUES ($1, $2, $3)";
    const std::string INSERT_USER_WITH_ROLE = "INSERT INTO users (username, password, email, role) VALUES ($1, $2, $3, $4)";
    const std::string GET_USER_BY_USERNAME = "SELECT id, username, email, role, password FROM users WHERE username = $1";
    const std::string GET_USER_ROLE = "SELECT role FROM users WHERE id = $1";
    
    // Students queries
    const std::string GET_ALL_STUDENTS = "SELECT id, name, surname, group_name FROM students ORDER BY id";
    const std::string INSERT_STUDENT = "INSERT INTO students (name, surname, group_name) VALUES ($1, $2, $3)";
    const std::string UPDATE_STUDENT = "UPDATE students SET name = $1, surname = $2, group_name = $3 WHERE id = $4";
    const std::string DELETE_STUDENT = "DELETE FROM students WHERE id = $1";
    
    // Grades queries
    const std::string GET_STUDENT_GRADES = "SELECT id, student_id, subject, grade, semester, attendance_percent, assignment_completion, exam_result "
                                           "FROM student_grades WHERE student_id = $1 ORDER BY semester, subject";
    const std::string GET_ALL_GRADES = "SELECT id, student_id, subject, grade, semester, attendance_percent, assignment_completion, exam_result "
                                       "FROM student_grades ORDER BY student_id, semester, subject";
    const std::string INSERT_GRADE = "INSERT INTO student_grades (student_id, subject, grade, semester, attendance_percent, assignment_completion, exam_result) "
                                     "VALUES ($1, $2, $3, $4, $5, $6, $7)";
    const std::string UPDATE_GRADE = "UPDATE student_grades SET subject = $1, grade = $2, semester = $3, attendance_percent = $4, "
                                     "assignment_completion = $5, exam_result = $6 WHERE id = $7";
    const std::string DELETE_GRADE = "DELETE FROM student_grades WHERE id = $1";
}

#endif

