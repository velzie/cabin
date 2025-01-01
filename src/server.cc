#include <App.h>
#include <Multipart.h>

#include "Loop.h"
#include "entities/Note.h"
#include "entities/Notification.h"
#include "fmt/format.h"
#include "common.h"
#include "server.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <openssl/err.h>
#include <queue>
#include <shared_mutex>
#include <stdexcept>
#include "router.h"
#include <execinfo.h> 
#include <cpptrace/from_current.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

std::unordered_map<std::string, __Handler> routes_get;
std::unordered_map<std::string, __BodyHandler> routes_post;
std::unordered_map<std::string, __BodyHandler> routes_put;

struct WSSession {
  std::queue<json> queue;
  std::vector<string> streams;
  string userid = cfg.instanceactor;
  int *closed = new int(0);
};
std::mutex managingSessionsLock;
vector<WSSession *> sessions;

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

  void PublishEvent(ServerEvent e) {
    managingSessionsLock.lock();
    if (auto n = std::get_if<Notification>(&e)) {
      for (auto &s : sessions) {
        if (n->notifieeId == s->userid && std::find(s->streams.begin(), s->streams.end(), "user") != s->streams.end()) {
          User u = User::lookupid(s->userid).value();
          s->queue.push({
            {"stream", {"user"}},
            {"event", "notification"},
            {"payload", n->renderMS(u).dump()}
          });
        }
      }
    } else if (auto n = std::get_if<Note>(&e)) {
      // for (auto &s : sessions) {
      //   if (std::find(s->streams.begin(), s->streams.end(), "public") != s->streams.end()) {
      //     s->queue.push(n->toJSON());
      //   }
      // }
    }
    
    managingSessionsLock.unlock();
  }

  void Listen() {
    app = new uWS::App();

    app->options("/*", [](uResponse res, uRequest req){
      res->writeStatus("204");
      res->writeHeader("Access-Control-Allow-Methods", "POST, GET, PUT, OPTIONS, DELETE");
      res->writeHeader("Access-Control-Allow-Headers", "*");
      res->writeHeader("Access-Control-Expose-Headers", HEADERLIST);
      res->writeHeader("Access-Control-Allow-Origin", "*");
      res->writeHeader("Access-Control-Allow-Credentials", "true");
      res->endWithoutBody();
    });

    app->any("/*", [](uResponse res, uRequest req){
      debug("unimplemented: {} {}", req->getMethod(), req->getUrl());

      res->writeStatus("404");
      res->end("unimplemented");
    });

    app->get("/media/*", [](uResponse res, uRequest req){
      string url (req->getUrl());
      url.erase(0, 6);

      string filePath(cfg.mediapath + url);
      std::filesystem::path p = filePath;

      if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
          res->writeHeader("Access-Control-Allow-Origin", "*");
          res->writeHeader("Content-Type", "image/png");

          std::ifstream file(p, std::ios::binary);
          std::ostringstream ss;
          ss << file.rdbuf();
          res->end(ss.str());
      }
      res->writeStatus("404");
      res->end();
    });


    app->ws<WSSession>("/api/v1/streaming", {
        /* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 10,
        .maxBackpressure = 1 * 1024 * 1024,
        /* Handlers */
        .upgrade = nullptr,
        .open = [](uWS::WebSocket<false, true, WSSession> *ws) {
          managingSessionsLock.lock();
          sessions.push_back(ws->getUserData());
          int *closed = ws->getUserData()->closed;
          managingSessionsLock.unlock();
          uWS::Loop::get()->addPostHandler(new int, [ws, closed](uWS::Loop*){
            if (*closed == 1) return;
            managingSessionsLock.lock();
            while (ws->getUserData()->queue.size() > 0) {
              ws->send(ws->getUserData()->queue.front().dump(), uWS::OpCode::TEXT);
              ws->getUserData()->queue.pop();
            }
            managingSessionsLock.unlock();
          });
        },
        .message = [](uWS::WebSocket<false, true, WSSession> *ws, std::string_view message, uWS::OpCode opCode) {
          dbg("message: {}", message);
          json j = json::parse(message);
          if (j["type"] == "subscribe") {
            managingSessionsLock.lock();
            ws->getUserData()->streams.push_back(j["stream"]);
            managingSessionsLock.unlock();
          } else if (j["type"] == "unsubscribe") {
            managingSessionsLock.lock();
            auto &streams = ws->getUserData()->streams;
            streams.erase(std::remove(streams.begin(), streams.end(), j["stream"]), streams.end());
            managingSessionsLock.unlock();
          }
        },
        .drain = [](auto */*ws*/) {
            /* Check getBufferedAmount here */
        },
        .ping = [](auto */*ws*/, std::string_view) {

        },
        .pong = [](auto */*ws*/, std::string_view) {

        },
        .close = [](auto *ws, int /*code*/, std::string_view /*message*/) {
          managingSessionsLock.lock();
          *ws->getUserData()->closed = 1;
          sessions.erase(std::remove(sessions.begin(), sessions.end(), ws->getUserData()), sessions.end());
          managingSessionsLock.unlock();
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
              uWS::MultipartParser mp(strdup(contentType.c_str()));
// https://app.wafrn.net/fediverse/post/f9ada55d-01cb-
// 40c7-bc5c-217d4d20dd10

              json j;
              if (contentType.find("json") != std::string::npos) {
                if (body->str().empty()) j = nullptr;
                else j = json::parse(body->str());
              } else if (mp.isValid()) {
                mp.setBody(strdup(body->str().c_str()));
                
              } else if (contentType == "application/x-www-form-urlencoded") {
                // todo
              }

              route.second(res, req, j, body->str(), mp);

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
      app->put(route.first, [route](uResponse res, uRequest req){
		    auto isAborted = std::make_shared<bool>(false);
		    auto body = std::make_shared<std::stringstream>();
		    res->onData([req, res, isAborted, body, route](std::string_view chunk, bool isFin) mutable {
		      *body << chunk;
          if (isFin && !*isAborted) {
            CPPTRACE_TRY {
              uWS::MultipartParser mp(req->getHeader("content-type"));

              json j;
              if (req->getHeader("content-type").find("json") != std::string::npos) {
                if (body->str().size() > 0)
                  j = json::parse(body->str());
              } else if (mp.isValid()) {
                mp.setBody(body->str());
              } else if (req->getHeader("content-type") == "application/x-www-form-urlencoded") {
                // todo
              }

              route.second(res, req, j, body->str(), mp);

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
