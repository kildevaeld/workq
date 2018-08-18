#include "debug.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <workq/queue.hpp>
#include <workq/work-queue.hpp>

namespace workq {

enum class State { Running, Paused, Done };

namespace internal {

class WorkQueuePrivate {

public:
  WorkQueuePrivate(size_t cap, const std::string &n)
      : capacity(cap), threads(cap), name(n) {
    for (int i = 0; i < cap; i++) {
      threads[i] =
          std::thread(std::bind(&WorkQueuePrivate::dispatch_handler, this));
    }
  }

  ~WorkQueuePrivate() {
    std::unique_lock<std::mutex> lock(mutex);
    state = State::Done;
    lock.unlock();
    condition.notify_all();
    for (size_t i = 0; i < threads.size(); i++) {
      if (threads[i].joinable()) {
        DEBUG("%s#Destructor: Joining thread %zu until completion",
              name.c_str(), i);
        threads[i].join();
      }
    }
  }

  void dispatch_handler();

  int capacity;
  Queue queue;
  std::mutex mutex;
  std::condition_variable condition;
  std::vector<std::thread> threads;
  State state = State::Running;
  std::string name;
  std::atomic<size_t> running = {0};
  HookCallback hook = nullptr;
};

void WorkQueuePrivate::dispatch_handler() {
  std::unique_lock<std::mutex> lock(mutex);

  for (;;) {
    if (state == State::Done)
      break;

    condition.wait(lock, [this]() {
      auto ret =
          state == State::Done || (state == State::Running && queue.size() > 0);
      if (!ret)
        condition.notify_one();
      return ret;
    });

    if (state != State::Running)
      continue;

    lock.unlock();
    auto task = queue.pop();
    if (hook)
      hook(TaskState::Started, task, Stats{queue.size(), running});
    running++;
    task->run();
    if (hook)
      hook(TaskState::Done, task, Stats{queue.size(), running});
    delete task;
    running--;
    condition.notify_one();
    lock.lock();
  }
}

} // namespace internal

WorkQueue::WorkQueue(size_t num, bool paused)
    : d(new internal::WorkQueuePrivate(num, "")) {
  if (paused)
    pause();
}

WorkQueue::WorkQueue(const std::string &name, size_t capacity, bool paused)
    : d(new internal::WorkQueuePrivate(capacity, name)) {
  if (paused)
    pause();
}

WorkQueue::~WorkQueue() {}

WorkQueue::WorkQueue(WorkQueue &&other) : d(std::move(other.d)) {}

WorkQueue &WorkQueue::operator=(WorkQueue &&other) {
  if (this != &other) {
    d.swap(other.d);
  }

  return *this;
}

void WorkQueue::set_hook(HookCallback callback) { d->hook = callback; }

WorkQueue &WorkQueue::dispatch(GenericTask *task) {
  d->queue.push(task);
  if (d->hook)
    d->hook(TaskState::Added, task, Stats{d->queue.size(), d->running});
  d->condition.notify_one();
  return *this;
}

WorkQueue &WorkQueue::pause() {
  std::unique_lock<std::mutex> lock(d->mutex);
  if (d->state != State::Paused) {
    d->state = State::Paused;
    lock.unlock();
    d->condition.notify_all();
  }
  return *this;
}

WorkQueue &WorkQueue::resume() {
  std::unique_lock<std::mutex> lock(d->mutex);
  if (d->state != State::Running) {
    d->state = State::Running;
    lock.unlock();
    d->condition.notify_all();
  }
  return *this;
}

void WorkQueue::wait() const {
  std::unique_lock<std::mutex> lock(d->mutex);

  d->condition.wait(lock, [this] {
    DEBUG("%s WAIT q:%lu r:%lu", d->name.c_str(), d->queue.size(),
          d->running.load());
    auto ret =
        d->state == State::Running && d->queue.size() == 0 && d->running == 0;
    if (!ret)
      d->condition.notify_one();
    return ret;
  });
  DEBUG("%s WAIT", d->name.c_str());
}

size_t WorkQueue::size() const { return d->queue.size(); }
size_t WorkQueue::capacity() const { return d->capacity; }

size_t WorkQueue::running() const { return d->running; }

} // namespace workq