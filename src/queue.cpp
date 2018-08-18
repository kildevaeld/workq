#include <mutex>
#include <queue>
#include <workq/queue.hpp>
#include <workq/task.hpp>
namespace workq {

namespace internal {
class QueuePrivate {
public:
  QueuePrivate() {}
  ~QueuePrivate() {
    while (tasks.size() > 0) {
      GenericTask *task = tasks.front();
      tasks.pop();
      delete task;
    }
  }

  std::queue<GenericTask *> tasks;
  std::mutex mutex;
};
} // namespace internal

Queue::Queue() : d(new internal::QueuePrivate()) {}

Queue::~Queue() {}

GenericTask *Queue::pop() {
  std::lock_guard<std::mutex> lock(d->mutex);
  auto task = d->tasks.front();
  d->tasks.pop();
  return task;
}

void Queue::push(GenericTask *task) {
  std::lock_guard<std::mutex> lock(d->mutex);
  d->tasks.push(task);
}

size_t Queue::size() const {
  std::lock_guard<std::mutex> lock(d->mutex);
  return d->tasks.size();
}

} // namespace workq