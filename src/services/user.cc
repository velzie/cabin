#include "SQLiteCpp/Statement.h"
#define USE_DB
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <httplib.h>

#include <common.h>

#include "../http.h"
#include "../schema.h"
#include "../utils.h"

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



void update_remote_user(string apid) {
  URL url(apid);
  APClient cli(url.host);

  auto response = cli.Get(url.path);
  assert(response->status == 200);

  json user = json::parse(response->body);


  User u = {
    .apid = apid,
    .local = false,
    
    .username = user["preferredUsername"],
    .displayname = user["name"],
    .summary = user["summary"]
  };

  auto query = STATEMENT("SELECT localid FROM user where apid = ?");
  query.bind(1, apid);
  if (query.executeStep()) {
    // user exists, update
    string localid = query.getColumn("localid");
    
    // TODO orm stuff etc
    auto delq = STATEMENT("DELETE FROM user WHERE apid = ?");
    delq.bind(1, apid);
    delq.exec();

    u.localid = localid;
    u.insert();
  } else {
    u.localid = utils::genid();
    u.insert();
  }
}


void registeruser() {
  string privkey;
  string pubkey;
  generateRSAKeyPair(privkey, pubkey);

  // update_remote_user("https://booping.synth.download/users/a005c9wl4pwj0arp");


  // User u = {
  //   .apid = USERPAGE("gyat"),
  //   .localid = "gyat",
  //   .local = 1,
  //   .publicKey = pubkey,
  //   .privateKey = privkey,
  //   .username = "gyattrous",
  //   .displayname = "The Rizzler...",
  //   .summary = "gyat user",
  // };
  // u.insert();
  
}

