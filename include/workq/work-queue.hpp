#pragma once
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <workq/task.hpp>

namespace workq {

struct Stats {
  size_t queue;
  size_t running;
};

using HookCallback =
    std::function<void(TaskState state, GenericTask *task, const Stats &stats)>;

namespace internal {
class WorkQueuePrivate;
}

class WorkQueue {

public:
  WorkQueue(const std::string &name,
            size_t capacity = std::thread::hardware_concurrency(),
            bool paused = false);
  WorkQueue(size_t numb = std::thread::hardware_concurrency(),
            bool paused = false);
  ~WorkQueue();

  WorkQueue(const WorkQueue &) = delete;
  WorkQueue &operator=(const WorkQueue &) = delete;

  WorkQueue(WorkQueue &&);
  WorkQueue &operator=(WorkQueue &&);

  void set_hook(HookCallback callback);

  template <typename T> void dispatch(T fn) {
    GenericTask *task = new Task<T>(std::move(fn));
    dispatch(task);
  }

  template <class T, typename... Args> void dispatch(Args... args) {
    GenericTask *task = new T(std::forward<Args>(args)...);
    dispatch(task);
  }

  WorkQueue &dispatch(GenericTask *task);

  WorkQueue &pause();
  WorkQueue &resume();
  void wait() const;

  size_t size() const;
  size_t capacity() const;
  size_t running() const;

private:
  std::unique_ptr<internal::WorkQueuePrivate> d;
};

template <> class Task<WorkQueue> : public GenericTask {
public:
  Task(WorkQueue &&t) : queue(std::move(t)) { queue.pause(); }
  ~Task(){};

  void run() override { queue.resume().wait(); }

private:
  WorkQueue queue;
};

} // namespace workq