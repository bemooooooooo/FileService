#include <gtest/gtest.h>
#include "../utils/CacheManager.hpp"
#include <thread>
#include <chrono>

class CacheManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache_manager = std::make_unique<utils::CacheManager>(5); // Кэш на 5 элементов
    }

    std::unique_ptr<utils::CacheManager> cache_manager;
};

TEST_F(CacheManagerTest, BasicOperations) {
    // Проверяем добавление и получение элемента
    cache_manager->put("key1", "value1");
    EXPECT_EQ(cache_manager->get("key1"), "value1");
    
    // Проверяем отсутствующий ключ
    EXPECT_EQ(cache_manager->get("nonexistent"), "");
    
    // Проверяем перезапись значения
    cache_manager->put("key1", "new_value1");
    EXPECT_EQ(cache_manager->get("key1"), "new_value1");
}

TEST_F(CacheManagerTest, LRUEviction) {
    // Заполняем кэш
    for (int i = 1; i <= 5; ++i) {
        cache_manager->put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    // Проверяем, что все элементы на месте
    for (int i = 1; i <= 5; ++i) {
        EXPECT_EQ(cache_manager->get("key" + std::to_string(i)), "value" + std::to_string(i));
    }
    
    // Добавляем новый элемент, который должен вытеснить самый старый
    cache_manager->put("key6", "value6");
    
    // Проверяем, что первый элемент удален
    EXPECT_EQ(cache_manager->get("key1"), "");
    
    // Проверяем, что новый элемент добавлен
    EXPECT_EQ(cache_manager->get("key6"), "value6");
}

TEST_F(CacheManagerTest, ThreadSafety) {
    const int num_threads = 4;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;
    
    // Создаем несколько потоков, которые одновременно работают с кэшем
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                std::string key = "key" + std::to_string(t) + "_" + std::to_string(i);
                std::string value = "value" + std::to_string(t) + "_" + std::to_string(i);
                
                cache_manager->put(key, value);
                std::string retrieved = cache_manager->get(key);
                
                // Проверяем, что значение либо наше, либо уже вытеснено
                EXPECT_TRUE(retrieved == value || retrieved == "");
            }
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST_F(CacheManagerTest, ExpirationTime) {
    // Устанавливаем элемент с временем жизни 1 секунда
    cache_manager->put("expiring_key", "expiring_value", std::chrono::seconds(1));
    
    // Проверяем, что элемент доступен сразу после добавления
    EXPECT_EQ(cache_manager->get("expiring_key"), "expiring_value");
    
    // Ждем 2 секунды
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Проверяем, что элемент удален
    EXPECT_EQ(cache_manager->get("expiring_key"), "");
}

TEST_F(CacheManagerTest, Clear) {
    // Добавляем несколько элементов
    for (int i = 1; i <= 3; ++i) {
        cache_manager->put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    // Очищаем кэш
    cache_manager->clear();
    
    // Проверяем, что все элементы удалены
    for (int i = 1; i <= 3; ++i) {
        EXPECT_EQ(cache_manager->get("key" + std::to_string(i)), "");
    }
}

TEST_F(CacheManagerTest, Size) {
    EXPECT_EQ(cache_manager->size(), 0);
    
    // Добавляем элементы
    cache_manager->put("key1", "value1");
    EXPECT_EQ(cache_manager->size(), 1);
    
    cache_manager->put("key2", "value2");
    EXPECT_EQ(cache_manager->size(), 2);
    
    // Перезаписываем существующий ключ
    cache_manager->put("key1", "new_value1");
    EXPECT_EQ(cache_manager->size(), 2);
    
    // Добавляем элементы до превышения размера кэша
    for (int i = 3; i <= 6; ++i) {
        cache_manager->put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    // Проверяем, что размер не превышает максимальный
    EXPECT_EQ(cache_manager->size(), 5);
}

TEST_F(CacheManagerTest, Contains) {
    cache_manager->put("key1", "value1");
    
    EXPECT_TRUE(cache_manager->contains("key1"));
    EXPECT_FALSE(cache_manager->contains("nonexistent"));
    
    // Проверяем после истечения срока действия
    cache_manager->put("expiring_key", "value", std::chrono::milliseconds(100));
    EXPECT_TRUE(cache_manager->contains("expiring_key"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(cache_manager->contains("expiring_key"));
}
