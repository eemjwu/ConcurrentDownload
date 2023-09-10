#include "base_downloadable.h"
#include <iostream>
#include <fstream>

namespace DownLoad {
int Downloadable::MergeFile(const std::string& local_path_file) {
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

}  // namespace DownLoad
