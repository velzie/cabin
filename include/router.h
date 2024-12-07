#pragma once
#include "Multipart.h"
#include <App.h>
#include "common.h"
#include "server.h"
#include "services/user.h"


using uResponse = uWS::HttpResponse<false> *;
using uRequest = uWS::HttpRequest *;

using __Handler = std::function<void(uResponse, uRequest)>;
using __BodyHandler = std::function<void(uResponse, uRequest, json, string, uWS::MultipartParser&)>;
void *register_route(std::string route, __Handler h);
void *register_route_post(std::string route, __BodyHandler h);


#define GET(name, route)\
  void name##_get_handle(uResponse, uRequest);\
  static void * name##_reg = register_route(route, name##_get_handle);\
  void name##_get_handle(uResponse res, uRequest req)

#define POST(name, route)\
  void name##_post_handle(uResponse, uRequest, json body, string bodyraw, uWS::MultipartParser&);\
  static void * name##_reg = register_route_post(route, name##_post_handle);\
  void name##_post_handle(uResponse res, uRequest req, json body, string bodyraw, uWS::MultipartParser&mp)

#define PUT(name, route)\
  void name##_put_handle(uResponse, uRequest, json body, string bodyraw, uWS::MultipartParser&);\
  static void * name##_reg = register_route_post(route, name##_put_handle);\
  void name##_put_handle(uResponse res, uRequest req, json body, string bodyraw, uWS::MultipartParser&mp)


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


#define MSAUTH\
  auto _authuser = UserService::lookup(cfg.instanceactor);\
  User authuser = _authuser.value();
