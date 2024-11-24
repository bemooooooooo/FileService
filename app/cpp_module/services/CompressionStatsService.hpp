#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include "../models/CompressionTask.h"

namespace services {
    struct CompressionStats {
        size_t totalFiles = 0;
        size_t totalBytesOriginal = 0;
        size_t totalBytesCompressed = 0;
        double averageCompressionRatio = 0.0;
        std::chrono::milliseconds averageCompressionTime{0};
        std::unordered_map<models::CompressionLevel, size_t> levelUsage;
        std::unordered_map<std::string, double> fileTypeStats; // extension -> avg compression ratio
    };

    class CompressionStatsService {
    public:
        static CompressionStatsService& getInstance() {
            static CompressionStatsService instance;
            return instance;
        }

        void recordCompression(
            const std::string& fileType,
            size_t originalSize,
            size_t compressedSize,
            models::CompressionLevel level,
            std::chrono::milliseconds compressionTime
        ) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            stats_.totalFiles++;
            stats_.totalBytesOriginal += originalSize;
            stats_.totalBytesCompressed += compressedSize;
            
            // Update compression ratio
            double ratio = static_cast<double>(compressedSize) / originalSize;
            stats_.averageCompressionRatio = (
                (stats_.averageCompressionRatio * (stats_.totalFiles - 1) + ratio)
                / stats_.totalFiles
            );
            
            // Update average compression time
            stats_.averageCompressionTime = std::chrono::milliseconds(
                (stats_.averageCompressionTime.count() * (stats_.totalFiles - 1) 
                 + compressionTime.count()) / stats_.totalFiles
            );
            
            // Update level usage
            stats_.levelUsage[level]++;
            
            // Update file type stats
            auto& typeStats = stats_.fileTypeStats[fileType];
            size_t typeCount = typeStats > 0 ? stats_.totalFiles : 1;
            typeStats = (typeStats * (typeCount - 1) + ratio) / typeCount;
        }

        CompressionStats getStats() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return stats_;
        }

        double getEfficiencyForFileType(const std::string& fileType) const {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = stats_.fileTypeStats.find(fileType);
            return it != stats_.fileTypeStats.end() ? it->second : 0.0;
        }

        models::CompressionLevel suggestCompressionLevel(
            const std::string& fileType,
            size_t fileSize
        ) const {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // If we have stats for this file type
            auto it = stats_.fileTypeStats.find(fileType);
            if (it != stats_.fileTypeStats.end()) {
                double efficiency = it->second;
                
                // If compression is not effective for this type
                if (efficiency > 0.9) {
                    return models::CompressionLevel::NONE;
                }
                
                // For very compressible files, use max compression
                if (efficiency < 0.5) {
                    return models::CompressionLevel::MAX;
                }
            }
            
            // Base decision on file size
            if (fileSize < 1024 * 1024) { // < 1MB
                return models::CompressionLevel::FAST;
            } else if (fileSize < 10 * 1024 * 1024) { // < 10MB
                return models::CompressionLevel::BALANCED;
            } else {
                return models::CompressionLevel::MAX;
            }
        }

        Json::Value getStatsJson() const {
            std::lock_guard<std::mutex> lock(mutex_);
            Json::Value json;
            
            json["total_files"] = static_cast<Json::UInt64>(stats_.totalFiles);
            json["total_bytes_original"] = static_cast<Json::UInt64>(stats_.totalBytesOriginal);
            json["total_bytes_compressed"] = static_cast<Json::UInt64>(stats_.totalBytesCompressed);
            json["average_compression_ratio"] = stats_.averageCompressionRatio;
            json["average_compression_time_ms"] = static_cast<Json::UInt64>(
                stats_.averageCompressionTime.count()
            );
            
            Json::Value levelUsage;
            for (const auto& [level, count] : stats_.levelUsage) {
                levelUsage[std::to_string(static_cast<int>(level))] = 
                    static_cast<Json::UInt64>(count);
            }
            json["level_usage"] = levelUsage;
            
            Json::Value fileTypeStats;
            for (const auto& [type, ratio] : stats_.fileTypeStats) {
                fileTypeStats[type] = ratio;
            }
            json["file_type_stats"] = fileTypeStats;
            
            return json;
        }

    private:
        CompressionStatsService() = default;
        mutable std::mutex mutex_;
        CompressionStats stats_;
    };
}
