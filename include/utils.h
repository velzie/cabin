#pragma once
#include <common.h>

namespace utils {
  string genid();
  string dateISO();
  time_t millis();

  time_t isoToMillis(const std::string& isoTime);
  string millisToIso(time_t millis);
  time_t clampmillis(time_t t);
  void getStackTrace();
}

bool startsWith(const string& str, const string& prefix);
