#pragma once

#include <cstddef>
#include <functional>

namespace xsmp {

/// Calls `task` once with each index below `count`, on at most eight threads:
/// the calling one and seven others. Every task runs, even after another one
/// failed. Then the failure of the lowest index is rethrown, so it does not
/// depend on the timing of the threads. Without threads, the calling thread
/// runs them all.
void runInParallel(std::size_t count,
                   const std::function<void(std::size_t)>& task);

}  // namespace xsmp
