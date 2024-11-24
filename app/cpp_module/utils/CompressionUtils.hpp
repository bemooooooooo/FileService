#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <zlib.h>
#include <stdexcept>

namespace utils {
    class CompressionUtils {
    public:
        static bool compressFile(const std::string& inputPath, const std::string& outputPath) {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open()) {
                return false;
            }

            std::vector<char> buffer(1024 * 1024); // 1MB buffer
            input.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = input.gcount();

            z_stream zs;
            zs.zalloc = Z_NULL;
            zs.zfree = Z_NULL;
            zs.opaque = Z_NULL;
            
            if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
                input.close();
                return false;
            }

            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open()) {
                deflateEnd(&zs);
                input.close();
                return false;
            }

            std::vector<char> outBuffer(1024 * 1024);
            int ret;
            
            do {
                if (!input.eof()) {
                    input.read(buffer.data(), buffer.size());
                    bytesRead = input.gcount();
                }

                zs.avail_in = static_cast<uInt>(bytesRead);
                zs.next_in = reinterpret_cast<Bytef*>(buffer.data());

                do {
                    zs.avail_out = static_cast<uInt>(outBuffer.size());
                    zs.next_out = reinterpret_cast<Bytef*>(outBuffer.data());

                    ret = deflate(&zs, input.eof() ? Z_FINISH : Z_NO_FLUSH);
                    if (ret == Z_STREAM_ERROR) {
                        deflateEnd(&zs);
                        input.close();
                        output.close();
                        return false;
                    }

                    output.write(outBuffer.data(), outBuffer.size() - zs.avail_out);
                } while (zs.avail_out == 0);

            } while (input.good() && ret != Z_STREAM_END);

            deflateEnd(&zs);
            input.close();
            output.close();

            return ret == Z_STREAM_END;
        }

        static bool decompressFile(const std::string& inputPath, const std::string& outputPath) {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open()) {
                return false;
            }

            std::vector<char> buffer(1024 * 1024);
            input.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = input.gcount();

            z_stream zs;
            zs.zalloc = Z_NULL;
            zs.zfree = Z_NULL;
            zs.opaque = Z_NULL;
            zs.avail_in = 0;
            zs.next_in = Z_NULL;

            if (inflateInit(&zs) != Z_OK) {
                input.close();
                return false;
            }

            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open()) {
                inflateEnd(&zs);
                input.close();
                return false;
            }

            std::vector<char> outBuffer(1024 * 1024);
            int ret;

            do {
                if (!input.eof()) {
                    input.read(buffer.data(), buffer.size());
                    bytesRead = input.gcount();
                }

                zs.avail_in = static_cast<uInt>(bytesRead);
                zs.next_in = reinterpret_cast<Bytef*>(buffer.data());

                do {
                    zs.avail_out = static_cast<uInt>(outBuffer.size());
                    zs.next_out = reinterpret_cast<Bytef*>(outBuffer.data());

                    ret = inflate(&zs, Z_NO_FLUSH);
                    if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                        inflateEnd(&zs);
                        input.close();
                        output.close();
                        return false;
                    }

                    output.write(outBuffer.data(), outBuffer.size() - zs.avail_out);
                } while (zs.avail_out == 0);

            } while (input.good() && ret != Z_STREAM_END);

            inflateEnd(&zs);
            input.close();
            output.close();

            return ret == Z_STREAM_END;
        }

        static size_t getCompressedFileSize(const std::string& filePath) {
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            return file.tellg();
        }
    };
}
