#include "File.h"

File::File(const drogon::MultiPartParser& parser) {
  if (parser.getFiles().empty()) {
    throw std::runtime_error("TEST");
  }
  filename = parser.getFiles()[0].getFileName();
  size = parser.getFiles()[0].fileLength();
  ext = parser.getFiles()[0].getFileExtension();
  uuid = std::hash<std::string>{}(filename);
  create_date = std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::system_clock::now().time_since_epoch());
}
