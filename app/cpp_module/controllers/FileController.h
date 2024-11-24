#pragma once
#include "drogon/HttpController.h"
#include "../utils/JWTUtils.hpp"
#include "../utils/CompressionUtils.hpp"
#include "../models/FileInfo.h"
#include <filesystem>
#include <uuid.h>

#define MAX_FILE_SIZE 62914560    // 60MB
#define MAX_FOLDER_SIZE 524288000 // 500MB
#define COMPRESSION_THRESHOLD 1048576 // 1MB

class FileController : public drogon::HttpController<FileController> {
public:
    FileController();
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(FileController::uploadFile, "/api/files/upload", {Post, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::getFile, "/api/files/{uuid}", {Get, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::listFiles, "/api/files", {Get, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::deleteFile, "/api/files/{uuid}", {Delete, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::setCompressionLevel, "/api/files/{uuid}/compression/level", {Post, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::getCompressionStatus, "/api/files/{uuid}/compression/status", {Get, "JWTAuthFilter"});
    ADD_METHOD_TO(FileController::getCompressionStats, "/api/files/compression/stats", {Get, "JWTAuthFilter"});
    METHOD_LIST_END

    void uploadFile(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    void getFile(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 const std::string& uuid);
    
    void listFiles(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void deleteFile(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   const std::string& uuid);

    void setCompressionLevel(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& uuid,
                           int level);

    void getCompressionStatus(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const std::string& uuid);

    void getCompressionStats(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    drogon::orm::DbClientPtr dbClient;
    
    bool validateFile(const drogon::HttpRequestPtr& req, 
                     const std::string& filename, 
                     size_t fileSize);
    
    std::string getUserStoragePath(const std::string& userId);
    std::string generateUuid();
    std::string getFileExtension(const std::string& filename);
    bool verifyFileOwnership(const std::string& uuid, const std::string& userId);
    bool shouldCompress(const std::string& filename, size_t fileSize) const;
    std::string getCompressedFilePath(const std::string& originalPath) const;
    void scheduleCompression(const models::FileInfo& fileInfo, 
                           models::CompressionLevel level);
};