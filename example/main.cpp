#include <chrono>
#include <iostream>
#include <thread>
#include <workq/workq++.hpp>

#include <random>

static int rangeRandomAlg2(int min, int max) {
  int n = max - min + 1;
  int remainder = RAND_MAX % n;
  int x;
  do {
    x = rand();
  } while (x >= RAND_MAX - remainder);
  return min + x % n;
}

class Task : public workq::GenericTask {

public:
  Task(const std::string &name, int number = 0, size_t duration = 800)
      : m_name(name), m_number(number), m_dur(duration) {}

  void run() {
    printf("Name %s#%d waiting for %lu\n", m_name.c_str(), m_number, m_dur);
    std::this_thread::sleep_for(std::chrono::milliseconds(m_dur));
    printf("Done waiting %s#%d\n", m_name.c_str(), m_number);
  }

private:
  std::string m_name;
  size_t m_dur;
  int m_number;
};

int main() {

  workq::WorkQueue q(4, true);

  q.dispatch<Task>("Bjorn", 1, 102);
  q.dispatch<Task>("Thomas", 2, 101);
  q.dispatch<Task>("Peter", 3, 130);
  q.dispatch<Task>("Hans", 4, 15000);
  q.dispatch<Task>("Jørgen", 5, 2000);
  q.dispatch<Task>("Oste", 6, 100);
  q.dispatch<Task>("Rapper Von Dyke", 7, 102);
  q.dispatch<Task>("Lars", 8, 100);

  q.set_hook([](workq::TaskState state, workq::GenericTask *task,
                const workq::Stats &stat) {
    printf("q1 running %d: %lu queue %lu\n", state, stat.running, stat.queue);
  });

  workq::WorkQueue q2(4, true);

  for (int i = 0; i < 150; i++) {
    q2.dispatch<Task>("rapper", i, 100);
  }

  q2.set_hook([](workq::TaskState state, workq::GenericTask *task,
                 const workq::Stats &stat) {
    printf("q2 running %d: %lu queue %lu\n", state, stat.running, stat.queue);
  });

  q.dispatch(std::move(q2));

  q.resume().wait();
  // workq::DispatchQueue q("q1", 4, false);

  // auto cb = [](int i) {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(300 * i));
  //   std::cout << "Thread " << i << " " << std::this_thread::get_id()
  //             << std::endl;
  // };

  // for (int i = 0; i < 10; i++) {
  //   q.async(std::bind(cb, i));
  // }

  // // workq::DispatchQueue q2("q2", 1);

  // // q2.async([]() { std::cout << "q2 Hello" << std::endl; });
  // // q2.async([]() { std::cout << "q2 Hello 2" << std::endl; });
  // // q2.async<Task>("test");
  // // q.async(std::move(q2));
  // // q2.async<Task>("Bjarne Hovsasa 2 ", 400);
  // q.async<Task>("Bjarne Hovsasa");

  // q.resume().wait();
  // // std::cout << "Wait " << std::this_thread::get_id() << std::endl;

  // return 0;

  // workq::WorkQueue q(4);
  // q.pause();
  // for (int i = 0; i < 10; i++) {
  //   q.dispatch<Task>("Hello, World", i, rangeRandomAlg2(100, 400));
  // }

  // workq::WorkQueue q2("q2", 1);

  // q2.dispatch([]() { std::cout << "q2 Hello" << std::endl; });
  // q2.dispatch([]() { std::cout << "q2 Hello 2" << std::endl; });
  // for (int i = 0; i < 10; i++) {
  //   q2.dispatch<Task>("Hello, World 2", i, rangeRandomAlg2(100, 400));
  // }

  // q2.dispatch<Task>("test");
  // q.dispatch(std::move(q2));
  // // q2.dispatch<Task>("Bjarne Hovsasa 2 ", 400);
  // q.dispatch<Task>("Bjarne Hovsasa");

  // q.resume().wait();

  return 0;
}