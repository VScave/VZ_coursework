# Система прогнозирования качества сдачи экзаменов

Простой многофайловый проект на C++ для прогнозирования успешности сдачи экзаменов студентами с веб-интерфейсом и PostgreSQL базой данных.

## Структура проекта

```
VZ_coursework/
├── backend/           # C++ бэкенд
│   ├── database.h     # Заголовочный файл для работы с БД
│   ├── database.cpp   # Реализация работы с PostgreSQL
│   ├── password_hash.h    # Заголовочный файл для хеширования паролей
│   ├── password_hash.cpp  # Реализация хеширования паролей (SHA-256 с солью)
│   ├── queries.h      # SQL запросы (вынесены из кода)
│   ├── server.cpp     # HTTP сервер (использует cpp-httplib)
│   └── httplib.h      # HTTP библиотека (нужно скачать)
├── frontend/          # Веб-интерфейс
│   ├── index.html     # Главная страница
│   ├── login.html     # Страница входа
│   ├── register.html  # Страница регистрации
│   ├── style.css      # Стили
│   └── app.js         # JavaScript логика
├── database/          # SQL скрипты
│   └── init.sql       # Инициализация БД
├── docker-compose.yml # Docker Compose конфигурация
├── Dockerfile         # Docker образ для бэкенда
├── Makefile           # Файл сборки
└── README.md          # Этот файл
```

## Требования

- C++ компилятор с поддержкой C++17 (g++, clang++)
- PostgreSQL (версия 12+)
- libpqxx (C++ библиотека для PostgreSQL)
- OpenSSL (для хеширования паролей - обычно уже установлен на Linux, на macOS через Homebrew)
- cpp-httplib (header-only HTTP библиотека)

## Установка зависимостей

### macOS

```bash
# Установка PostgreSQL, libpqxx и OpenSSL через Homebrew
brew install postgresql libpqxx openssl

# Скачать httplib.h
curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o backend/httplib.h
```

### Ubuntu/Debian

```bash
# Установка зависимостей
sudo apt-get update
sudo apt-get install -y libpqxx-dev postgresql-server-dev-all build-essential libssl-dev

# Скачать httplib.h
curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o backend/httplib.h
```

## Быстрый запуск с Docker (Рекомендуется)

Самый простой способ запустить проект - использовать Docker Compose:

```bash
# Запуск всех сервисов (PostgreSQL + Backend)
docker-compose up -d

# Просмотр логов
docker-compose logs -f

# Остановка сервисов
docker-compose down

# Остановка с удалением данных
docker-compose down -v
```

После запуска приложение будет доступно на http://localhost:8080

**Преимущества Docker:**
- Автоматическая установка всех зависимостей
- Изолированная среда
- Не нужно устанавливать PostgreSQL локально
- Работает на любой ОС

## Ручная установка

### Настройка базы данных

1. Запустите PostgreSQL сервер:
```bash
# macOS
brew services start postgresql

# Ubuntu/Debian
sudo systemctl start postgresql
```

2. Создайте базу данных и выполните скрипт инициализации:
```bash
# Войти в PostgreSQL как суперпользователь
psql -U postgres

# В PostgreSQL выполнить:
\i database/init.sql
```

Или выполнить скрипт напрямую:
```bash
psql -U postgres -f database/init.sql
```

**Важно:** Сервер автоматически использует переменные окружения для подключения к БД. Если они не установлены, используются значения по умолчанию:
- DB_HOST=localhost
- DB_PORT=5432
- DB_NAME=exam_prediction
- DB_USER=postgres
- DB_PASSWORD=postgres

## Сборка проекта

Используйте Makefile:

```bash
# Скачать httplib.h и собрать проект
make httplib.h
make

# Или просто собрать (если httplib.h уже есть)
make
```

Или вручную:

```bash
cd backend
g++ -std=c++17 -Wall -O2 -c password_hash.cpp -o password_hash.o
g++ -std=c++17 -Wall -O2 -c database.cpp -o database.o -I.
g++ -std=c++17 -Wall -O2 -c server.cpp -o server.o -I.
g++ password_hash.o database.o server.o -o ../server -lpqxx -lpq -lssl -lcrypto
cd ..
```

## Запуск

**Важно:** Запускайте сервер из корня проекта (где находится Makefile):

```bash
# Из корня проекта
./server
```

Сервер запустится на `http://localhost:8080`

