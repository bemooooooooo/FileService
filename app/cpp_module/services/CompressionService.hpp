#pragma once
#include <string>
#include <chrono>
#include "../utils/CompressionUtils.hpp"
#include "../utils/AsyncTaskManager.hpp"
#include "../utils/CacheManager.hpp"
#include "../models/CompressionTask.h"
#include "CompressionStatsService.hpp"

namespace services {
    class CompressionService {
    public:
        static CompressionService& getInstance() {
            static CompressionService instance;
            return instance;
        }

        void processTask(models::CompressionTask& task) {
            auto start = std::chrono::high_resolution_clock::now();

            try {
                task.status = models::CompressionStatus::IN_PROGRESS;
                updateTaskStatus(task);

                size_t originalSize = std::filesystem::file_size(task.input_path);
                
                if (utils::CompressionUtils::compressFile(
                    task.input_path,
                    task.output_path,
                    static_cast<int>(task.level)
                )) {
                    size_t compressedSize = std::filesystem::file_size(task.output_path);
                    
                    // Если сжатие не дало существенного выигрыша, отменяем его
                    if (compressedSize >= originalSize * 0.9) {
                        std::filesystem::remove(task.output_path);
                        task.status = models::CompressionStatus::COMPLETED;
                        task.error_message = "Compression skipped: no significant size reduction";
                    } else {
                        // Обновляем информацию о файле
                        updateFileInfo(task.file_uuid, compressedSize, true);
                        task.status = models::CompressionStatus::COMPLETED;
                        
                        // Записываем статистику
                        auto end = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end - start
                        );
                        
                        std::string fileType = getFileExtension(task.input_path);
                        CompressionStatsService::getInstance().recordCompression(
                            fileType,
                            originalSize,
                            compressedSize,
                            task.level,
                            duration
                        );
                    }
                } else {
                    task.status = models::CompressionStatus::FAILED;
                    task.error_message = "Compression failed";
                }
            } catch (const std::exception& e) {
                task.status = models::CompressionStatus::FAILED;
                task.error_message = e.what();
            }

            task.completed_at = getCurrentTimestamp();
            updateTaskStatus(task);
        }

    private:
        CompressionService() = default;

        void updateTaskStatus(const models::CompressionTask& task) {
            auto dbClient = drogon::app().getDbClient();
            dbClient->execSqlSync(
                "UPDATE compression_tasks SET status = $1, error_message = $2, completed_at = $3 "
                "WHERE id = $4",
                statusToString(task.status),
                task.error_message,
                task.completed_at,
                task.id
            );
        }

        void updateFileInfo(const std::string& uuid, size_t compressedSize, bool isCompressed) {
            auto dbClient = drogon::app().getDbClient();
            dbClient->execSqlSync(
                "UPDATE file_info SET compressed_size = $1, is_compressed = $2 "
                "WHERE uuid = $3",
                compressedSize,
                isCompressed,
                uuid
            );
        }

        std::string getCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }

        std::string getFileExtension(const std::string& path) {
            size_t pos = path.find_last_of(".");
            return (pos == std::string::npos) ? "" : path.substr(pos + 1);
        }

        std::string statusToString(models::CompressionStatus status) {
            switch (status) {
                case models::CompressionStatus::PENDING: return "PENDING";
                case models::CompressionStatus::IN_PROGRESS: return "IN_PROGRESS";
                case models::CompressionStatus::COMPLETED: return "COMPLETED";
                case models::CompressionStatus::FAILED: return "FAILED";
                default: return "UNKNOWN";
            }
        }
    };
}
