#define USE_DB
#include <common.h>
#include <router.h>
#include "../../schema.h"
#include "../../utils.h"
#include "local.h"

POST(post_status, "/api/v1/statuses") {
  string id = utils::genid();
  json body = json::parse(req.body);

  Note note = {
    .apid = NOTE(id),
    .localid = id,
    .content = body["status"],
    .owner = USERPAGE("gyat"),
    .published = utils::millis(),
    .local = 1,
    .sensitive = 0,
  };

  note.insert();

  json j = renderNote(note);
  res.set_content(j.dump(), "application/json");
}


