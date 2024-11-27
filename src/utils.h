#pragma once
#include <common.h>
#include <random>

namespace utils {
  inline string genid() {
    int length = 16;
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr))); 
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    for (size_t i = 0; i < length; ++i) {
        result += chars[dist(rng)];
    }

    return result;
  }
  inline string dateISO() {
      auto now = std::chrono::system_clock::now();
      std::time_t now_c = std::chrono::system_clock::to_time_t(now);
      std::tm utc_tm = *std::gmtime(&now_c);
      std::ostringstream oss;
      oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
      return oss.str();
  }

  inline long long millis() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
  }

  inline long long isoToMillis(const std::string& isoTime) {
    std::tm tm = {};
    std::istringstream ss(isoTime);

    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        throw std::runtime_error("Failed to parse time");
    }

    size_t dotPos = isoTime.find('.');
    int milliseconds = 0;
    if (dotPos != std::string::npos) {
        milliseconds = std::stoi(isoTime.substr(dotPos + 1, 3));
    }

    std::time_t timeSinceEpoch = std::mktime(&tm);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::seconds(timeSinceEpoch));

    return duration.count() + milliseconds;
  }

  inline string millisToIso(long long millis) {
      std::time_t time = millis;
      std::tm utc_tm = *std::gmtime(&time);
      std::ostringstream oss;
      oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
      return oss.str();
  }
}
