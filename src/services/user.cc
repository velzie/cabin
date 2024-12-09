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
#include "services/instance.h"

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
  optional<User> lookup(const string id) {
    auto q = STATEMENT("SELECT * FROM user WHERE id = ?");
    q.bind(1, id);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    User u;
    u.load(q);
    return u;
  }

  optional<User> lookup_ap(const string id) {
    auto q = STATEMENT("SELECT * FROM user WHERE uri = ?");
    q.bind(1, id);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    User u;
    u.load(q);
    return u;
  }



  User fetchRemote(const string uri) {
    auto cached = lookup_ap(uri);
    if (cached.has_value()) {
      // TODO: don't skip if it's been a while
      return cached.value();
    }

    trace("fetching user {}", uri);
    URL url(uri);
    auto instance = InstanceService::fetchRemote(url.host);

    auto ia = UserService::lookup(cfg.instanceactor);
    APClient cli(ia.value(), url.host);

    auto response = cli.Get(url.path);
    json user = json::parse(response->body);

    if (user["type"] != "Person") {
      throw InvalidActivityError(FMT("unknown entity type {}", (string)user["type"]));
    }


    User u = {
      .uri = uri,
      .local = false,
      .host = url.host,
      
      .username = user["preferredUsername"],
      .summary = JstringOrEmpty(user, "summary"),
      .friendlyUrl = user["url"],

      .lastUpdatedAt = utils::millis(),
      .isCat = JboolOrFalse(user, "isCat"),
      .speakAsCat = JboolOrFalse(user, "speakAsCat"),

      .inbox = user["inbox"],
      .featured = user["featured"]
    };

    if (user.contains("sharedInbox") && user["sharedInbox"].is_string())
      u.sharedInbox = user["sharedInbox"];
    else
      u.sharedInbox = user["inbox"];

    if (user.contains("name") && user["name"].is_string())
      u.displayname = user["name"];
    else
      u.displayname = user["preferredUsername"];

    if (user.contains("icon") && user["icon"].is_object())
      u.avatar = user["icon"]["url"];

    if (user.contains("image") && user["image"].is_string())
      u.banner = user["image"]["url"];

    INSERT_OR_UPDATE(u, uri, id, utils::genid());
    return u;
  }
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
    .username = "test3",
    .displayname = "cabin's #3 strongest soldier",
    .summary = "mrrrrrrrrrpppp",

    .isBot = false,
    .isCat = true,
    .speakAsCat = false,
  };
  u.insert();
  
}
