#pragma once
#include "spdlog/spdlog.h"
#define DBG_MACRO_NO_WARNING
#include <dbg.h>

#include "../src/server.h"
#include <nlohmann/json.hpp>

#define VERSION_LONG "0.1-commit"
#define SOFTWARE "cabin"
#define HOMEPAGE "https://github.com/velzie/cabin"



#define STATEMENT(string) SQLite::Statement(*ct->db, string);

#include "SQLiteCpp/Database.h"
#define _Database SQLite::Database

using json = nlohmann::json;
using string = std::string;
#define optional std::optional
#define nullopt std::nullopt
using namespace spdlog;
#define FMT fmt::format

struct Config {
  std::string domain;

  std::string host;
  ushort port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, domain);

class Cabin {
  public:
  Cabin(std::string config_path);
  ~Cabin();

  void InitDB();

  Config cfg;
  json context;
  std::string baseurl = "https://rizz.velzie.rip/";
  std::string userid = "test3";
  _Database *db;
};


extern std::shared_ptr<Cabin> ct;


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

#define API(path) ct->baseurl + path
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
