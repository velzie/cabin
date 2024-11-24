#include "fmt/format.h"
#define _Server httplib::Server
#include <httplib.h>
#include "common.h"
#include "server.h"
#include <stdexcept>
#include "router.h"


std::string dump_headers(const httplib::Headers &headers) {
  std::string s;
  char buf[BUFSIZ];

  for (auto it = headers.begin(); it != headers.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
    s += buf;
  }

  return s;
}

std::string log(const httplib::Request &req, const httplib::Response &res) {
  // std::string s;
  // char buf[BUFSIZ];
  //
  // s += "================================\n";
  //
  // snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
  //          req.version.c_str(), req.path.c_str());
  // s += buf;
  //
  // std::string query;
  // for (auto it = req.params.begin(); it != req.params.end(); ++it) {
  //   const auto &x = *it;
  //   snprintf(buf, sizeof(buf), "%c%s=%s",
  //            (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
  //            x.second.c_str());
  //   query += buf;
  // }
  // snprintf(buf, sizeof(buf), "%s\n", query.c_str());
  // s += buf;
  //
  // s += dump_headers(req.headers);
  //
  // s += "--------------------------------\n";
  //
  // snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
  // s += buf;
  // s += dump_headers(res.headers);
  // s += "\n";
  //
  // if (!res.body.empty()) { s += res.body; }
  //
  // s += "\n";

  // return s;
  

  std::string sr;
  sr += FMT("{} {} {}\n", req.method, req.version, req.path);

  auto ua = req.headers.find("User-Agent");
  if (ua != req.headers.end()) {
    sr += FMT("ua: {}\n", ua->second);
  }
  auto sig = req.headers.find("Signature");
  if (sig != req.headers.end()) {
    sr += FMT("sig: {}\n", sig->second);
  }
  return sr;
}

Server::~Server() {
  delete svr;
}


std::map<std::string, __Handler> routes_get;
std::map<std::string, __Handler> routes_post;
void *register_route(std::string route, __Handler h) {
  routes_get[route] = h;
  return nullptr;
}

void *register_route_post(std::string route, __Handler h) {
  routes_post[route] = h;
  return nullptr;
}

Server::Server() {
  svr = new httplib::Server();
  if (!svr->is_valid()) {
    throw std::runtime_error("server error lol");
  }

  for (const auto route : routes_get) {
    svr->Get(route.first, [route, this](const httplib::Request &req, httplib::Response &res) {
      route.second(req, res, this);
    });
  }

  for (const auto route : routes_post) {
    info("{}", route.first);
    svr->Post(route.first, [route, this](const httplib::Request &req, httplib::Response &res) {
      route.second(req, res, this);
    });
  }

  svr->Get("/", [](const httplib::Request&, httplib::Response &res) {
    res.set_content("aaaaaaaaaaaaaaa", "text/plain");
  });

  // svr->set_error_handler([](const httplib::Request & /*req*/, httplib::Response &res) {
  //   const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
  //   char buf[BUFSIZ];
  //   snprintf(buf, sizeof(buf), fmt, res.status);
  //   res.set_content(buf, "text/html");
  // });

  svr->set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
    auto fmt = "<h1>Error 500</h1><p>%s</p>";
    char buf[BUFSIZ];
    try {
      std::rethrow_exception(ep);
    } catch (std::exception &e) {
      snprintf(buf, sizeof(buf), fmt, e.what());
    } catch (...) { // See the following NOTE
      snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
    }
    res.set_content(buf, "text/html");
    res.status = 500;
  });

  svr->set_logger([](const httplib::Request &req, const httplib::Response &res) {
    printf("%s", log(req, res).c_str());
  });
}

void Server::Start() {
  std::cout << "listening" << "\n";
  svr->listen("0.0.0.0", 2001);
}
