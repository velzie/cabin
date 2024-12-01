#include "spdlog/spdlog-inl.h"
#include <openssl/http.h>
#include <stdexcept>
#define USE_DB

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <thread>
#include "common.h"
#include "schema.h"
#include "http.h"
#include "services/user.h"
#include "utils.h"
#include <cpptrace/cpptrace.hpp>


Config default_config = {
  .domain = "your.domain",
  .host = "0.0.0.0",
  .port = 2001,
};

URL::URL(string s) {
  char *_scheme;
  char *_user;
  char *_host;
  char *_port;
  int _port_num;
  char *_path;
  char *_query;
  char *_frag;

  OSSL_parse_url(s.c_str(), &_scheme, &_user, &_host, &_port, &_port_num, &_path, &_query, &_frag);

  scheme = _scheme;
  host = _host;
  port = _port;
  path = _path;
  query = _query;
  frag = _frag;
}

Cabin::~Cabin() {
  delete db;
}
Cabin::Cabin(std::string config_path) {
  std::ifstream s(config_path);

  if (!s) {
    std::ofstream s(config_path, std::ios::app);
    if (!s) {
      std::cout << "can't read config" << "\n";
    }
    json j = default_config;
    s << j.dump();
    s.close();
    std::cout << "No config file found, wrote one for you";
    exit(1);
  }

  json j = json::parse(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()));
  cfg = j.template get<Config>();

  info("loaded ({})", cfg.domain);
}

std::shared_ptr<Cabin> ct;

void registeruser();
int main(int argc, char **argv) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("[%M:%S] [%^%L%$] [%&] %v");
  

  ct = std::make_shared<Cabin>("config.json");

  std::ifstream contextst("context.json");
  ct->context = json::parse(std::string((std::istreambuf_iterator<char>(contextst)), std::istreambuf_iterator<char>()));

  ct->InitDB();
  registeruser();


  std::thread tserver([](){
    Server::Init();
    Server::Listen();
  });

  if (argc > 1) {
    URL url(argv[1]);
    dbg(argv[1]);
    auto u = UserService::lookup(ct->userid);
    APClient cli(*u, url.host);
    auto c = cli.Get(url.path);

    if (c->status == 200) {
      dbg(c->body);
      std::cout << json::parse(c->body).dump(2);
    } else {
      error("{} : ({})", c->status, c->body);
    }

    exit(0);
  }



  // json j = {
  //   {"id", API("follows/0")},
  //   {"type", "Follow"},
  //   {"actor", USERPAGE(ct->userid)},
  //   {"object", "https://booping.synth.download/users/a005c9wl4pwj0arp"}
  // };
  // APClient cli(u.value(), "booping.synth.download");
  // auto c = cli.Post("/inbox", j);
  // error("{} : ({})", c->status, c->body);
  
 // json j = {
 //    {"id", API("bites/0")},
 //    {"type", "Bite"},
 //    {"actor", USERPAGE(ct->userid)},
 //    {"target", "https://is.notfire.cc/users/9yr98tgk15rj9zaw"},
 //    {"to", "https://is.notfire.cc/users/9yr98tgk15rj9zaw"},
 //    {"published", utils::dateISO()},
 //  };
 //  APClient cli(u.value(), "is.notfire.cc");
 //  auto c = cli.Post("/inbox", j);
 //  error("{} : ({})", c->status, c->body);

 // json j = {
 //    {"id", API("bites/0")},
 //    {"type", "Bite"},
 //    {"actor", USERPAGE(ct->userid)},
 //    {"target", "https://is.notfire.cc/users/9yr98tgk15rj9zaw"},
 //    {"to", "https://is.notfire.cc/users/9yr98tgk15rj9zaw"},
 //    {"published", utils::dateISO()},
 //  };
 //  APClient cli(u.value(), "is.notfire.cc");
 //  auto c = cli.Post("/inbox", j);
 //  error("{} : ({})", c->status, c->body);


  tserver.join();
  return 0;
}
