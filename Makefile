CXX = g++
# Определение пути к OpenSSL (для macOS через Homebrew)
OPENSSL_PREFIX := $(shell brew --prefix openssl 2>/dev/null || echo "")
ifeq ($(OPENSSL_PREFIX),)
    # Если brew не найден, используем стандартные пути (пустые, если OpenSSL в стандартных местах)
    INCLUDE_FLAGS = -I/usr/local/opt/openssl/include -I/opt/homebrew/opt/openssl/include
    LIB_FLAGS = -L/usr/local/opt/openssl/lib -L/opt/homebrew/opt/openssl/lib
else
    INCLUDE_FLAGS = -I$(OPENSSL_PREFIX)/include
    LIB_FLAGS = -L$(OPENSSL_PREFIX)/lib
endif

# Фильтруем пустые пути
INCLUDE_FLAGS := $(strip $(INCLUDE_FLAGS))
LIB_FLAGS := $(strip $(LIB_FLAGS))

CXXFLAGS = -std=c++17 -Wall -O2 $(if $(INCLUDE_FLAGS),$(INCLUDE_FLAGS))
LDFLAGS = $(if $(LIB_FLAGS),$(LIB_FLAGS)) -lpqxx -lpq -lssl -lcrypto
TARGET = server
BACKEND_DIR = backend
BUILD_DIR = build

# Скачать httplib.h если его нет
httplib.h:
	@echo "Скачивание httplib.h..."
	@curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o $(BACKEND_DIR)/httplib.h || \
	 (echo "Ошибка скачивания httplib.h. Пожалуйста, скачайте вручную:" && \
	  echo "curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o $(BACKEND_DIR)/httplib.h")

# Проверка наличия httplib.h
check-httplib:
	@test -f $(BACKEND_DIR)/httplib.h || (echo "Ошибка: httplib.h не найден. Выполните: make httplib.h" && exit 1)

# Создание директории для сборки
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Компиляция
all: check-httplib $(BUILD_DIR)
	@echo "Компиляция password_hash.cpp..."
	$(CXX) $(CXXFLAGS) -c $(BACKEND_DIR)/password_hash.cpp -o $(BUILD_DIR)/password_hash.o -I$(BACKEND_DIR)
	@echo "Компиляция database.cpp..."
	$(CXX) $(CXXFLAGS) -c $(BACKEND_DIR)/database.cpp -o $(BUILD_DIR)/database.o -I$(BACKEND_DIR)
	@echo "Компиляция server.cpp..."
	$(CXX) $(CXXFLAGS) -c $(BACKEND_DIR)/server.cpp -o $(BUILD_DIR)/server.o -I$(BACKEND_DIR)
	@echo "Линковка..."
	$(CXX) $(BUILD_DIR)/password_hash.o $(BUILD_DIR)/database.o $(BUILD_DIR)/server.o -o $(TARGET) $(LDFLAGS)
	@echo "Сборка завершена: запуск из корня проекта: ./$(TARGET)"

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Очистка завершена"

# Установка зависимостей (macOS)
install-deps-macos:
	@echo "Установка зависимостей для macOS..."
	@brew install postgresql libpqxx openssl || echo "Ошибка: убедитесь, что установлен Homebrew"

# Установка зависимостей (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "Установка зависимостей для Ubuntu/Debian..."
	@sudo apt-get update && sudo apt-get install -y libpqxx-dev postgresql-server-dev-all build-essential libssl-dev || echo "Ошибка установки"

.PHONY: all clean httplib.h check-httplib install-deps-macos install-deps-ubuntu

