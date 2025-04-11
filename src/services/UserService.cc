#include "entities/UserSettings.h"
#define USE_DB
#include "SQLiteCpp/Statement.h"
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <httplib.h>

#include <common.h>

#include "error.h"
#include "database.h"
#include "utils.h"
#include "http.h"

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

namespace UserService {

}



void registeruser() {
  string privkey;
  string pubkey;
  generateRSAKeyPair(privkey, pubkey);

  // APClient cli("booping.synth.download");
  // auto r = cli.Get("/users/a005c9wl4pwj0arp");
  // dbg(r->body);

  // update_remote_user("https://booping.synth.download/users/a005c9wl4pwj0arp");


  User u = {
    .uri = USERPAGE(cfg.instanceactor),
    .id = cfg.instanceactor,
    .local = 1,
    .host = cfg.domain,

    .publicKey = pubkey,
    .privateKey = privkey,
    .username = "test",
    .displayname = "cabin's strongest soldier",
    .summary = "mrrrrrrrrrpppp",

    .isBot = false,
    .isCat = true,
    .speakAsCat = false,
  };
  auto usr = u.insert();

  UserSettings us = {
      .userId = cfg.instanceactor,
      .email = "test@test.com",
      .password = "test"
  };
  us.insert();

}
