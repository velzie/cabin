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
#include <dbg.h>


bool generateRSAKeyPair(std::string& privateKeyBuffer, std::string& publicKeyBuffer, int bits = 2048) {
  bool success = false;
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
  EVP_PKEY* pkey = nullptr;
  BIO* privateBio = nullptr;
  BIO* publicBio = nullptr;

  char* publicKeyData = nullptr;
  char* privateKeyData = nullptr;
  long privateKeyLength = 0, publicKeyLength = 0;

  if (!ctx) goto err;
  if (EVP_PKEY_keygen_init(ctx) <= 0) goto err;
  if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) goto err;

  if (EVP_PKEY_keygen(ctx, &pkey) <= 0) goto err;
  privateBio = BIO_new(BIO_s_mem());
  publicBio = BIO_new(BIO_s_mem());
  if (!privateBio || !publicBio) goto err;

  if (!PEM_write_bio_PrivateKey(privateBio, pkey, nullptr, nullptr, 0, nullptr, nullptr)) goto err;
  if (!PEM_write_bio_PUBKEY(publicBio, pkey)) goto err;

  privateKeyLength = BIO_get_mem_data(privateBio, &privateKeyData);
  if (privateKeyLength > 0) {
      privateKeyBuffer.assign(privateKeyData, privateKeyLength);
  }

  publicKeyLength = BIO_get_mem_data(publicBio, &publicKeyData);
  if (publicKeyLength > 0) {
      publicKeyBuffer.assign(publicKeyData, publicKeyLength);
  }
  success = !privateKeyBuffer.empty() && !publicKeyBuffer.empty();

  goto cleanup;

err:
    ERR_print_errors_fp(stderr);

cleanup:
  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(ctx);
  BIO_free_all(privateBio);
  BIO_free_all(publicBio);
  return success;
}

void writeKey() {
  std::string key;
  std::string pem;
  generateRSAKeyPair(key, pem);

  std::ofstream keyf("user.key");
  std::ofstream pemf("user.pem");
  keyf << key;
  pemf << pem;
}

Config default_config = {
  .domain = "your.domain",
  .host = "0.0.0.0",
  .port = 2001,
};

Cabin::~Cabin() {
  delete db;
}
Cabin::Cabin(std::string config_path) {
  db = new SQLite::Database(SOFTWARE".db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

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

int main() {
  // spdlog::set_pattern("[%M:%S] [%^%L%$] [%&] %v");
  ct = std::make_shared<Cabin>("config.json");

  std::ifstream contextst("context.json");
  
  ct->context = json::parse(std::string((std::istreambuf_iterator<char>(contextst)), std::istreambuf_iterator<char>()));

  std::thread tserver([](){
      ct->server.Start();
  });


  json j = {
    {"id", API("follows/0")},
    {"type", "Follow"},
    {"actor", USERPAGE(ct->userid)},
    {"object", "https://booping.synth.download/users/a005c9wl4pwj0arp"}
  };

  // APClient cli("booping.synth.download");
  // auto c = cli.Post("/inbox", j);
  // trace("{} : ({})", c->status, c->body);

  tserver.join();

  return 0;
}
