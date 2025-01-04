#include <fcntl.h>
#include <filesystem>
#include <openssl/http.h>

#include <fstream>
#include <iostream>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <thread>
#include "common.h"
#include "database.h"
#include "http.h"
#include "server.h"
#include "utils.h"
#include "workers/BubbleFetcher.h"


Config default_config = {
  .domain = "your.domain",
  .sockethost = "0.0.0.0",
  .socketport = 2001,
  .instanceactor = "test3",
  .mediapath = "media",
  .bubbledHosts = {"wetdry.world"}
};
Config cfg;
json context;

#define LOCK_FILE "/tmp/cabin.lock"

void registeruser();
int main(int argc, char **argv) {
  spdlog::set_level(spdlog::level::debug);
  if (argc > 1 && string(argv[1]) == "trace") {
    spdlog::set_level(spdlog::level::trace);
  }
  spdlog::set_pattern("[%M:%S] [%^%L%$] [%&] %v");

  int lockFile = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
  if (lockFile < 0) {
      std::cerr << "Error: Unable to open lock file." << std::endl;
      exit(EXIT_FAILURE);
  }

  if (lockf(lockFile, F_TLOCK, 0) < 0) {
      std::cerr << "Error: Unable to lock file." << std::endl;
      exit(EXIT_FAILURE);
  }

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


  Database::Init();
  BubbleFetcher::Launch();
  std::thread s1([](){
    // NoteService::fetchRemote("https://brain.worm.pink/objects/1944398f-007c-42e6-8dfd-efcada1500a8");
  });

  vector<std::thread> clusters;
  for (int i = 0; i < 20; i++) {
    clusters.push_back(std::thread([i](){
      Server::Listen();
    }));

    // FIXME: uws is thread safe but our init code is not.. fix this eventually
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }


  // auto u = INSTANCEACTOR;
  // json jj = {
  //   {"@context", "https://www.w3.org/ns/activitystreams"},
  //   {"id", API("relayfollows/0")},
  //   {"type", "Follow"},
  //   {"actor", u.uri},
  //   {"object", "https://www.w3.org/ns/activitystreams#Public"}
  // };
  // APClient cli(u, "relay.kitsu.life");
  // auto c = cli.Post("/inbox", jj);
  // error("{} : ({})", c->status, c->body);

  for (auto &t : clusters) {
    t.join();
  }

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
