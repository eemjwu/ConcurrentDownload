#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  // 构造函数
  ThreadPool() : stop_(false) {}

  int Init(int thread_num) {
    if (thread_num <= 0) {
      return -1;
    }
    thread_nums_ = thread_num;
    return 0;
  }

  void Start() {
    for (size_t i = 0; i < thread_nums_; ++i) {
      workers_.emplace_back([this]() {
        while (true) {
          std::function<void()> task;
          {
            // 等待任务
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->condition_.wait(lock, [this] {
              // 直到满足以下条件
              return this->stop_ || !this->tasks_.empty();
            });
            // 如果线程池结束了，退出线程
            if (this->stop_ && this->tasks_.empty()) return;
            // 获取第一个任务
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }

          // 执行任务
          task();
        }
      });
    }
  }

  // 析构函数
  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }
    // 记得唤醒条件变量
    condition_.notify_all();
    // join 每个线程
    for (std::thread &worker : workers_) worker.join();
  }

  // enqueue 任务函数,
  template <typename F>
  void enqueue(F &&f) {
    // push进队列
    {
      // 先判断是否结束
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
      // 完美转发
      tasks_.emplace(std::forward<F>(f));
    }
    // 唤醒条件变量
    condition_.notify_one();
  }

 private:
  // 禁止拷贝 移动构造函数
  ThreadPool(const ThreadPool &another) = delete;
  ThreadPool(ThreadPool &&another) = delete;

 private:
  // 工作线程
  std::vector<std::thread> workers_;
  // 任务队列
  std::queue<std::function<void()>> tasks_;
  // 队列锁
  std::mutex queue_mutex_;
  // 条件变量，用于等待队列不为空
  std::condition_variable condition_;
  // 线程池是否停止
  bool stop_;
  // 线程个数
  int thread_nums_;
};