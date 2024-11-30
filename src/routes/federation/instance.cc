#include <stdexcept>
#define USE_DB

#include "router.h"
#include "common.h"
#include <optional>
#include "../../utils.h"
#include "../../schema.h"
#include "../../services/ingest.h"

POST(inbox, "/inbox") {
  trace("queuing ingest of {}", (string)body["type"]);
  IngestService::QueueIngest(body);
}

POST(userinbox, "/users/:id/inbox") {
  std::string uid(req->getParameter("id"));
  trace("queuing ingest of {} (pointed to {})", (string)body["type"], uid);

  IngestService::QueueIngest(body);
}

GET(user, "/users/:id") {
  std::string uid(req->getParameter("id"));
  std::string userurl = USERPAGE(uid);


  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE id = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    ERROR(404, "no user");
  }
  u.load(q);

  json j = {
    {"@context", ct->context},
    {"type", "Person"},
    {"id", userurl},
    {"inbox", userurl + "/inbox"},
    {"outbox", userurl + "/outbox"},
    {"followers", userurl + "/followers"},
    {"following", userurl + "/following"},
    {"featured", userurl + "/featured"},
    {"sharedinbox", API("inbox")},
    {"endpoints", { 
      {"sharedInbox", API("inbox")} 
    } },
    {"publicKey", {
      {"id", userurl},
      {"owner", userurl},
      {"publicKeyPem", u.publicKey},
    }},
    {"url", userurl},
    {"preferredUsername", u.username},
    {"name", u.displayname},
    {"summary", u.summary},
    {"discoverable", true},
    {"noindex", true},
    {"attachment", json::array()},
    {"alsoKnownAs", json::array()}
  };

  OK(j, MIMEAP);
}
