#include "FileController.h"
#include <json/json.h>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

FileController::FileController() {
    dbClient = drogon::app().getDbClient();
    models::FileInfo::createTable(dbClient);
}

void FileController::uploadFile(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto& upload = req->getUploadFile("file");
    if (!upload) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("No file uploaded"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    const std::string& filename = upload->getFileName();
    size_t fileSize = upload->fileLength();

    if (!validateFile(req, filename, fileSize)) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("File validation failed"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string userId = req->getAttributes()->get<std::string>("user_id");
    std::string userPath = getUserStoragePath(userId);
    
    fs::create_directories(userPath);

    std::string fileUuid = generateUuid();
    std::string extension = getFileExtension(filename);
    std::string newFilename = fileUuid + extension;
    std::string fullPath = userPath + "/" + newFilename;

    // Save file to disk
    upload->save(fullPath);

    bool isCompressed = false;
    size_t compressedSize = 0;

    // Check if file should be compressed
    if (shouldCompress(filename, fileSize)) {
        std::string compressedPath = getCompressedFilePath(fullPath);
        if (utils::CompressionUtils::compressFile(fullPath, compressedPath)) {
            compressedSize = utils::CompressionUtils::getCompressedFileSize(compressedPath);
            if (compressedSize < fileSize) {
                fs::remove(fullPath);
                isCompressed = true;
            } else {
                fs::remove(compressedPath);
                compressedSize = 0;
            }
        }
    }

    // Save file info to database
    try {
        if (isCompressed) {
            dbClient->execSqlSync(
                "INSERT INTO file_info (uuid, original_name, user_id, size, compressed_size, is_compressed) "
                "VALUES ($1, $2, $3, $4, $5, $6)",
                fileUuid,
                filename,
                userId,
                fileSize,
                compressedSize,
                true
            );
        } else {
            dbClient->execSqlSync(
                "INSERT INTO file_info (uuid, original_name, user_id, size, is_compressed) "
                "VALUES ($1, $2, $3, $4, $5)",
                fileUuid,
                filename,
                userId,
                fileSize,
                false
            );
        }

        Json::Value result;
        result["uuid"] = fileUuid;
        result["original_name"] = filename;
        result["size"] = static_cast<Json::UInt64>(fileSize);
        if (isCompressed) {
            result["compressed_size"] = static_cast<Json::UInt64>(compressedSize);
            result["compression_ratio"] = static_cast<double>(fileSize) / compressedSize;
        }
        result["is_compressed"] = isCompressed;
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        callback(resp);
    } catch (const drogon::orm::DrogonDbException& e) {
        // If database insert fails, delete the file
        if (isCompressed) {
            fs::remove(getCompressedFilePath(fullPath));
        } else {
            fs::remove(fullPath);
        }
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Failed to save file info"));
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void FileController::getFile(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& uuid) {
    std::string userId = req->getAttributes()->get<std::string>("user_id");
    
    try {
        auto result = dbClient->execSqlSync(
            "SELECT * FROM file_info WHERE uuid = $1 AND user_id = $2",
            uuid,
            userId
        );
        
        if (result.empty()) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("File not found"));
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }

        auto fileInfo = models::FileInfo::fromRow(result[0]);
        std::string basePath = getUserStoragePath(userId) + "/" + uuid + getFileExtension(fileInfo.original_name);
        std::string filePath = fileInfo.is_compressed ? getCompressedFilePath(basePath) : basePath;

        if (!fs::exists(filePath)) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("File not found on disk"));
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }

        if (fileInfo.is_compressed) {
            // Create temporary file for decompressed content
            std::string tempPath = basePath + ".temp";
            if (utils::CompressionUtils::decompressFile(filePath, tempPath)) {
                auto resp = drogon::HttpResponse::newFileResponse(tempPath);
                resp->addHeader("Content-Disposition", "attachment; filename=\"" + fileInfo.original_name + "\"");
                resp->setCallback([tempPath](const drogon::HttpResponsePtr&) {
                    fs::remove(tempPath);
                });
                callback(resp);
                return;
            }
            fs::remove(tempPath);
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Failed to decompress file"));
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
            return;
        }

        auto resp = drogon::HttpResponse::newFileResponse(filePath);
        resp->addHeader("Content-Disposition", "attachment; filename=\"" + fileInfo.original_name + "\"");
        callback(resp);
    } catch (const drogon::orm::DrogonDbException& e) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Database error"));
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void FileController::deleteFile(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              const std::string& uuid) {
    std::string userId = req->getAttributes()->get<std::string>("user_id");
    
    try {
        if (!verifyFileOwnership(uuid, userId)) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("File not found"));
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }

        auto result = dbClient->execSqlSync(
            "SELECT original_name, is_compressed FROM file_info WHERE uuid = $1",
            uuid
        );

        if (!result.empty()) {
            std::string originalName = result[0]["original_name"].as<std::string>();
            bool isCompressed = result[0]["is_compressed"].as<bool>();
            
            std::string basePath = getUserStoragePath(userId) + "/" + uuid + getFileExtension(originalName);
            
            // Delete file from disk
            if (isCompressed) {
                fs::remove(getCompressedFilePath(basePath));
            } else {
                fs::remove(basePath);
            }

            // Delete from database
            dbClient->execSqlSync(
                "DELETE FROM file_info WHERE uuid = $1",
                uuid
            );
        }

        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("File deleted"));
        callback(resp);
    } catch (const drogon::orm::DrogonDbException& e) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Database error"));
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

