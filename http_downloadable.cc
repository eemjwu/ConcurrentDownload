#include "http_downloadable.h"
#include <iostream>

namespace DownLoad {
int HttpDownloadable::Download(const std::string& url, const std::string& local_path, const std::string& local_name) {
  // 首先获取文件大小
  curl_off_t file_size;
  int ret = GetFileSize(url, file_size);
  if (ret != 0) {
    return ret;
  }

  // std::cout << "size: " << file_size << std::endl;
  // 多线程下载文件
  ThreadPool thread_pool;
  thread_pool.Init(thread_nums_);
  thread_pool.Start();

  SyncHelpPtr sync_help = std::make_shared<SyncHelp>(thread_nums_);

  std::string local_path_file = local_path + "/" + local_name;
  for (int i = 0; i < thread_nums_; ++i) {
    thread_pool.enqueue([this, url, local_path_file, i, file_size, sync_help]() {
      int ret = DownloadPart(url, local_path_file, i, file_size);
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
      sync_help->cv_.wait(lock, [sync_help]() { return sync_help->has_download_size_ == sync_help->total_part_size_; });
    }
  }
  if (sync_help->task_status_ != 0) return sync_help->task_status_;

  // 将分片文件合并
  return MergeFile(local_path_file);
}

int HttpDownloadable::MergeFile(const std::string& local_path_file) {
  // 将分片文件合并
  std::ofstream tooutput_file(local_path_file, std::ios::binary);
  if (!tooutput_file.is_open()) {
    std::cerr << "Failed to open output file: " << local_path_file << std::endl;
    return -1;
  }

  for (int i = 0; i < thread_nums_; i++) {
    std::ifstream inputFile(local_path_file + std::to_string(i), std::ios::binary);
    if (!inputFile.is_open()) {
      std::cerr << "Failed to open input file: " << local_path_file + std::to_string(i) << std::endl;
      return -1;
    }
    tooutput_file << inputFile.rdbuf();
    if (tooutput_file.fail()) {
      std::cerr << "Failed to write to output file: " << local_path_file << std::endl;
      return -1;
    }
     // 删除碎片文件
    remove((local_path_file + std::to_string(i)).c_str());
  }

  return 0;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* tooutput_file) {
  size_t total_size = size * nmemb;
  tooutput_file->write(static_cast<char*>(contents), total_size);

  return total_size;
}

int HttpDownloadable::DownloadPart(const std::string& url, const std::string& local_path_file, int part,
                                   size_t total_size) {
  int total_parts = thread_nums_;
  CURL* curl = curl_easy_init();
  if (!curl) {
    return -1;
  }

  // 计算每个分片的范围
  curl_off_t start = part * (total_size / total_parts);
  curl_off_t end = (part == total_parts - 1) ? total_size : (part + 1) * (total_size / total_parts) - 1;

  // 设置参数
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_RANGE, (std::to_string(start) + "-" + std::to_string(end)).c_str());
  // std::cout << (std::to_string(start) + "-" + std::to_string(end)).c_str() << std::endl;

  // 将下载的文件写入本地
  std::ofstream tooutput_file(local_path_file + std::to_string(part), std::ios::binary);
  if (!tooutput_file.is_open()) {
    std::cerr << "Failed to open output file: " << local_path_file + std::to_string(part) << std::endl;
    curl_easy_cleanup(curl);
    return -1;
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tooutput_file);

  // 执行下载
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    curl_easy_cleanup(curl);
    return -1;
  }

  // Clean up
  curl_easy_cleanup(curl);
  return 0;
}

int HttpDownloadable::GetFileSize(const std::string& url, curl_off_t& file_size) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  
  curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

  if (curl_easy_perform(curl) != CURLE_OK) {
    curl_easy_cleanup(curl);
    return -1;
  }

  if (curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &file_size) != CURLE_OK) {
    curl_easy_cleanup(curl);
    return -1;
  }

  curl_easy_cleanup(curl);
  return 0;
}
}  // namespace DownLoad