#pragma once
#include <iostream>
#include <string>
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

class Downloadable {
 public:
  using RealDownloadPartOP = std::function<int(const std::string& url, const std::string& local_path_file,
                                               size_t total_size, int total_part, int part)>;

 public:
  Downloadable() {}
  Downloadable(int thread_nums) {
    if (thread_nums > 0) thread_nums_ = thread_nums;
  }
  // ret 0： 全部成功； >1 失败的分片数
  virtual int Download(const std::string& url, const std::string& local_path, const std::string& local_name) = 0;
  virtual ~Downloadable() {}

 protected:
  int MergeFile(const std::string& local_path_file);
  // download_part_op 不同协议实际的下载方法
  int ParallelDownload(const std::string& url, const std::string& local_path_file, size_t file_size,
                       RealDownloadPartOP download_part_op) {
    // std::cout << "size: " << file_size << std::endl;
    // 多线程下载文件
    ThreadPool thread_pool;
    thread_pool.Init(thread_nums_);
    thread_pool.Start();

    SyncHelpPtr sync_help = std::make_shared<SyncHelp>(thread_nums_);
    for (int i = 0; i < thread_nums_; ++i) {
      thread_pool.enqueue([this, url, local_path_file, i, file_size, sync_help, download_part_op]() {
        int ret = download_part_op(url, local_path_file, file_size, thread_nums_, i);
        {
          std::unique_lock<std::mutex> lock(sync_help->m_);
          if (ret != 0) sync_help->task_status_++;
          if (++sync_help->has_download_size_ == sync_help->total_part_size_) {
            sync_help->cv_.notify_one();
          }
        }
      });
    }
    {
      std::unique_lock<std::mutex> lock(sync_help->m_);
      // 等待条件前检测是否已经满足条件
      if (sync_help->has_download_size_ != sync_help->total_part_size_) {
        sync_help->cv_.wait(lock,
                            [sync_help]() { return sync_help->has_download_size_ == sync_help->total_part_size_; });
      }
    }
    if (sync_help->task_status_ != 0) return sync_help->task_status_;

    // 将分片文件合并
    return MergeFile(local_path_file);
  }

 private:
  // 默认10线程
  int thread_nums_ = 10;
};
}  // namespace DownLoad
