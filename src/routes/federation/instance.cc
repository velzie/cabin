#include <stdexcept>
#define USE_DB

#include "router.h"
#include "common.h"
#include <optional>
#include "utils.h"
#include "database.h"
#include "services/ingest.h"
#include "services/user.h"

POST(inbox, "/inbox") {
  if (body["type"] == "Delete") return;
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

  auto u = UserService::lookup(uid);
  if (!u.has_value()) {
    ERROR(404, "no user");
  }

  OK(u->renderAP(), MIMEAP);
}