bool FileController::validateFile(const drogon::HttpRequestPtr& req,
                                const std::string& filename,
                                size_t fileSize) {
    if (fileSize == 0 || fileSize > MAX_FILE_SIZE) {
        return false;
    }

    std::string userId = req->getAttributes()->get<std::string>("user_id");
    
    try {
        auto result = dbClient->execSqlSync(
            "SELECT SUM(size) as total_size FROM file_info WHERE user_id = $1",
            userId
        );

        size_t currentSize = 0;
        if (!result.empty() && !result[0]["total_size"].isNull()) {
            currentSize = result[0]["total_size"].as<size_t>();
        }

        return (currentSize + fileSize) <= MAX_FOLDER_SIZE;
    } catch (const drogon::orm::DrogonDbException& e) {
        return false;
    }
}

bool FileController::verifyFileOwnership(const std::string& uuid, const std::string& userId) {
    try {
        auto result = dbClient->execSqlSync(
            "SELECT COUNT(*) as count FROM file_info WHERE uuid = $1 AND user_id = $2",
            uuid,
            userId
        );
        return !result.empty() && result[0]["count"].as<int>() > 0;
    } catch (...) {
        return false;
    }
}

std::string FileController::getUserStoragePath(const std::string& userId) {
    return "./storage/users/" + userId;
}

std::string FileController::generateUuid() {
    return uuids::to_string(uuids::uuid_system_generator{}());
}

std::string FileController::getFileExtension(const std::string& filename) {
    size_t pos = filename.find_last_of(".");
    return (pos == std::string::npos) ? "" : filename.substr(pos);
}

bool FileController::shouldCompress(
    const std::string& filename,
    size_t fileSize
) const {
    if (fileSize < 1024) { // Файлы меньше 1KB не сжимаем
        return false;
    }

    std::string ext = getFileExtension(filename);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Уже сжатые форматы
    static const std::unordered_set<std::string> compressedFormats = {
        "jpg", "jpeg", "png", "gif", "mp3", "mp4", "zip", "rar", "7z",
        "gz", "bz2", "xz", "pdf", "docx", "xlsx", "pptx"
    };

    if (compressedFormats.find(ext) != compressedFormats.end()) {
        return false;
    }

    // Проверяем эффективность сжатия для этого типа файлов
    double efficiency = services::CompressionStatsService::getInstance()
        .getEfficiencyForFileType(ext);
    
    return efficiency < 0.9; // Сжимаем только если можем достичь >10% сжатия
}

void FileController::setCompressionLevel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& uuid,
    int level
) {
    auto userId = req->getUser();
    if (!verifyFileOwnership(uuid, userId)) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }

    auto compressionLevel = static_cast<models::CompressionLevel>(level);
    if (level < 0 || level > 9) {
        auto resp = HttpResponse::newHttpJsonResponse(
            {{"error", "Invalid compression level. Must be between 0 and 9."}}
        );
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    try {
        auto fileInfo = models::FileInfo::findByUuid(dbClient, uuid);
        scheduleCompression(fileInfo, compressionLevel);

        auto resp = HttpResponse::newHttpJsonResponse(
            {{"message", "Compression task scheduled"}}
        );
        callback(resp);
    } catch (const std::exception& e) {
        auto resp = HttpResponse::newHttpJsonResponse(
            {{"error", e.what()}}
        );
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void FileController::getCompressionStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& uuid
) {
    auto userId = req->getUser();
    if (!verifyFileOwnership(uuid, userId)) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }

    try {
        auto result = dbClient->execSqlSync(
            "SELECT * FROM compression_tasks WHERE file_uuid = $1",
            uuid
        );

        if (result.size() == 0) {
            auto resp = HttpResponse::newHttpJsonResponse(
                {{"status", "NO_COMPRESSION_TASK"}}
            );
            callback(resp);
            return;
        }

        auto task = models::CompressionTask::fromRow(result[0]);
        callback(HttpResponse::newHttpJsonResponse(task.toJson()));
    } catch (const std::exception& e) {
        auto resp = HttpResponse::newHttpJsonResponse(
            {{"error", e.what()}}
        );
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void FileController::getCompressionStats(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
    try {
        auto stats = services::CompressionStatsService::getInstance().getStatsJson();
        callback(HttpResponse::newHttpJsonResponse(stats));
    } catch (const std::exception& e) {
        auto resp = HttpResponse::newHttpJsonResponse(
            {{"error", e.what()}}
        );
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void FileController::scheduleCompression(
    const models::FileInfo& fileInfo,
    models::CompressionLevel level
) {
    models::CompressionTask task;
    task.id = generateUuid();
    task.file_uuid = fileInfo.uuid;
    task.user_id = fileInfo.user_id;
    task.input_path = fileInfo.path;
    task.output_path = getCompressedFilePath(fileInfo.path);
    task.level = level;
    task.status = models::CompressionStatus::PENDING;

    dbClient->execSqlSync(
        "INSERT INTO compression_tasks "
        "(id, file_uuid, user_id, input_path, output_path, compression_level, status) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7)",
        task.id,
        task.file_uuid,
        task.user_id,
        task.input_path,
        task.output_path,
        static_cast<int>(task.level),
        "PENDING"
    );

    utils::AsyncTaskManager::getInstance().addCompressionTask(task);
}

std::string FileController::getCompressedFilePath(const std::string& originalPath) {
    return originalPath + ".gz";
}