Откройте браузер и перейдите по адресу: http://localhost:8080

## Использование Docker

### Запуск с Docker Compose

```bash
# Запустить все сервисы
docker-compose up -d

# Просмотр логов
docker-compose logs -f backend
docker-compose logs -f postgres

# Остановить сервисы
docker-compose stop

# Запустить снова
docker-compose start

# Полная остановка и удаление контейнеров
docker-compose down

# Остановка с удалением всех данных (включая БД)
docker-compose down -v
```

### Пересборка образа

```bash
# Пересобрать образ после изменений в коде
docker-compose build

# Пересобрать и перезапустить
docker-compose up -d --build
```

### Подключение к БД в Docker

```bash
# Подключиться к PostgreSQL контейнеру
docker-compose exec postgres psql -U postgres -d exam_prediction

# Или извне (если PostgreSQL слушает на localhost:5432)
psql -h localhost -U postgres -d exam_prediction
```

## Использование

### Тестовые учетные записи

**Тестовый админ создается автоматически при первом запуске сервера:**

- **Логин:** `admin`
- **Пароль:** `admin`
- **Email:** `admin@example.com`
- **Роль:** `admin`

Если админ уже существует, он не будет пересоздан.

**Обычные пользователи:**
Регистрируйте через веб-интерфейс на странице `/register`. Все новые пользователи создаются с ролью `user` по умолчанию.

Пароли автоматически хешируются при регистрации с использованием SHA-256 и соли.

### Функционал

1. **Регистрация и вход:**
   - Перейдите на `/register` для создания нового аккаунта
   - Войдите через `/login`

2. **Просмотр прогноза:**
   - На главной странице выберите студента
   - Нажмите "Получить прогноз"
   - Система покажет прогнозируемую оценку на основе исторических данных

3. **Админ-панель:**
   - Войдите как администратор
   - Нажмите кнопку "Админ-панель"
   - Управляйте студентами: добавление, редактирование, удаление
   - Управляйте оценками: добавление, редактирование, удаление

## API Endpoints

- `POST /api/register` - Регистрация нового пользователя
- `POST /api/login` - Вход в систему
- `GET /api/students?session_id=...` - Получить список студентов
- `GET /api/predict?session_id=...&student_id=...` - Получить прогноз для студента
- `GET /api/grades?session_id=...` - Получить все оценки (требуется авторизация)
- `POST /api/admin/students/add` - Добавить студента (только админ)
- `POST /api/admin/students/update` - Обновить студента (только админ)
- `POST /api/admin/students/delete` - Удалить студента (только админ)
- `POST /api/admin/grades/add` - Добавить оценку (только админ)
- `POST /api/admin/grades/update` - Обновить оценку (только админ)
- `POST /api/admin/grades/delete` - Удалить оценку (только админ)

## Алгоритм прогнозирования

Система использует взвешенную формулу на основе трех факторов:

### Формула прогноза:

```
prediction_score = (средняя_оценка × 50%) + (посещаемость% / 20 × 25%) + (выполнение_заданий% / 20 × 25%)
```

### Факторы:

1. **Средняя оценка по всем предметам (вес 50%)** - основной показатель успеваемости
2. **Средняя посещаемость занятий (вес 25%)** - показатель регулярности посещения
3. **Средний процент выполнения заданий (вес 25%)** - показатель ответственности

### Определение оценки:

- **prediction_score ≥ 4.5** → Отлично (5)
- **3.5 ≤ score < 4.5** → Хорошо (4)
- **2.5 ≤ score < 3.5** → Удовлетворительно (3)
- **score < 2.5** → Неудовлетворительно (2)

**Подробное описание алгоритма:** см. файл [ALGORITHM.md](ALGORITHM.md)

## Примечания

⚠️ **Важно:** Это учебный проект. Для продакшена рекомендуется:
- ✅ Хеширование паролей (реализовано: SHA-256 с солью)
- JWT токены для сессий вместо простых session_id (сейчас используются простые session_id)
- ✅ Защита от SQL-инъекций (используется параметризованные запросы через libpqxx - уже реализовано)
- HTTPS для безопасности
- Валидация входных данных
- Логирование ошибок
- Использовать bcrypt или argon2 вместо SHA-256 (SHA-256 быстрее, но bcrypt/argon2 специально разработаны для паролей)

## Лицензия

Образовательный проект.

