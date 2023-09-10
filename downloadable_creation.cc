
#include "downloadable_creation.h"

namespace DownLoad {
std::shared_ptr<Downloadable> DownloadableCreation::CreateDownloadable(const std::string &protocol, int thread_num) {
  if (protocol == "http") {
    return std::make_shared<HttpDownloadable>(thread_num);
  }
  // 其他下载协议
  else {
    return nullptr;
  }
}
}  // namespace DownLoad
