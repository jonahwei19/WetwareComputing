#include "utils.hpp"

#include <cstdint>
#include <sys/time.h>

/* Return the current system time since epoch
 * Definition copied from juce_time.h */
std::int64_t CurrentTimeMillis() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return ((std::int64_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

void LogText(const std::string &text) {
  std::cout << "[" << CurrentTimeMillis() << "] " << text << std::endl;
}
