#pragma once
#include "spdlog/spdlog.h"
#define DBG_MACRO_NO_WARNING
#include <dbg.h>

#include <nlohmann/json.hpp>

#define VERSION_LONG "0.1-commit"
#define SOFTWARE "cabin"
#define HOMEPAGE "https://github.com/velzie/cabin"

using nlohmann::json;
using std::string;
using std::optional;
using std::nullopt;
using namespace spdlog;
#define FMT fmt::format


struct Config {
  string domain;

  string sockethost;
  ushort socketport;

  string instanceactor;


  inline string baseurl() {
    return "https://"+domain+"/";
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, domain, instanceactor);



extern Config cfg;
extern json context;


class URL {
  public:
  string scheme;
  string host;
  string port;
  string path;
  string query;
  string frag;

  URL(string s);
};

#define ARR json::array()

#define API(path) cfg.baseurl() + path
#define MAPI(path) API("api/v1/" + path) 
#define USERPAGE(id) API("users/") + id
#define NOTE(id) API("notes/" + id)
#define LIKE(id) API("likes/" + id)
#define FOLLOW(id) API("follows/" + id)

#define ASSERT(expr) \
    if (!(expr)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << #expr << ", "\
            << " (" << __FILE__ << ":" << __LINE__ << ")"; \
        throw std::runtime_error(oss.str()); \
    }
