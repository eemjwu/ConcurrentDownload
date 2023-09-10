#pragma once
#include <string>

namespace DownLoad {

class Downloadable {
 public:
  virtual int Download(const std::string& url, const std::string& local_path ,const std::string& local_name) = 0;
  virtual ~Downloadable() {}
};
}  // namespace DownLoad
