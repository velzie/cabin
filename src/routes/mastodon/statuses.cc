#define USE_DB
#include <common.h>
#include <router.h>
#include "../../schema.h"
#include "../../utils.h"
#include "local.h"
#include "../../services/note.h"

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

  res.set_content(response.dump(), "application/json");
}
