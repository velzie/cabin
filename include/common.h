#pragma once
#include "spdlog/spdlog.h"
#define DBG_MACRO_NO_WARNING
#include <dbg.h>
#include <nlohmann/json.hpp>
#include <optional>

#define VERSION_LONG "0.1-commit"
#define SOFTWARE "cabin"
#define HOMEPAGE "https://github.com/velzie/cabin"

using nlohmann::json;
using std::string;
using std::optional;
using std::nullopt;
using std::vector;
using namespace spdlog;
#define FMT fmt::format


struct Config {
  string domain;

  string sockethost;
  ushort socketport;

  string instanceactor;

  string mediapath;

  inline string baseurl() {
    return "https://"+domain+"/";
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, domain, instanceactor, sockethost, socketport, mediapath);


inline string JstringOrEmpty(json &j, string name) {
  if (j.contains(name) && j[name].is_string()) {
    return j[name];
  }
  return "";
}
inline bool JboolOrFalse(json &j, string name) {
  if (j.contains(name) && j[name].is_boolean()) {
    return j[name];
  }
  return false;
}


extern Config cfg;
extern json context;


class URL {
  public:
  string scheme;
  string host;
  string port;
  string path;
  string query;
  std::map<string, string> queryMap;
  string frag;

  URL(string s);
  private:
    void parseQuery(const string& queryStr);
};

#define ARR json::array()

#define API(path) cfg.baseurl() + path
#define MAPI(path) API("api/v1/" + path) 
#define USERPAGE(id) API("users/") + id
#define NOTE(id) API("notes/" + id)
#define LIKE(id) API("likes/" + id)
#define REACT(id) API("react/" + id)
#define FOLLOW(id) API("follows/" + id)

#define ASSERT(expr) \
    if (!(expr)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << #expr << ", "\
            << " (" << __FILE__ << ":" << __LINE__ << ")"; \
        throw std::runtime_error(oss.str()); \
    }
