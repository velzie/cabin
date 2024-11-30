#pragma once

#include <App.h>
#include "../src/server.h"
#include "common.h"


using uResponse = uWS::HttpResponse<false> *;
using uRequest = uWS::HttpRequest *;

using __Handler = std::function<void(uResponse, uRequest)>;
void *register_route(std::string route, __Handler h);
void *register_route_post(std::string route, std::function<void(uResponse, uRequest, json, string)> h);


#define GET(name, route)\
  void name##_get_handle(uResponse, uRequest);\
  static void * name##_reg = register_route(route, name##_get_handle);\
  void name##_get_handle(uResponse res, uRequest req)

#define POST(name, route)\
  void name##_post_handle(uResponse, uRequest, json body, string bodyraw);\
  static void * name##_reg = register_route_post(route, name##_post_handle);\
  void name##_post_handle(uResponse res, uRequest req, json body, string bodyraw)


#define MIMEJRD "application/jrd+json; charset=utf-8"
#define MIMEJSON "application/json; charset=utf-8"
#define MIMEAP "application/activity+json; charset=utf-8"

#define REDIRECT(url)\
  res->writeStatus("302");\
  res->writeHeader("Location", url);\
  res->end();\
  return

#define OK(json, mime)\
  trace("200 ok {}", req->getUrl());\
  res->writeStatus("200");\
  res->writeHeader("Content-Type", mime);\
  res->writeHeader("Access-Control-Allow-Origin", "*");\
  res->write(json.dump());\
  res->end();\
  return

#define ERROR(status, text)\
  error("returned {} from {}: {}", status, req->getUrl(), text);\
  res->writeStatus(#status);\
  res->writeHeader("Access-Control-Allow-Origin", "*");\
  res->writeHeader("Content-Type", "text/plain");\
  res->write(text);\
  res->end();\
  return
