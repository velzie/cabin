#include <App.h>
#include <Multipart.h>

#include "fmt/format.h"
#include "common.h"
#include "server.h"
#include <stdexcept>
#include "router.h"
#include <execinfo.h> 
#include <cpptrace/from_current.hpp>

std::unordered_map<std::string, __Handler> routes_get;
std::unordered_map<std::string, __BodyHandler> routes_post;
std::unordered_map<std::string, __BodyHandler> routes_put;
void *register_route(std::string route, __Handler h) {
  routes_get[route] = h;
  return nullptr;
}
void *register_route_post(std::string route, __BodyHandler h) {
  routes_post[route] = h;
  return nullptr;
}
void *register_route_put(std::string route, __BodyHandler h) {
  routes_put[route] = h;
  return nullptr;
}


string getStackTrace() {
  auto t = cpptrace::from_current_exception();
  t.frames.erase(std::remove_if(t.frames.begin(), t.frames.end(), [](auto f) {
    return f.filename.find(".third-party") != std::string::npos || f.filename.find("/usr") != std::string::npos;
  }), t.frames.end());

  std::stringstream s;
  t.print_with_snippets(s, false);
  t.print_with_snippets(std::cerr, true);

  return s.str();
}

namespace Server {
  uWS::App *app;

  void Listen() {
    app = new uWS::App();

    app->options("/*", [](uResponse res, uRequest req){
      res->writeStatus("204");
      res->writeHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE");
      res->writeHeader("Access-Control-Allow-Headers", "*");
      res->writeHeader("Access-Control-Allow-Origin", "*");
      res->writeHeader("Access-Control-Allow-Credentials", "true");
      res->endWithoutBody();
    });

    app->any("/*", [](uResponse res, uRequest req){
      debug("unimplemented: {} {}", req->getMethod(), req->getUrl());

      res->writeStatus("404");
      res->end("unimplemented");
    });
    struct PerSocketData {
        /* Fill with user data */
    };
    app->ws<PerSocketData>("/*", {
        /* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 10,
        .maxBackpressure = 1 * 1024 * 1024,
        /* Handlers */
        .upgrade = nullptr,
        .open = [](auto */*ws*/) {

        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            ws->send(message, opCode);
        },
        .drain = [](auto */*ws*/) {
            /* Check getBufferedAmount here */
        },
        .ping = [](auto */*ws*/, std::string_view) {

        },
        .pong = [](auto */*ws*/, std::string_view) {

        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {

        }
    });

    for (const auto route : routes_get) {
      app->get(route.first, [route](uResponse res, uRequest req){
        CPPTRACE_TRY {
          route.second(res, req);


          if (!res->hasResponded()) {
            res->writeStatus("204");
            res->endWithoutBody();
          }
        } CPPTRACE_CATCH (const std::exception& e){
          res->writeStatus("500");
          res->writeHeader("Content-Type", "text/plain");
          res->writeHeader("Access-Control-Allow-Origin", "*");

          string ex = FMT("Exception while responding to {}\n{}\n", route.first, e.what());
          std::cout << ex;
          res->write(ex);
          res->end(getStackTrace());
        }
      });
    }

    for (const auto route : routes_post) {
      app->post(route.first, [route](uResponse res, uRequest req){
        string contentType(req->getHeader("content-type"));
		    auto isAborted = std::make_shared<bool>(false);
		    auto body = std::make_shared<std::stringstream>();
		    res->onData([req, res, isAborted, body, route, contentType](std::string_view chunk, bool isFin) mutable {
		      *body << chunk;
          if (isFin && !*isAborted) {
            CPPTRACE_TRY {
              uWS::MultipartParser mp(contentType);
// https://app.wafrn.net/fediverse/post/f9ada55d-01cb-
// 40c7-bc5c-217d4d20dd10

              json j;
              if (contentType.find("json") != std::string::npos) {
                if (body->str().empty()) j = nullptr;
                else j = json::parse(body->str());
              } else if (mp.isValid()) {
                mp.setBody(body->str());
              } else if (contentType == "application/x-www-form-urlencoded") {
                // todo
              }

              route.second(res, req, j, body->str());

              if (!res->hasResponded()) {
                res->writeStatus("204");
                res->endWithoutBody();
              }
            } CPPTRACE_CATCH(const std::exception& e) {
              res->writeStatus("500");
              res->writeHeader("Content-Type", "text/plain");
              res->writeHeader("Access-Control-Allow-Origin", "*");

              string ex = FMT("Exception while responding to {}\n{}\n", route.first, e.what());
              std::cout << ex;
              res->write(ex);
              res->end(getStackTrace());
            }
          }
        });
        res->onAborted([isAborted](){
            *isAborted = true;
        });
      });
    }

    for (const auto route : routes_put) {
      app->post(route.first, [route](uResponse res, uRequest req){
		    auto isAborted = std::make_shared<bool>(false);
		    auto body = std::make_shared<std::stringstream>();
		    res->onData([req, res, isAborted, body, route](std::string_view chunk, bool isFin) mutable {
		      *body << chunk;
          if (isFin && !*isAborted) {
            CPPTRACE_TRY {
              uWS::MultipartParser mp(req->getHeader("content-type"));

              json j;
              if (req->getHeader("content-type").find("json") != std::string::npos) {
                j = json::parse(body->str());
              } else if (mp.isValid()) {
                mp.setBody(body->str());
              } else if (req->getHeader("content-type") == "application/x-www-form-urlencoded") {
                // todo
              }

              route.second(res, req, j, body->str());

              if (!res->hasResponded()) {
                res->writeStatus("204");
                res->endWithoutBody();
              }
            } CPPTRACE_CATCH(const std::exception& e) {
              res->writeStatus("500");
              res->writeHeader("Content-Type", "text/plain");
              res->writeHeader("Access-Control-Allow-Origin", "*");

              string ex = FMT("Exception while responding to {}\n{}\n", route.first, e.what());
              std::cout << ex;
              res->write(ex);
              res->end(getStackTrace());
            }
          }
        });
        res->onAborted([isAborted](){
            *isAborted = true;
        });
      });
    }

    app->listen(2001,[](auto *listen_socket) {
	  }).run();
  }
}
