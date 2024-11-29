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
std::unordered_map<std::string, std::function<void(uResponse, uRequest, json, string)>> routes_post;
void *register_route(std::string route, __Handler h) {
  routes_get[route] = h;
  return nullptr;
}

void *register_route_post(std::string route, std::function<void(uResponse, uRequest, json, string)> h) {
  routes_post[route] = h;
  return nullptr;
}

namespace Server {
  uWS::App *app;

  void Init() {
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
    
    for (const auto route : routes_get) {
      app->get(route.first, [route](uResponse res, uRequest req){
        CPPTRACE_TRY {
          ASSERT_THROW(0,"");
          route.second(res, req);


          if (!res->hasResponded()) {
            res->writeStatus("204");
            res->endWithoutBody();
          }
        } CPPTRACE_CATCH (const std::exception& e){
          res->writeHeader("Content-Type", "text/plain");
          auto t = cpptrace::from_current_exception();

          string ex = FMT("Exception while responding to {}\n{}\n", route.first, e.what());

          t.frames.erase(std::remove_if(t.frames.begin(), t.frames.end(), [](auto f) {
            return f.filename.find(".third-party") != std::string::npos || f.filename.find("/usr") != std::string::npos;
          }), t.frames.end());

          std::stringstream s;
          t.print_with_snippets(s);
          ex += s.str();


          std::cout << ex;
          res->end(ex);
        }
      });
    }

    for (const auto route : routes_post) {
      app->post(route.first, [route](uResponse res, uRequest req){
		    auto isAborted = std::make_shared<bool>(false);
		    auto body = std::make_shared<std::stringstream>();
		    res->onData([req, res, isAborted, body, route](std::string_view chunk, bool isFin) mutable {
		      *body << chunk;
          if (isFin && !*isAborted) {
            CPPTRACE_TRY {
              uWS::MultipartParser mp(req->getHeader("content-type"));

              json j;
              if (req->getHeader("content-type") == "application/json") {
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
              res->writeHeader("Content-Type", "text/plain");
              auto t = cpptrace::from_current_exception();

              string ex = FMT("Exception while responding to {}\n{}\n", route.first, e.what());

              t.frames.erase(std::remove_if(t.frames.begin(), t.frames.end(), [](auto f) {
                return f.filename.find(".third-party") != std::string::npos || f.filename.find("/usr") != std::string::npos;
              }), t.frames.end());

              std::stringstream s;
              t.print_with_snippets(s);
              ex += s.str();


              std::cout << ex;
              res->end(ex);
            }
          }
        });
        res->onAborted([isAborted](){
            *isAborted = true;
        });
      });
    }
  }

  void Listen() {
    app->listen(2001,[](auto *listen_socket) {
	    if (listen_socket) {
			  std::cout << "Listening on port " << 3000 << std::endl;
	    }
	  }).run();
  }
}
