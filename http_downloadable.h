#pragma once
#include <curl/curl.h>
#include <fstream>

#include "base_downloadable.h"

namespace DownLoad {

class HttpDownloadable : public Downloadable {
 public:
  HttpDownloadable() {}
  HttpDownloadable(int thread_nums) : Downloadable(thread_nums) {}
  // ret 0： 全部成功； >1 失败的分片数
  virtual int Download(const std::string& url, const std::string& local_path, const std::string& local_name) override;

  virtual ~HttpDownloadable() {}

 private:
  // 获取文件大小
  int GetFileSize(const std::string& url, curl_off_t& file_size);
  // 利用http分块下载
  int DownloadPart(const std::string& url, const std::string& local_path_file, size_t total_size, int total_part,
                   int part);
};
}  // namespace DownLoad
