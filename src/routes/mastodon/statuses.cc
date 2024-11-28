#define USE_DB
#include <common.h>
#include <router.h>
#include "../../schema.h"
#include "../../utils.h"
#include "local.h"
#include "../../services/note.h"
#include "../../services/user.h"
#include "../../http.h"

POST(post_status, "/api/v1/statuses") {
  json body = json::parse(req.body);

  Note note = NoteService::create("gyat", body["content"]);
  
  json j = MSrenderNote(note);
  res.set_content(j.dump(), "application/json");
}

GET(status, "/api/v1/statuses/:id") {
  string id = req.path_params.at("id");

  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    res.status = 404;
    return;
  }

  json j = MSrenderNote(n.value());
  res.set_content(j.dump(), "application/json");
}

POST(status_like, "/api/v1/statuses/:id/favourite") {
  string id = req.path_params.at("id");

  auto user = UserService::lookup("gyat");
  auto note = NoteService::lookup(id);
  if (!note.has_value()) {
    res.status = 404;
    return;
  }

  string likeid = utils::genid();
  Like like = {
    .apid = LIKE(likeid),
    .localid = likeid,
    .local = true,

    .owner = user->apid,
    .object = note->apid
  };
  like.insert();

  URL url(note->apid);
  APClient cli(user.value(), url.host);

  json activity = {
    {"actor", user->apid},
    {"id", like.apid},
    {"object", note->apid},
    {"content", "❤"},
    {"_misskey_reaction", "❤"},
    {"type", "Like"}
  };

  auto resp = cli.Post("/inbox", activity);
}

GET(status_context, "/api/v1/statuses/:id/context") {
  string id = req.path_params.at("id");
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    res.status = 404;
    return;
  }


  json ancestors = json::array();
  json descendants = json::array();


  json j = {
    {"ancestors", ancestors},
    {"descendants", descendants},
  };

  res.set_content(j.dump(), "application/json");
}


GET(timelines, "/api/v1/timelines/:id") {
  auto q = STATEMENT("SELECT * FROM note");


  json response = json::array();
  while (q.executeStep()) {
    Note n;
    n.load(q);

    response.push_back(MSrenderNote(n));
  }
  std::reverse(response.begin(), response.end());

  res.set_content(response.dump(), "application/json");
}
