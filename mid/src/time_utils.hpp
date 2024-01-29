#ifndef MID_TIME_UTILS_H_
#define MID_TIME_UTILS_H_
#include <cstdint>
#include <string>
typedef std::int64_t JuceTime;
JuceTime CurrentTimeMillis();
void LogText(const std::string &text);
#endif
