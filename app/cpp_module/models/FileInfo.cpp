#include "FileInfo.h"
#include <drogon/orm/Exception.h>

namespace models {

bool FileInfo::createTable(drogon::orm::DbClientPtr& client) {
    try {
        client->execSqlSync(
            "CREATE TABLE IF NOT EXISTS file_info ("
            "uuid VARCHAR(36) PRIMARY KEY,"
            "original_name VARCHAR(255) NOT NULL,"
            "user_id VARCHAR(36) NOT NULL,"
            "size BIGINT NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "CONSTRAINT fk_user FOREIGN KEY (user_id) REFERENCES users(id)"
            ");"
        );
        return true;
    } catch (const drogon::orm::DrogonDbException& e) {
        // Log error
        return false;
    }
}

FileInfo FileInfo::fromRow(const drogon::orm::Row& row) {
    FileInfo info;
    info.uuid = row["uuid"].as<std::string>();
    info.original_name = row["original_name"].as<std::string>();
    info.user_id = row["user_id"].as<std::string>();
    info.size = row["size"].as<size_t>();
    info.created_at = row["created_at"].as<std::string>();
    return info;
}

Json::Value FileInfo::toJson() const {
    Json::Value json;
    json["uuid"] = uuid;
    json["original_name"] = original_name;
    json["size"] = static_cast<Json::UInt64>(size);
    json["created_at"] = created_at;
    return json;
}

}
