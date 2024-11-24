#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <filesystem>
#include <thread>

namespace utils {
    struct CacheEntry {
        std::string path;
        std::chrono::system_clock::time_point lastAccess;
        size_t size;
    };

    class CacheManager {
    public:
        static CacheManager& getInstance() {
            static CacheManager instance;
            return instance;
        }

        std::string getCachedFile(const std::string& fileId) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(fileId);
            if (it != cache_.end()) {
                it->second.lastAccess = std::chrono::system_clock::now();
                return it->second.path;
            }
            return "";
        }

        void addToCache(const std::string& fileId, const std::string& path, size_t size) {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_[fileId] = {path, std::chrono::system_clock::now(), size};
            currentSize_ += size;
            cleanupIfNeeded();
        }

        void removeFromCache(const std::string& fileId) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(fileId);
            if (it != cache_.end()) {
                std::filesystem::remove(it->second.path);
                currentSize_ -= it->second.size;
                cache_.erase(it);
            }
        }

    private:
        CacheManager() : currentSize_(0), maxSize_(1024 * 1024 * 1024) { // 1GB cache
            cleanupThread_ = std::thread([this]() {
                while (true) {
                    std::this_thread::sleep_for(std::chrono::minutes(5));
                    cleanup();
                }
            });
            cleanupThread_.detach();
        }

        void cleanupIfNeeded() {
            if (currentSize_ > maxSize_) {
                cleanup();
            }
        }

        void cleanup() {
            std::lock_guard<std::mutex> lock(mutex_);
            auto now = std::chrono::system_clock::now();
            
            std::vector<std::pair<std::string, CacheEntry>> entries(cache_.begin(), cache_.end());
            std::sort(entries.begin(), entries.end(),
                [](const auto& a, const auto& b) {
                    return a.second.lastAccess < b.second.lastAccess;
                });

            for (const auto& entry : entries) {
                auto age = std::chrono::duration_cast<std::chrono::hours>(
                    now - entry.second.lastAccess).count();
                
                if (age > 24 || currentSize_ > maxSize_) { // Remove files older than 24h or if cache is full
                    std::filesystem::remove(entry.second.path);
                    currentSize_ -= entry.second.size;
                    cache_.erase(entry.first);
                }
                
                if (currentSize_ <= maxSize_ * 0.8) { // Stop when we're below 80% capacity
                    break;
                }
            }
        }

        std::unordered_map<std::string, CacheEntry> cache_;
        std::mutex mutex_;
        size_t currentSize_;
        const size_t maxSize_;
        std::thread cleanupThread_;
    };
}
