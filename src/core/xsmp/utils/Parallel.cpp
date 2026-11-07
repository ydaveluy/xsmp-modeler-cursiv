#include <algorithm>
#include <atomic>
#include <exception>
#include <future>
#include <system_error>
#include <vector>
#include <xsmp/utils/Parallel.hpp>

namespace xsmp {

void runInParallel(std::size_t count,
                   const std::function<void(std::size_t)>& task) {
  // Each task has a slot, for a failure that does not depend on the timing.
  std::vector<std::exception_ptr> failures(count);
  std::atomic<std::size_t> next{0};
  const auto work = [&] {
    for (std::size_t index; (index = next++) < count;) {
      try {
        task(index);
      } catch (...) {
        failures[index] = std::current_exception();
      }
    }
  };
#if defined(__wasi__) && !defined(__wasm_atomics__)
  work();
#else
  using Worker = std::future<void>;
  std::vector<Worker> workers;
  try {
    for (auto threads = std::min<std::size_t>(8, count); threads-- > 1;)
      workers.push_back(std::async(std::launch::async, work));
  } catch (const std::system_error&) {
    // Too few threads: those there are drain the tasks.
  }
  work();
  for (auto& worker : workers) worker.get();
#endif
  for (const auto& failure : failures)
    if (failure) std::rethrow_exception(failure);
}

}  // namespace xsmp
