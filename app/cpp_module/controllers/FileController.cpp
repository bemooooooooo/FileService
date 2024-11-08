#include "FileController.h"

std::string FileController::StoreFile(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
      /// TODO: организовать хранение файлов + валидация
  return std::string();
}

std::string FileController::GetFile(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& name) {
      /// TODO: Написать быстрый поиск по файлам (скорее всего бинарный 
      /// или можно организовать дерево поиска но надо ебаться с хранением тогда)
  return std::string();
}
