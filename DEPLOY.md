# Инструкция по развертыванию на Linux сервере

## Установка Docker

### Ubuntu/Debian

```bash
# Обновление списка пакетов
sudo apt-get update

# Установка необходимых зависимостей
sudo apt-get install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

# Добавление официального GPG ключа Docker
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# Настройка репозитория Docker
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# Установка Docker Engine
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Проверка установки
sudo docker --version
sudo docker compose version
```

### CentOS/RHEL/Fedora

```bash
# Установка необходимых пакетов
sudo yum install -y yum-utils

# Добавление репозитория Docker
sudo yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo

# Установка Docker Engine
sudo yum install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Запуск Docker
sudo systemctl start docker
sudo systemctl enable docker

# Проверка установки
sudo docker --version
sudo docker compose version
```

### Альтернативный способ (универсальный скрипт)

```bash
# Скачать и запустить официальный скрипт установки Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

# Добавить текущего пользователя в группу docker (чтобы не использовать sudo)
sudo usermod -aG docker $USER

# Выйти и войти снова, или выполнить:
newgrp docker

# Проверка установки
docker --version
docker compose version
```

## Настройка прав доступа (опционально, но рекомендуется)

Чтобы не использовать `sudo` для каждой команды Docker:

```bash
# Добавить пользователя в группу docker
sudo usermod -aG docker $USER

# Применить изменения (выйти и войти снова, или выполнить):
newgrp docker

# Проверка (должно работать без sudo)
docker ps
```

## Развертывание проекта

### 1. Загрузить проект на сервер

Если проект еще не загружен:

```bash
# Клонировать репозиторий (если используете git)
git clone <ваш-репозиторий>
cd curs-vad

# Или загрузить файлы через scp/sftp
```

### 2. Перейти в директорию проекта

```bash
cd curs-vad
```

### 3. Запустить проект

```bash
# Запустить все сервисы (PostgreSQL + Backend)
docker compose up -d

# Или если используется старая версия Docker Compose:
docker-compose up -d
```

### 4. Проверить статус

```bash
# Проверить запущенные контейнеры
docker compose ps

# Просмотр логов
docker compose logs -f

# Логи только бэкенда
docker compose logs -f backend

# Логи только базы данных
docker compose logs -f postgres
```

## Полезные команды

### Управление контейнерами

```bash
# Остановить все сервисы
docker compose stop

# Запустить снова
docker compose start

# Перезапустить
docker compose restart

# Остановить и удалить контейнеры
docker compose down

# Остановить и удалить контейнеры + данные БД
docker compose down -v
```

### Пересборка после изменений

```bash
# Пересобрать образы
docker compose build

# Пересобрать и перезапустить
docker compose up -d --build
```

### Подключение к базе данных

```bash
# Подключиться к PostgreSQL контейнеру
docker compose exec postgres psql -U postgres -d exam_prediction

# Или извне (если порт 5432 открыт)
psql -h localhost -U postgres -d exam_prediction
```

## Настройка файрвола

Если нужно открыть порты для внешнего доступа:

```bash
# Ubuntu/Debian (ufw)
sudo ufw allow 8080/tcp
sudo ufw allow 5432/tcp  # Только если нужен прямой доступ к БД
sudo ufw reload

# CentOS/RHEL (firewalld)
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --permanent --add-port=5432/tcp
sudo firewall-cmd --reload
```

## Проверка работы

После запуска приложение будет доступно по адресу:
- `http://<IP_СЕРВЕРА>:8080`
- `http://localhost:8080` (если подключаетесь локально)

### Тестовые учетные данные

- **Логин:** `admin`
- **Пароль:** `admin`
- **Email:** `admin@example.com`

## Решение проблем

### Docker не запускается

```bash
# Проверить статус службы
sudo systemctl status docker

# Запустить службу
sudo systemctl start docker
sudo systemctl enable docker
```

### Проблемы с правами доступа

```bash
# Добавить пользователя в группу docker
sudo usermod -aG docker $USER
newgrp docker
```

### Ошибки при сборке

```bash
# Очистить кэш Docker
docker system prune -a

# Пересобрать без кэша
docker compose build --no-cache
```

### Просмотр логов для отладки

```bash
# Все логи
docker compose logs

# Логи конкретного сервиса
docker compose logs backend
docker compose logs postgres

# Следить за логами в реальном времени
docker compose logs -f
```

## Автозапуск при перезагрузке сервера

Docker Compose автоматически запускает контейнеры с флагом `restart: unless-stopped` в `docker-compose.yml`, поэтому контейнеры будут автоматически запускаться при перезагрузке сервера.

Если нужно убедиться:

```bash
# Проверить политику перезапуска
docker compose ps

# Вручную установить политику перезапуска
docker update --restart unless-stopped exam_prediction_backend
docker update --restart unless-stopped exam_prediction_db
```

## Резервное копирование базы данных

```bash
# Создать бэкап БД
docker compose exec postgres pg_dump -U postgres exam_prediction > backup_$(date +%Y%m%d_%H%M%S).sql

# Восстановить из бэкапа
docker compose exec -T postgres psql -U postgres exam_prediction < backup_YYYYMMDD_HHMMSS.sql
```

