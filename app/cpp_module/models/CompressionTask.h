#pragma once
#include <string>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <json/json.h>

namespace models {
    enum class CompressionLevel {
        NONE = 0,
        FAST = 1,
        BALANCED = 6,
        MAX = 9
    };

    enum class CompressionStatus {
        PENDING,
        IN_PROGRESS,
        COMPLETED,
        FAILED
    };

    class CompressionTask {
    public:
        std::string id;
        std::string file_uuid;
        std::string user_id;
        std::string input_path;
        std::string output_path;
        CompressionLevel level;
        CompressionStatus status;
        std::string error_message;
        std::string created_at;
        std::string completed_at;

        static bool createTable(drogon::orm::DbClientPtr& client) {
            try {
                client->execSqlSync(
                    "CREATE TABLE IF NOT EXISTS compression_tasks ("
                    "id VARCHAR(36) PRIMARY KEY,"
                    "file_uuid VARCHAR(36) NOT NULL,"
                    "user_id VARCHAR(36) NOT NULL,"
                    "input_path TEXT NOT NULL,"
                    "output_path TEXT NOT NULL,"
                    "compression_level INTEGER NOT NULL,"
                    "status VARCHAR(20) NOT NULL,"
                    "error_message TEXT,"
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                    "completed_at TIMESTAMP,"
                    "CONSTRAINT fk_file FOREIGN KEY (file_uuid) REFERENCES file_info(uuid) ON DELETE CASCADE,"
                    "CONSTRAINT fk_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
                    ")"
                );
                return true;
            } catch (...) {
                return false;
            }
        }

        static CompressionTask fromRow(const drogon::orm::Row& row) {
            CompressionTask task;
            task.id = row["id"].as<std::string>();
            task.file_uuid = row["file_uuid"].as<std::string>();
            task.user_id = row["user_id"].as<std::string>();
            task.input_path = row["input_path"].as<std::string>();
            task.output_path = row["output_path"].as<std::string>();
            task.level = static_cast<CompressionLevel>(row["compression_level"].as<int>());
            task.status = stringToStatus(row["status"].as<std::string>());
            if (!row["error_message"].isNull()) {
                task.error_message = row["error_message"].as<std::string>();
            }
            task.created_at = row["created_at"].as<std::string>();
            if (!row["completed_at"].isNull()) {
                task.completed_at = row["completed_at"].as<std::string>();
            }
            return task;
        }

        Json::Value toJson() const {
            Json::Value json;
            json["id"] = id;
            json["file_uuid"] = file_uuid;
            json["compression_level"] = static_cast<int>(level);
            json["status"] = statusToString(status);
            if (!error_message.empty()) {
                json["error_message"] = error_message;
            }
            json["created_at"] = created_at;
            if (!completed_at.empty()) {
                json["completed_at"] = completed_at;
            }
            return json;
        }

    private:
        static std::string statusToString(CompressionStatus status) {
            switch (status) {
                case CompressionStatus::PENDING: return "PENDING";
                case CompressionStatus::IN_PROGRESS: return "IN_PROGRESS";
                case CompressionStatus::COMPLETED: return "COMPLETED";
                case CompressionStatus::FAILED: return "FAILED";
                default: return "UNKNOWN";
            }
        }

        static CompressionStatus stringToStatus(const std::string& status) {
            if (status == "PENDING") return CompressionStatus::PENDING;
            if (status == "IN_PROGRESS") return CompressionStatus::IN_PROGRESS;
            if (status == "COMPLETED") return CompressionStatus::COMPLETED;
            if (status == "FAILED") return CompressionStatus::FAILED;
            return CompressionStatus::PENDING;
        }
    };
}
