#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>
#include <memory>
#include "../models/CompressionTask.h"

namespace utils {
    class AsyncTaskManager {
    public:
        static AsyncTaskManager& getInstance() {
            static AsyncTaskManager instance;
            return instance;
        }

        template<typename F>
        auto enqueueTask(F&& f) -> std::future<typename std::result_of<F()>::type> {
            using return_type = typename std::result_of<F()>::type;
            
            auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
            std::future<return_type> result = task->get_future();
            
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                tasks_.emplace([task]() { (*task)(); });
            }
            
            condition_.notify_one();
            return result;
        }

        void addCompressionTask(const models::CompressionTask& task) {
            std::unique_lock<std::mutex> lock(compressionQueueMutex_);
            compressionTasks_.push(task);
            condition_.notify_one();
        }

        models::CompressionTask getNextCompressionTask() {
            std::unique_lock<std::mutex> lock(compressionQueueMutex_);
            if (compressionTasks_.empty()) {
                return models::CompressionTask();
            }
            auto task = compressionTasks_.front();
            compressionTasks_.pop();
            return task;
        }

    private:
        AsyncTaskManager() {
            const size_t numThreads = std::thread::hardware_concurrency();
            workers_.reserve(numThreads);
            
            for (size_t i = 0; i < numThreads; ++i) {
                workers_.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(queueMutex_);
                            condition_.wait(lock, [this] {
                                return !tasks_.empty() || !compressionTasks_.empty() || stopping_;
                            });
                            
                            if (stopping_ && tasks_.empty() && compressionTasks_.empty()) {
                                return;
                            }
                            
                            if (!tasks_.empty()) {
                                task = std::move(tasks_.front());
                                tasks_.pop();
                            } else if (!compressionTasks_.empty()) {
                                auto compressionTask = compressionTasks_.front();
                                compressionTasks_.pop();
                                task = [this, compressionTask]() {
                                    processCompressionTask(compressionTask);
                                };
                            }
                        }
                        if (task) {
                            task();
                        }
                    }
                });
            }
        }

        ~AsyncTaskManager() {
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                stopping_ = true;
            }
            condition_.notify_all();
            for (auto& worker : workers_) {
                worker.join();
            }
        }

        void processCompressionTask(const models::CompressionTask& task) {
            // Implement compression task processing
            // This will be implemented in the CompressionService
        }

        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::queue<models::CompressionTask> compressionTasks_;
        std::mutex queueMutex_;
        std::mutex compressionQueueMutex_;
        std::condition_variable condition_;
        bool stopping_ = false;
    };
}
