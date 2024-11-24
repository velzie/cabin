#pragma once

#include <httplib.h>
#include "../src/server.h"


using __Handler = std::function<void(const httplib::Request &, httplib::Response &, Server *srv)>;
void *register_route(std::string route, __Handler h);
void *register_route_post(std::string route, __Handler h);

#define GET(name, route)\
  class name##_classdef : Server {\
    public:\
    void handle(const httplib::Request& req, httplib::Response &res);\
  };\
  static void * name##_reg = register_route(route, [](const httplib::Request& req, httplib::Response &res, Server *srv){\
    ((name##_classdef*)srv)->handle(req, res);\
  });\
  void name##_classdef::handle(const httplib::Request& req, httplib::Response &res)

#define POST(name, route)\
  class name##_classdef : Server {\
    public:\
    void handle(const httplib::Request& req, httplib::Response &res);\
  };\
  static void * name##_reg = register_route_post(route, [](const httplib::Request& req, httplib::Response &res, Server *srv){\
    ((name##_classdef*)srv)->handle(req, res);\
  });\
  void name##_classdef::handle(const httplib::Request& req, httplib::Response &res)

