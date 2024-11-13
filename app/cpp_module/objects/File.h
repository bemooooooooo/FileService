#pragma once
#include <drogon/MultiPart.h>

#include <chrono>
#include <iostream>

class File {
 private:
  std::string filename;
  uint32_t size;
  std::string uuid;
	std::string ext;
  std::chrono::microseconds create_date;

 public:
  File(const drogon::MultiPartParser& parser);
  ~File() = default;
	File(const File& other) = default;

};  // Class File