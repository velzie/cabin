#include "fmt/format.h"
#define _Server httplib::Server
#include <httplib.h>
#include "common.h"
#include "server.h"
#include <stdexcept>


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


std::string dateUTC() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

Server::Server() {
  svr = new httplib::Server();
  if (!svr->is_valid()) {
    throw std::runtime_error("server error lol");
  }


  std::ifstream userkeyf("user.key");
  std::ifstream userpemf("user.pem");
  std::string key((std::istreambuf_iterator<char>(userkeyf)), std::istreambuf_iterator<char>());
  std::string pem((std::istreambuf_iterator<char>(userpemf)), std::istreambuf_iterator<char>());

  svr->Get("/", [](const httplib::Request&, httplib::Response &res) {
    res.set_content("aaaaaaaaaaaaaaa", "text/plain");
  });

  svr->Get("/.well-known/webfinger", [](const httplib::Request &req, httplib::Response &res){
    std::string resource = req.get_param_value("resource");
    std::cout << "r: " << resource << "\n";

    if (resource.rfind("acct:", 0) == 0) {
      resource.erase(0, 5);
    }

    json j = {
      {"links", {
        {
          {"rel", "self"},
          {"type", "application/activity+json"},
          {"href", USERPAGE(ct->userid)}
        },
        {
          {"rel", "http://webfinger.net/rel/profile-page"},
          {"type", "text/html"},
          {"href", USERPAGE(ct->userid)}
        },
        {
          {"rel", "http://ostatus.org/schema/1.0/subscribe"},
          {"template", API("authorize-follow?acct={uri}")}
        }
      }},

      {"subject", req.get_param_value("resource")}
    };

    res.set_content(j.dump(), "application/jrd+json; charset=utf-8");
  });

  svr->Get("/.well-known/host-meta", [](const httplib::Request& req, httplib::Response &res){

      res.set_content(R"(
      <XRD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="http://docs.oasis-open.org/ns/xri/xrd-1.0">
        <Link rel="lrdd" type="application/jrd+json" template="https://rizz.velzie.rip/.well-known/webfinger?resource={uri}" />
        <Link rel="lrdd" type="application/xrd+xml" template="https://rizz.velzie.rip/.well-known/webfinger?resource={uri}" />
      </XRD>
      )", "application/xml");

  });

  svr->Get("/users/:id", [pem](const httplib::Request& req, httplib::Response &res){
    std::string uid = req.path_params.at("id");
    std::cout << "AAAA USER" << uid << "\n";
    std::string userurl = USERPAGE(uid);

    json j = {
      {"@context", ct->context},
      {"type", "Person"},
      {"id", userurl},
      {"inbox", userurl + "/inbox"},
      {"outbox", userurl + "/inbox"},
      {"followers", userurl + "/followers"},
      {"following", userurl + "/following"},
      {"featured", userurl + "/featured"},
      {"sharedinbox", API("inbox")},
      {"endpoints", { 
        {"sharedInbox", API("sharedinbox")} 
      } },
      {"publicKey", {
        {"id", userurl},
        {"owner", userurl},
        {"publicKeyPem", pem},
      }},
      {"url", userurl},
      {"preferredUsername", uid},
      {"name", "the rizzler..."},
      {"summary", "yeah i'm doing this again"},
      {"discoverable", true},
      {"noindex", true},
      {"attachment", {}},
      {"alsoKnownAs", {}}
    };

    res.set_content(j.dump(), "application/activity+json");
  });

  svr->Get("/notes/:id", [](const httplib::Request &req, httplib::Response &res){

    std::string id = "0";
    std::string idurl = API("notes/"+id);
    json j = {
      {"@context", ct->context},
      {"id", idurl},
      {"type", "Note"},
      {"summary", nullptr},
      {"inReplyTo", nullptr},
      {"published", dateUTC()},
      {"url", idurl},
      {"attributedTo", USERPAGE(ct->userid)},
      {"to", {"https://www.w3.org/ns/activitystreams#Public"}},
      {"cc", {USERPAGE(ct->userid)+"/followers"}},
      {"sensitive", false},
      {"content", "i cant think of anything funny to put here sorry"},
      {"attachment", {}},
      {"tag", {}}
    };

    res.set_content(j.dump(), "application/activity+json");
  });


  svr->set_error_handler([](const httplib::Request & /*req*/, httplib::Response &res) {
    const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
    char buf[BUFSIZ];
    snprintf(buf, sizeof(buf), fmt, res.status);
    res.set_content(buf, "text/html");
  });

  svr->set_logger([](const httplib::Request &req, const httplib::Response &res) {
    printf("%s", log(req, res).c_str());
  });
}

void Server::Start() {
  std::cout << "listening" << "\n";
  svr->listen("0.0.0.0", 2001);
}
