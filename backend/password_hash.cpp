#include "password_hash.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>

namespace PasswordHash {
    
    // Преобразование бинарных данных в hex строку
    std::string toHex(const unsigned char* data, size_t length) {
        std::stringstream ss;
        for (size_t i = 0; i < length; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        }
        return ss.str();
    }
    
    // Генерация случайной соли
    std::string generateSalt() {
        unsigned char salt[16];
        if (RAND_bytes(salt, sizeof(salt)) != 1) {
            // Если RAND_bytes не работает, используем простую соль
            std::string simpleSalt = "default_salt_2024";
            return simpleSalt;
        }
        return toHex(salt, sizeof(salt));
    }
    
    // Хеширование пароля с солью (SHA-256)
    std::string hashPassword(const std::string& password) {
        // Генерируем соль
        std::string salt = generateSalt();
        
        // Объединяем пароль и соль
        std::string saltedPassword = password + salt;
        
        // Вычисляем SHA-256 хеш
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.length());
        SHA256_Final(hash, &sha256);
        
        // Преобразуем хеш в hex строку
        std::string hashHex = toHex(hash, SHA256_DIGEST_LENGTH);
        
        // Возвращаем соль и хеш в формате: salt:hash
        return salt + ":" + hashHex;
    }
    
    // Проверка пароля (сравнение с хешем из БД)
    bool verifyPassword(const std::string& password, const std::string& storedHash) {
        // Парсим сохраненный хеш (формат: salt:hash)
        size_t colonPos = storedHash.find(':');
        if (colonPos == std::string::npos) {
            // Старый формат без хеширования (для обратной совместимости)
            return password == storedHash;
        }
        
        std::string salt = storedHash.substr(0, colonPos);
        std::string storedHashValue = storedHash.substr(colonPos + 1);
        
        // Вычисляем хеш введенного пароля с той же солью
        std::string saltedPassword = password + salt;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.length());
        SHA256_Final(hash, &sha256);
        
        std::string computedHash = toHex(hash, SHA256_DIGEST_LENGTH);
        
        // Сравниваем хеши
        return computedHash == storedHashValue;
    }
}

