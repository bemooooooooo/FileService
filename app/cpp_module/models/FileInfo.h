#pragma once
#include <string>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <json/json.h>

namespace models {
    class FileInfo {
    public:
        std::string uuid;
        std::string original_name;
        std::string user_id;
        size_t size;
        size_t compressed_size;
        bool is_compressed;
        std::string created_at;

        static bool createTable(drogon::orm::DbClientPtr& client) {
            try {
                client->execSqlSync(
                    "CREATE TABLE IF NOT EXISTS file_info ("
                    "uuid VARCHAR(36) PRIMARY KEY,"
                    "original_name VARCHAR(255) NOT NULL,"
                    "user_id VARCHAR(36) NOT NULL,"
                    "size BIGINT NOT NULL,"
                    "compressed_size BIGINT,"
                    "is_compressed BOOLEAN DEFAULT FALSE,"
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                    "CONSTRAINT fk_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
                    ")"
                );
                return true;
            } catch (...) {
                return false;
            }
        }
        
        static FileInfo fromRow(const drogon::orm::Row& row) {
            FileInfo info;
            info.uuid = row["uuid"].as<std::string>();
            info.original_name = row["original_name"].as<std::string>();
            info.user_id = row["user_id"].as<std::string>();
            info.size = row["size"].as<size_t>();
            info.compressed_size = row["compressed_size"].isNull() ? 0 : row["compressed_size"].as<size_t>();
            info.is_compressed = row["is_compressed"].as<bool>();
            info.created_at = row["created_at"].as<std::string>();
            return info;
        }

        Json::Value toJson() const {
            Json::Value json;
            json["uuid"] = uuid;
            json["original_name"] = original_name;
            json["size"] = static_cast<Json::UInt64>(size);
            if (is_compressed) {
                json["compressed_size"] = static_cast<Json::UInt64>(compressed_size);
                json["compression_ratio"] = static_cast<double>(size) / compressed_size;
            }
            json["is_compressed"] = is_compressed;
            json["created_at"] = created_at;
            return json;
        }
    };
}
