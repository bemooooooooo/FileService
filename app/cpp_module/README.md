# FileService - C++ File Storage Service

Современный сервис хранения файлов с поддержкой сжатия, JWT аутентификацией и расширенными возможностями управления файлами.

## Основные возможности

### 1. Аутентификация и авторизация
```http
POST /api/auth/register
- Регистрация нового пользователя
- Body: { "username": "string", "password": "string", "email": "string" }

POST /api/auth/login
- Вход в систему
- Body: { "username": "string", "password": "string" }
- Returns: JWT токен

POST /api/auth/refresh
- Обновление JWT токена
- Headers: Authorization: Bearer <token>
- Returns: Новый JWT токен
```

### 2. Управление файлами
```http
POST /api/files/upload
- Загрузка файла
- Headers: Authorization: Bearer <token>
- Multipart form data: file
- Returns: { "uuid": "string", "filename": "string", "size": number }

GET /api/files/{uuid}
- Скачивание файла
- Headers: Authorization: Bearer <token>
- Returns: File stream

DELETE /api/files/{uuid}
- Удаление файла
- Headers: Authorization: Bearer <token>

GET /api/files
- Список файлов пользователя
- Headers: Authorization: Bearer <token>
- Query params: 
  * page: number
  * limit: number
  * sort: "name" | "size" | "date"
  * order: "asc" | "desc"
- Returns: {
    "files": [{
      "uuid": "string",
      "filename": "string",
      "size": number,
      "created_at": "string",
      "is_compressed": boolean,
      "compressed_size": number
    }],
    "total": number,
    "page": number,
    "pages": number
  }
```

### 3. Управление сжатием
```http
POST /api/files/{uuid}/compression/level
- Установка уровня сжатия
- Headers: Authorization: Bearer <token>
- Body: { "level": 0-9 }

GET /api/files/{uuid}/compression/status
- Получение статуса сжатия
- Headers: Authorization: Bearer <token>
- Returns: {
    "status": "PENDING|IN_PROGRESS|COMPLETED|FAILED",
    "error_message": "string",
    "created_at": "string",
    "completed_at": "string"
  }

GET /api/files/compression/stats
- Статистика сжатия
- Headers: Authorization: Bearer <token>
- Returns: {
    "total_files": number,
    "total_bytes_original": number,
    "total_bytes_compressed": number,
    "average_compression_ratio": number,
    "average_compression_time_ms": number,
    "level_usage": {
      "1": number,
      "6": number,
      "9": number
    },
    "file_type_stats": {
      "txt": number,
      "log": number,
      "xml": number
    }
  }
```

### 4. Управление хранилищем
```http
GET /api/storage/quota
- Информация о квоте пользователя
- Headers: Authorization: Bearer <token>
- Returns: {
    "total": number,
    "used": number,
    "available": number,
    "files_count": number
  }

GET /api/storage/types
- Статистика по типам файлов
- Headers: Authorization: Bearer <token>
- Returns: {
    "types": [{
      "extension": "string",
      "count": number,
      "total_size": number,
      "average_size": number
    }]
  }
```

### 5. Системные функции
```http
GET /api/system/health
- Проверка состояния сервиса
- Returns: {
    "status": "UP|DOWN",
    "database": "UP|DOWN",
    "storage": "UP|DOWN",
    "compression": "UP|DOWN"
  }

GET /api/system/metrics
- Метрики системы (только для админов)
- Headers: Authorization: Bearer <token>
- Returns: {
    "users_count": number,
    "files_count": number,
    "storage_used": number,
    "compression_queue": number,
    "cache_size": number,
    "uptime": number
  }
```

## Технические особенности

### Внутренние функции
- Асинхронная обработка сжатия файлов
- Кэширование распакованных файлов
- Автоматическая очистка кэша
- Сбор статистики сжатия
- Мониторинг состояния системы
- Логирование операций
- Обработка ошибок
- Проверка целостности файлов
- Управление временными файлами
- Оптимизация использования памяти

### Особенности реализации
- JWT аутентификация
- Многопоточная обработка
- Асинхронные операции
- Кэширование
- Сжатие файлов (zlib)
- Пул соединений с БД
- Защита от переполнения диска
- Проверка MIME типов
- Валидация файлов
- Безопасное хранение

### Безопасность
- Аутентификация через JWT
- Проверка владельца файла
- Ограничение размера файлов
- Проверка типов файлов
- Защита от SQL инъекций
- Защита от XSS
- Rate limiting
- Безопасное хранение паролей
- Логирование доступа
- Изоляция пользователей

### Оптимизации
- Умное сжатие файлов
- Кэширование частых запросов
- Асинхронная обработка
- Пул потоков
- Оптимизация SQL запросов
- Индексы в базе данных
- Эффективное использование памяти
- Балансировка нагрузки
- Очистка временных файлов
- Оптимизация сетевых запросов

## Зависимости
- C++17 или выше
- Drogon Framework
- PostgreSQL
- zlib
- OpenSSL
- JWT-CPP
- nlohmann/json

## Установка и запуск

### Требования к системе
- CMake 3.15+
- GCC/Clang с поддержкой C++17
- PostgreSQL 12+
- OpenSSL 1.1+

### Сборка проекта
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Настройка окружения
1. Создайте базу данных PostgreSQL
2. Скопируйте `.env.example` в `.env`
3. Настройте параметры в `.env`:
   ```env
   DB_HOST=localhost
   DB_PORT=5432
   DB_NAME=fileservice
   DB_USER=postgres
   DB_PASSWORD=your_password
   JWT_SECRET=your_secret_key
   STORAGE_PATH=/path/to/storage
   ```

### Запуск
```bash
./FileService
```

## Документация
Подробная документация API доступна после запуска по адресу:
`http://localhost:3000/api/docs`

## Разработка

### Структура проекта
```
app/cpp_module/
├── controllers/     # Контроллеры API
├── models/         # Модели данных
├── services/       # Бизнес-логика
├── utils/          # Утилиты
├── config/         # Конфигурация
└── tests/          # Тесты
```

### Добавление новых функций
1. Создайте новый контроллер в `controllers/`
2. Добавьте модели в `models/`
3. Реализуйте бизнес-логику в `services/`
4. Добавьте тесты в `tests/`
5. Обновите документацию

## Тестирование
```bash
cd build
ctest
```

## Лицензия
MIT License

## Авторы
- Команда разработки FileService
