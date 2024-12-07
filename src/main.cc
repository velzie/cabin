#include <filesystem>
#include <openssl/http.h>

#include <fstream>
#include <iostream>
#include <fmt/format.h>
#include <thread>
#include "common.h"
#include "database.h"
#include "server.h"
#include "services/note.h"


Config default_config = {
  .domain = "your.domain",
  .sockethost = "0.0.0.0",
  .socketport = 2001,
  .instanceactor = "test3",
  .mediapath = "media"
};
Config cfg;
json context;

int m();
int main(int argc, char **argv) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("[%M:%S] [%^%L%$] [%&] %v");

  string config_path = "config.json";
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

  std::ifstream contextst("context.json");
  context = json::parse(std::string((std::istreambuf_iterator<char>(contextst)), std::istreambuf_iterator<char>()));

  if (!std::filesystem::exists(cfg.mediapath)) {
    std::filesystem::create_directories(cfg.mediapath);
  }

  info("loaded ({})", cfg.domain); 


  std::thread s1([](){
    // NoteService::fetchRemote("https://brain.worm.pink/objects/1944398f-007c-42e6-8dfd-efcada1500a8");
  });

  Database::Init();
  Server::Listen();
  s1.join();

  // if (argc > 1) {
  //   URL url(argv[1]);
  //   dbg(argv[1]);
  //   auto u = UserService::lookup(ct->userid);
  //   APClient cli(*u, url.host);
  //   auto c = cli.Get(url.path);
  //
  //   if (c->status == 200) {
  //     dbg(c->body);
  //     std::cout << json::parse(c->body).dump(2);
  //   } else {
  //     error("{} : ({})", c->status, c->body);
  //   }
  //
  //   exit(0);
  // }
  //
  // auto u = UserService::lookup(ct->userid);
  // json j = {
  //   {"@context", "https://www.w3.org/ns/activitystreams"},
  //   {"id", API("relayfollows/0")},
  //   {"type", "Follow"},
  //   {"actor", USERPAGE(ct->userid)},
  //   {"object", "https://www.w3.org/ns/activitystreams#Public"}
  // };
  // APClient cli(u.value(), "relay.toot.io");
  // auto c = cli.Post("/inbox", j);
  // error("{} : ({})", c->status, c->body);


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


  return 0;
}
