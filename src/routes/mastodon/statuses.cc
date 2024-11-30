#define USE_DB
#include <router.h>
#include <common.h>
#include "../../schema.h"
#include "../../utils.h"
#include "local.h"
#include "../../services/note.h"
#include "../../services/user.h"
#include "../../http.h"
#include "../../entities/Like.h"

POST(post_status, "/api/v1/statuses") {
  MSAUTH
  Note note = NoteService::create(ct->userid, body["status"]);

  OK(note.renderMS(authuser), MIMEJSON);
}

GET(status, "/api/v1/statuses/:id") {
  MSAUTH
  string id (req->getParameter("id"));

  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json j = n.value().renderMS(authuser);
  OK(j, MIMEJSON);
}

POST(status_like, "/api/v1/statuses/:id/favourite") {
  MSAUTH
  string id (req->getParameter("id"));

  auto user = UserService::lookup(ct->userid);
  auto note = NoteService::lookup(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  string likeid = utils::genid();
  Like like = {
    .uri = LIKE(likeid),
    .id = likeid,
    .local = true,

    .owner = user->uri,
    .object = note->uri
  };
  like.insert();

  URL url(note->uri);
  APClient cli(user.value(), url.host);

  json activity = {
    {"actor", user->uri},
    {"id", like.uri},
    {"object", note->uri},
    {"content", "❤"},
    {"_misskey_reaction", "❤"},
    {"type", "Like"}
  };

  auto resp = cli.Post("/inbox", activity);

  note = NoteService::lookup(id);
  OK(note->renderMS(authuser), MIMEJSON);
}

POST(status_renote, "/api/v1/statuses/:id/reblog") {
  MSAUTH

  string id(req->getParameter("id"));
  auto note = NoteService::lookup(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  Note n = NoteService::createRenote(ct->userid, note->uri);

  OK(n.renderMS(authuser), MIMEJSON);
}

GET(status_context, "/api/v1/statuses/:id/context") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }


  json ancestors = json::array();
  json descendants = json::array();


  json j = {
    {"ancestors", ancestors},
    {"descendants", descendants},
  };

  OK(j, MIMEJSON);
}


GET(timelines, "/api/v1/timelines/:id") {
  MSAUTH

  auto q = STATEMENT("SELECT * FROM note");


  json response = json::array();
  while (q.executeStep()) {
    Note n;
    n.load(q);

    response.push_back(n.renderMS(authuser));
  }
  std::reverse(response.begin(), response.end());

  OK(response, MIMEJSON);
}
