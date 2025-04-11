#include "common.h"
#include <cpptrace/from_current.hpp>
#include <random>
#include <regex>

time_t parseiso8601utc(const char *date) {
  struct tm tt = {0};
  double seconds;
  if (sscanf(date, "%04d-%02d-%02dT%02d:%02d:%lfZ", &tt.tm_year, &tt.tm_mon,
             &tt.tm_mday, &tt.tm_hour, &tt.tm_min, &seconds) != 6)
    return -1;
  tt.tm_sec = (int)seconds;
  tt.tm_mon -= 1;
  tt.tm_year -= 1900;
  tt.tm_isdst = -1;
  return mktime(&tt) - timezone;
}

namespace utils {

// id format is timestamp(base64) + 8 random bytes(base64)
// will this work? no clue
string genid() {
  int random_length = 8;
  const std::string chars =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::string random_suffix;
  random_suffix.reserve(random_length);

  auto now = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now.time_since_epoch())
                      .count();

  std::stringstream timestamp_stream;
  timestamp_stream << std::hex << duration;

  thread_local std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<> dist(0, chars.size() - 1);

  for (size_t i = 0; i < random_length; ++i) {
    random_suffix += chars[dist(rng)];
  }

  return timestamp_stream.str() + random_suffix;
}

string dateISO() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm utc_tm = *std::gmtime(&now_c);
  std::ostringstream oss;
  oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

time_t millis() {
  auto now = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch());
  return duration.count();
}

void getStackTrace() {
  auto t = cpptrace::from_current_exception();

  t.frames.erase(std::remove_if(t.frames.begin(), t.frames.end(), [](auto f)
  {
    return f.filename.find(".third-party") != std::string::npos ||
    f.filename.find("/usr") != std::string::npos;
  }), t.frames.end());

  t.print_with_snippets();
  // free everything
}

time_t clampmillis(time_t t) {
  auto now = millis();
  if (t > now)
    return now;
  return t;
}

string millisToIso(time_t millis) {

    // use standard timezone, include millis
    std::tm utc_tm = *std::gmtime(&millis);
    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S.") << std::setfill('0') << std::setw(3) << millis % 1000 << "Z";
    return oss.str();
}

time_t isoToMillis(const std::string &isoTime) {
   return parseiso8601utc(isoTime.c_str());
}


} // namespace utils

bool startsWith(const string& str, const string& prefix) {
  return str.substr(0, prefix.length()) == prefix;
}

URL::URL(string s) {
  std::regex urlRegex(
      R"(([^:]+)://([^:/?#]+)(?::([0-9]+))?([^?#]*)(\?([^#]*))?(#(.*))?)");
  std::smatch urlMatch;

  if (regex_match(s, urlMatch, urlRegex)) {
    // Assign parts of the URL
    scheme = urlMatch[1];
    host = urlMatch[2];
    port = urlMatch[3];
    path = urlMatch[4];
    query = urlMatch[6];
    frag = urlMatch[8];

    // Parse query into key-value pairs
    if (!query.empty()) {
      parseQuery(query);
    }
  } else {
    throw std::invalid_argument("Invalid URL format");
  }
}
void URL::parseQuery(const string &queryStr) {
  std::stringstream ss(queryStr);
  string pair;

  while (getline(ss, pair, '&')) {
    size_t eqPos = pair.find('=');
    if (eqPos != string::npos) {
      string key = pair.substr(0, eqPos);
      string value = pair.substr(eqPos + 1);
      queryMap[key] = value;
    } else {
      // Key without a value
      queryMap[pair] = "";
    }
  }
}
