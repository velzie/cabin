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
  Note note = NoteService::create(authuser, body["status"]);

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

  Note n = NoteService::createRenote(authuser, note.value());

  OK(n.renderMS(authuser), MIMEJSON);
}

GET(status_context, "/api/v1/statuses/:id/context") {
  MSAUTH

  string id (req->getParameter("id"));
  auto originalnote = NoteService::lookup(id);
  if (!originalnote.has_value()) {
    ERROR(404, "no note");
  }

  auto q = STATEMENT("SELECT * FROM note WHERE conversation = ?");
  q.bind(1, originalnote->conversation);

  std::vector<Note> bag;
  while (q.executeStep()) {
    Note note;
    note.load(q);
    if (note.id != id)
      bag.push_back(note);
  }


  json ancestors = json::array();
  json descendants = json::array();

  Note topmost = originalnote.value();
  while (topmost.replyToUri.has_value()) {
    auto s = std::find_if(bag.begin(), bag.end(), [topmost](Note n){ return n.uri == topmost.replyToUri.value(); });
    topmost = *s;
    bag.erase(s);

    ancestors.push_back(topmost.renderMS(authuser));
  }

  for (Note n : bag) {
    descendants.push_back(n.renderMS(authuser));
  }





  json j = {
    {"ancestors", ancestors},
    {"descendants", descendants},
  };

  OK(j, MIMEJSON);
}

GET(status_favourited_by, "/api/v1/statuses/:id/favourited_by") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qFavourited = STATEMENT("SELECT owner FROM like WHERE object = ?");
  qFavourited.bind(1, n->uri);

  while (qFavourited.executeStep()) {
    User u = UserService::lookup_ap(qFavourited.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reblogged_by, "/api/v1/statuses/:id/reblogged_by") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qRenoted = STATEMENT("SELECT owner FROM note WHERE renoteUri = ?");
  qRenoted.bind(1, n->uri);

  while (qRenoted.executeStep()) {
    User u = UserService::lookup_ap(qRenoted.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reactions, "/api/v1/statuses/:id/reactions") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json reactions = json::array();
  OK(reactions, MIMEJSON);
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
