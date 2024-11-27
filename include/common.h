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

#ifdef USE_DB
#include "SQLiteCpp/Database.h"
  #define _Database SQLite::Database
#endif

#ifndef USE_DB
  struct _Database;
#endif

using json = nlohmann::json;
using string = std::string;
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
  std::string userid = "test2";
  _Database *db;
  Server server;
};


extern std::shared_ptr<Cabin> ct;


#define API(path) ct->baseurl + path
#define USERPAGE(id) API("users/") + id
#define NOTE(id) API("notes/" + id)
