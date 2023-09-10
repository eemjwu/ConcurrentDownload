#pragma once
#include <curl/curl.h>
#include <fstream>

#include "base_downloadable.h"
#include "thread_pool.h"

namespace DownLoad {

struct SyncHelp {
  SyncHelp(int total_part_size) : total_part_size_(total_part_size), task_status_(0), has_download_size_(0) {}
  // 同步任务执行状态
  std::condition_variable cv_;
  std::mutex m_;
  // 0： 全部成功； >1 失败的分片数
  int task_status_;
  // 已经执行完成的分片数量
  int has_download_size_;
  // 总的分片数量
  int total_part_size_;
};
using SyncHelpPtr = std::shared_ptr<SyncHelp>;

class HttpDownloadable : public Downloadable {
 public:
  HttpDownloadable() {}
  HttpDownloadable(int thread_nums) {
    if (thread_nums > 0) thread_nums_ = thread_nums;
  }
  // ret 0： 全部成功； >1 失败的分片数
  virtual int Download(const std::string& url, const std::string& local_path, const std::string& local_name) override;

  virtual ~HttpDownloadable() {}

 private:
  // 获取文件大小
  int GetFileSize(const std::string& url, curl_off_t& file_size);
  // 分块下载
  int DownloadPart(const std::string& url, const std::string& local_path_file, int part, size_t total_size);
  // 合并分块
  int MergeFile(const std::string& local_path_file);

 private:
  // 默认10线程
  int thread_nums_ = 10;
};
}  // namespace DownLoad
