#include <stdexcept>
#define USE_DB

#include "common.h"
#include "router.h"
#include <optional>
#include "../../utils.h"
#include "../../schema.h"
#include "../../services/user.h"
#include "../../services/note.h"

void handle_activity(json body) {
  std::cout << body.dump() << "\n";
  std::string type = body["type"];

  info("recieved {} activity", type);

  if (type == "Create") {
    auto object = body["object"];
    if (object["type"] != "Note") return;


    UserService::fetchRemote(object["attributedTo"]);
    Note n = {
      .apid = object["id"],
      .localid = utils::genid(),
      .local = false,

      .content = object["content"],
      .owner = object["attributedTo"],
      .published = utils::isoToMillis(object["published"]),

      .sensitive = object["sensitive"]
    };

    if (!n.insert()) {
      throw std::runtime_error("");
    }
  } else if (type == "Announce") {
    string object = body["object"];
    Note note = NoteService::fetchRemote(object);
  }
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


  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE localid = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    res.status = 404;
    return;
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

  res.set_content(j.dump(), "application/activity+json");
}
