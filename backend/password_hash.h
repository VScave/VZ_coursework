#ifndef PASSWORD_HASH_H
#define PASSWORD_HASH_H

#include <string>

namespace PasswordHash {
    // Генерация хеша пароля с солью
    std::string hashPassword(const std::string& password);
    
    // Проверка пароля (сравнение с хешем из БД)
    bool verifyPassword(const std::string& password, const std::string& hash);
    
    // Генерация случайной соли
    std::string generateSalt();
}

#endif

