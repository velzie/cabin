#include "common.h"
#include "router.h"
#include <optional>

void handle_activity(json body) {
  std::cout << body.dump() << "\n";
  std::string type = body["Type"];

  info("recieved {} activity", type);
}

POST(inbox, "/inbox") {
  handle_activity(json::parse(req.body));
}

POST(userinbox, "/users/:id/inbox") {
  std::string uid = req.path_params.at("id");
  handle_activity(json::parse(req.body));
}

GET(user, "/users/:id") {
  std::string uid = req.path_params.at("id");
  std::string userurl = USERPAGE(uid);

  std::ifstream userkeyf("user.key");
  std::ifstream userpemf("user.pem");
  std::string key((std::istreambuf_iterator<char>(userkeyf)), std::istreambuf_iterator<char>());
  std::string pem((std::istreambuf_iterator<char>(userpemf)), std::istreambuf_iterator<char>());



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
      {"publicKeyPem", pem},
    }},
    {"url", userurl},
    {"preferredUsername", uid},
    {"name", "the rizzler..."},
    {"summary", "yeah i'm doing this again"},
    {"discoverable", true},
    {"noindex", true},
    {"attachment", {}},
    {"alsoKnownAs", {}}
  };

  res.set_content(j.dump(), "application/activity+json");
}
