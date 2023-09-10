#include <iostream>
#include "downloadable_creation.h"

using namespace DownLoad;

int main(int argc, char* argv[])  {
  int thread_num = 10;
  DownloadableCreation download_creation;
  DownloadablePtr http_downlaodable = download_creation.CreateDownloadable("http", thread_num);
  std::string file_url = "http://a11.gdl.netease.com/MuMuInstaller_1.4.2.1_nochannel_zh-Hans_1653021714.exe";
  std::string local_file_path = "./test";
  std::string local_file_path_name = "test.exe";
  int ret = http_downlaodable->Download(file_url, local_file_path, local_file_path_name);
  if (ret != 0) {
    std::cout << "download: " << file_url << " fail!" << std::endl;
    return -1;
  }
  std::cout << "download: " << file_url << " success!" << std::endl;
}  // namespace DownLoad