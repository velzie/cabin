#pragma once
#define USE_DB
#include <common.h>
#include <httplib.h>
#include "schema.h"

class APClient {
  std::string instance;
  httplib::Client cli; 
  User &user;

  public:
    APClient(User &u, std::string hostname);
    httplib::Result Get(std::string pathname);
    httplib::Result Post(std::string pathname, nlohmann::json data);
};


