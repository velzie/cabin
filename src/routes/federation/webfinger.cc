#include "common.h"
#include "router.h"

GET(webfinger, "/.well-known/webfinger") {
  std::string resource = req.get_param_value("resource");

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
        {"rel", "http://webfinger.net/rel/profile-page"},
        {"type", "text/html"},
        {"href", USERPAGE(ct->userid)}
      },
      {
        {"rel", "http://ostatus.org/schema/1.0/subscribe"},
        {"template", API("authorize-follow?acct={uri}")}
      }
    }},

    {"subject", req.get_param_value("resource")}
  };

  res.set_content(j.dump(), "application/jrd+json; charset=utf-8");
};
