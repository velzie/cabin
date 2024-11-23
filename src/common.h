#pragma once

#include "server.h"
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#ifdef USE_DB
#include "SQLiteCpp/Database.h"
  #define _Database SQLite::Database
#endif

#ifndef USE_DB
  struct _Database;
#endif

using json = nlohmann::json;
#define FMT fmt::format

void x();
struct Config {
  std::string domain;

  std::string host;
  ushort port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, domain);

class Cottage {
  public:
  Cottage(std::string config_path);
  ~Cottage();

  Config cfg;
  json context;
  std::string baseurl = "https://rizz.velzie.rip/";
  std::string userid = "test2";
  _Database *db;
  Server server;
};


extern std::shared_ptr<Cottage> ct;


#define API(path) ct->baseurl + path
#define USERPAGE(id) API("users/") + id
