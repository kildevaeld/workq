#pragma once

namespace workq {
class GenericTask;
namespace internal {
class QueuePrivate;
}

class Queue {

public:
  Queue();
  ~Queue();

  void push(GenericTask *task);
  GenericTask *pop();
  size_t size() const;

private:
  std::unique_ptr<internal::QueuePrivate> d;
};

} // namespace workq