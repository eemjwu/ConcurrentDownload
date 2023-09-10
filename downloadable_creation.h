#pragma once
#include <memory>
#include "base_downloadable.h"
#include "http_downloadable.h"
namespace DownLoad {
class DownloadableCreation {
 public:
  DownloadableCreation() {}
  ~DownloadableCreation() {}
  std::shared_ptr<Downloadable> CreateDownloadable(const std::string &protocol, int thread_num);
};

using DownloadablePtr = std::shared_ptr<Downloadable>;
}  // namespace DownLoad
