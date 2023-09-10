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

  // std::cout << file_size << std::endl;

  std::string local_path_file = local_path + "/" + local_name;
  return ParallelDownload(
      url, local_path_file, file_size,
      [this](const std::string& url, const std::string& local_path_file, size_t total_size, int total_part,
             int part) -> int { this->DownloadPart(url, local_path_file, total_size, total_part, part); });
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* tooutput_file) {
  size_t total_size = size * nmemb;
  tooutput_file->write(static_cast<char*>(contents), total_size);

  return total_size;
}

int HttpDownloadable::DownloadPart(const std::string& url, const std::string& local_path_file, size_t total_size,
                                   int total_parts, int part) {
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