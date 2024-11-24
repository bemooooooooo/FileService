#include <gtest/gtest.h>
#include "../utils/CompressionUtils.hpp"
#include <fstream>
#include <filesystem>

class CompressionUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовый файл
        std::ofstream test_file("test.txt");
        test_file << "This is a test file content for compression testing.\n"
                  << "It contains multiple lines of text to ensure proper compression.\n"
                  << "The content should be large enough to test compression effectively.";
        test_file.close();
    }

    void TearDown() override {
        // Удаляем тестовые файлы
        std::filesystem::remove("test.txt");
        std::filesystem::remove("test.txt.gz");
    }
};

TEST_F(CompressionUtilsTest, CompressFile) {
    // Проверяем сжатие файла
    bool result = utils::CompressionUtils::compressFile("test.txt", "test.txt.gz");
    EXPECT_TRUE(result);
    
    // Проверяем, что сжатый файл существует
    EXPECT_TRUE(std::filesystem::exists("test.txt.gz"));
    
    // Проверяем, что сжатый файл меньше оригинального
    auto original_size = std::filesystem::file_size("test.txt");
    auto compressed_size = std::filesystem::file_size("test.txt.gz");
    EXPECT_LT(compressed_size, original_size);
}

TEST_F(CompressionUtilsTest, DecompressFile) {
    // Сначала сжимаем файл
    utils::CompressionUtils::compressFile("test.txt", "test.txt.gz");
    
    // Затем распаковываем его
    bool result = utils::CompressionUtils::decompressFile("test.txt.gz", "test_decompressed.txt");
    EXPECT_TRUE(result);
    
    // Проверяем, что распакованный файл существует
    EXPECT_TRUE(std::filesystem::exists("test_decompressed.txt"));
    
    // Проверяем, что содержимое совпадает с оригиналом
    std::ifstream original("test.txt");
    std::ifstream decompressed("test_decompressed.txt");
    std::string original_content((std::istreambuf_iterator<char>(original)),
                               std::istreambuf_iterator<char>());
    std::string decompressed_content((std::istreambuf_iterator<char>(decompressed)),
                                   std::istreambuf_iterator<char>());
    EXPECT_EQ(original_content, decompressed_content);
    
    // Очищаем
    std::filesystem::remove("test_decompressed.txt");
}

TEST_F(CompressionUtilsTest, CompressWithDifferentLevels) {
    std::vector<std::pair<int, size_t>> compression_results;
    
    // Тестируем разные уровни сжатия
    for (int level = 0; level <= 9; ++level) {
        std::string compressed_file = "test_level_" + std::to_string(level) + ".gz";
        utils::CompressionUtils::compressFile("test.txt", compressed_file, level);
        
        size_t size = std::filesystem::file_size(compressed_file);
        compression_results.push_back({level, size});
        
        std::filesystem::remove(compressed_file);
    }
    
    // Проверяем, что более высокий уровень сжатия дает меньший размер файла
    for (size_t i = 1; i < compression_results.size(); ++i) {
        EXPECT_LE(compression_results[i].second, compression_results[i-1].second)
            << "Compression level " << compression_results[i].first
            << " should give better or equal compression than level "
            << compression_results[i-1].first;
    }
}

TEST_F(CompressionUtilsTest, CompressNonExistentFile) {
    bool result = utils::CompressionUtils::compressFile(
        "nonexistent.txt",
        "nonexistent.txt.gz"
    );
    EXPECT_FALSE(result);
}

TEST_F(CompressionUtilsTest, DecompressNonExistentFile) {
    bool result = utils::CompressionUtils::decompressFile(
        "nonexistent.txt.gz",
        "nonexistent.txt"
    );
    EXPECT_FALSE(result);
}

TEST_F(CompressionUtilsTest, CompressEmptyFile) {
    // Создаем пустой файл
    std::ofstream empty_file("empty.txt");
    empty_file.close();
    
    bool result = utils::CompressionUtils::compressFile("empty.txt", "empty.txt.gz");
    EXPECT_TRUE(result);
    
    // Проверяем размер сжатого файла
    EXPECT_GT(std::filesystem::file_size("empty.txt.gz"), 0);
    
    // Очищаем
    std::filesystem::remove("empty.txt");
    std::filesystem::remove("empty.txt.gz");
}

TEST_F(CompressionUtilsTest, CompressLargeFile) {
    // Создаем большой файл (10MB)
    std::ofstream large_file("large.txt");
    for (int i = 0; i < 1024 * 1024; ++i) {
        large_file << "This is a test line that will be repeated many times.\n";
    }
    large_file.close();
    
    bool result = utils::CompressionUtils::compressFile("large.txt", "large.txt.gz");
    EXPECT_TRUE(result);
    
    // Проверяем степень сжатия
    auto original_size = std::filesystem::file_size("large.txt");
    auto compressed_size = std::filesystem::file_size("large.txt.gz");
    EXPECT_LT(compressed_size, original_size / 2); // Ожидаем сжатие минимум в 2 раза
    
    // Очищаем
    std::filesystem::remove("large.txt");
    std::filesystem::remove("large.txt.gz");
}
