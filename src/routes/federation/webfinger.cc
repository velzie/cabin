#include "router.h"
#include "common.h"

GET(webfinger, "/.well-known/webfinger") {
  std::string resource(req->getQuery("resource"));

  if (resource.rfind("acct:", 0) == 0) {
    resource.erase(0, 5);
  }

  string usersbase = USERPAGE("");
  if (resource.rfind(usersbase, 0) == 0) {
    resource.erase(0, usersbase.length());
  }

  json j = {
    {"links", {
      {
        {"rel", "self"},
        {"type", "application/activity+json"},
        {"href", USERPAGE(ct->userid)}
      },
      {
        {"rel", "self"},
        {"type", "application/ld+json; profile=\"https://www.w3.org/ns/activitystreams\""},
        {"href", USERPAGE(ct->userid)}
      },
      {
        {"rel", "http://webfinger.net/rel/profile-page"},
        {"type", "text/html"},
        {"href", USERPAGE(ct->userid)}
      },
      {
        {"rel", "http://ostatus.org/schema/1.0/subscribe"},
        {"template", API("authorize-follow?acct={uri}")}
      }
    }},
    {"subject", "acct:gyat@rizz.velzie.rip"}
  };

  OK(j, MIMEJRD);
};
