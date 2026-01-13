# Многоэтапная сборка для C++ приложения
FROM ubuntu:22.04 AS builder

# Установка зависимостей для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    libpqxx-dev \
    libpq-dev \
    libssl-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Установка httplib.h
RUN curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o /tmp/httplib.h

# Рабочая директория
WORKDIR /build

# Копирование исходного кода
COPY backend/ ./backend/
COPY frontend/ ./frontend/
COPY Makefile ./

# Копирование httplib.h
RUN cp /tmp/httplib.h ./backend/httplib.h

# Сборка приложения напрямую (без использования Makefile с macOS флагами)
RUN mkdir -p build && \
    g++ -std=c++17 -Wall -O2 -c backend/password_hash.cpp -o build/password_hash.o -Ibackend && \
    g++ -std=c++17 -Wall -O2 -c backend/database.cpp -o build/database.o -Ibackend && \
    g++ -std=c++17 -Wall -O2 -c backend/server.cpp -o build/server.o -Ibackend && \
    g++ build/password_hash.o build/database.o build/server.o -o server -lpqxx -lpq -lssl -lcrypto && \
    ls -la && \
    test -f server && echo "Сборка успешна: server найден" || (echo "Ошибка: server не найден" && exit 1)

# Финальный образ
FROM ubuntu:22.04

# Установка только runtime зависимостей
RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libpq-dev \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Создание пользователя для запуска приложения
RUN useradd -m -u 1000 appuser

# Рабочая директория
WORKDIR /app

# Копирование скомпилированного приложения
COPY --from=builder /build/server ./server
COPY --chown=appuser:appuser frontend/ ./frontend/

# Права на выполнение
RUN chmod +x ./server

# Переключение на непривилегированного пользователя
USER appuser

# Открываем порт
EXPOSE 8080

# Запуск приложения
CMD ["./server"]

