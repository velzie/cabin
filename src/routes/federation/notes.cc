#include "SQLiteCpp/Statement.h"
#include <type_traits>
#define USE_DB
#include "router.h"
#include "common.h"

#include "../../utils.h"
GET(notes, "/notes/:id") {
  std::string id = req.path_params.at("id");
  std::string idurl = API("notes/"+id);

  auto q = STATEMENT("SELECT content FROM note WHERE localid = ? LIMIT 1");
  q.bind(1, id);

  if (!q.executeStep()) {
    res.status = 404;
    return;
  }

  std::string content = q.getColumn("content");

  
  json j = {
    {"@context", ct->context},
    {"id", idurl},
    {"type", "Note"},
    {"summary", nullptr},
    {"inReplyTo", nullptr},
    {"published", utils::dateISO()},
    {"url", idurl},
    {"attributedTo", USERPAGE(ct->userid)},
    {"to", {"https://www.w3.org/ns/activitystreams#Public"}},
    {"cc", {USERPAGE(ct->userid)+"/followers"}},
    {"sensitive", false},
    {"content", content},
    {"attachment", {}},
    {"tag", {}}
  };

  res.set_content(j.dump(), "application/activity+json");
}
