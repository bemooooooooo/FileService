#include "FileController.h"

std::string FileController::StoreFile(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
  /// TODO: организовать хранение файлов + валидация
  return std::string();
}

std::string FileController::GetFile(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string& name) {
  /// TODO: Написать быстрый поиск по файлам (скорее всего бинарный
  /// или можно организовать дерево поиска но надо ебаться с хранением тогда)
  return std::string();
}

bool FileController::validate(const File& file) {
  if(file.size <= )  
  return false;
}
