#pragma once
#include "drogon/HttpController.h"
#include "objects/File.h"

class FileController : public drogon::HttpController<FileController> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(FileController::StoreFile, "/storeFile", drogon::Post);
  METHOD_ADD(FileController::GetFile, "/getFile?name={1}", drogon::Get);
  METHOD_LIST_END

 public:
  /// @brief По запросу POST размещает файл в файловом хранилище
  /// @param req
  /// @param callback
  /// @return url к файлу
  std::string StoreFile(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

  /// @brief По запросу GET возвращает файл
  /// @param req
  /// @param callback
  /// @param name Название файла
  /// @return url к файлу
  std::string GetFile(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
      const std::string &name);
  
  bool validate(const File& file);

}; // Class FileController