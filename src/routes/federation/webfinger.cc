#define USE_DB
#include "router.h"
#include "common.h"
#include "database.h"
#include "services/user.h"

GET(webfinger, "/.well-known/webfinger") {
  std::string resource(req->getQuery("resource"));


  User u;

  if (resource.rfind("acct:", 0) == 0) {
    resource.erase(0, 5);
    std::stringstream acct(resource);
    string user, host;
    std::getline(acct, user, '@');
    std::getline(acct, host);

    auto q = STATEMENT("SELECT * FROM user WHERE username = ? AND host = ? LIMIT 1");
    q.bind(1, user);
    q.bind(2, host);

    if (!q.executeStep()) {
      ERROR(404, "no such user");
    }

    u.load(q);
  }

  string usersbase = USERPAGE("");
  if (resource.rfind(usersbase, 0) == 0) {
    auto lookup = UserService::lookup_ap(usersbase);
    if (lookup.has_value()) {
      u = lookup.value();
    } else {
      ERROR(404, "no such user");
    }
  }

  json j = {
    {"links", {
      {
        {"rel", "self"},
        {"type", "application/activity+json"},
        {"href", USERPAGE(u.id)}
      },
      {
        {"rel", "self"},
        {"type", "application/ld+json; profile=\"https://www.w3.org/ns/activitystreams\""},
        {"href", USERPAGE(u.id)}
      },
      {
        {"rel", "http://webfinger.net/rel/profile-page"},
        {"type", "text/html"},
        {"href", USERPAGE(u.id)}
      },
      {
        {"rel", "http://ostatus.org/schema/1.0/subscribe"},
        {"template", API("authorize-follow?acct={uri}")}
      }
    }},
    {"subject", FMT("acct:{}@{}", u.username, u.host)}
  };

  OK(j, MIMEJRD);
};
