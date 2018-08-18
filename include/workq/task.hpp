#pragma once
#include <functional>

namespace workq {

enum class TaskState { Added, Started, Done };

class GenericTask {

public:
  virtual ~GenericTask() {}
  virtual void run() = 0;
};

template <typename T> class Task : public GenericTask {

public:
  Task(T &&t) : task(std::move(t)) {}
  ~Task(){};

  void run() override { task(); }

private:
  T task;
};

template <> class Task<std::function<void()>> : public GenericTask {
public:
  Task(std::function<void()> &&t) : task(std::move(t)) {}
  ~Task(){};

  void run() override { task(); }

private:
  std::function<void()> task;
};

} // namespace workq