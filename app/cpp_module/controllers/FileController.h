#pragma once
#include "drogon/HttpController.h"

using namespace drogon;

class FileController : public drogon::HttpController<FileController> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(FileController::StoreFile, "/storeFile", Post);
  METHOD_ADD(FileController::GetFile, "/getFile?name={1}", Get);
  METHOD_LIST_END
  
 public:

  /// @brief По запросу POST размещает файл в файловом хранилище
  /// @param req 
  /// @param callback 
  /// @return url к файлу 
  std::string StoreFile(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback);

  /// @brief По запросу GET возвращает файл
  /// @param req 
  /// @param callback 
  /// @param name Название файла
  /// @return url к файлу
  std::string GetFile(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback,
                      const std::string &name);
};