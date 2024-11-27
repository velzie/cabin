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
#include <backward.hpp>
#include <thread>
#include "common.h"
#include "http.h"

Config default_config = {
  .domain = "your.domain",
  .host = "0.0.0.0",
  .port = 2001,
};


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
backward::SignalHandling sh;

void registeruser();
int main() {
  // spdlog::set_pattern("[%M:%S] [%^%L%$] [%&] %v");
  ct = std::make_shared<Cabin>("config.json");

  std::ifstream contextst("context.json");
  ct->context = json::parse(std::string((std::istreambuf_iterator<char>(contextst)), std::istreambuf_iterator<char>()));

  ct->InitDB();
  registeruser();

  std::thread tserver([](){
      ct->server.Start();
  });


  // json j = {
  //   {"id", API("follows/0")},
  //   {"type", "Follow"},
  //   {"actor", USERPAGE(ct->userid)},
  //   {"object", "https://booping.synth.download/users/a005c9wl4pwj0arp"}
  // };

  // APClient cli("booping.synth.download");
  // auto c = cli.Post("/inbox", j);
  // trace("{} : ({})", c->status, c->body);

  tserver.join();

  return 0;
}
