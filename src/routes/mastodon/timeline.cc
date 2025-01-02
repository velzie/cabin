#include "common.h"
#include "mshelper.h"
#include "router.h"
#include "querybuilder.h"


GET(suggestions, "/api/v1/suggestions") {
  MSAUTH
  OK(json::array(), MIMEJSON);
}

GET(timeline_federated, "/api/v1/timelines/public") {
  MSAUTH

  QueryBuilder query;
  query = query
    .select({"*"})
    .from("note")
    .orderBy("publishedClamped", "DESC");

  if (req->getQuery("local") == "true") {
    query = query.where(EQ("host", cfg.domain));
  }

  if (req->getQuery("only_media") == "true") {
    query = query.where(GT("json_array_length(attachments)", 0));
  }

  PAGINATE(query, Note, publishedClamped);
}

GET(timeline_bubble, "/api/v1/timelines/bubble") {
  MSAUTH

  QueryBuilder query;
  query = query
    .select({"*"})
    .from("note")
    .orderBy("publishedClamped", "DESC");

  
  vector<string> bubbledHosts = cfg.bubbledHosts;
  QueryConstraint bubble = EQ("host", bubbledHosts.back());
  bubbledHosts.pop_back();
  for (const string host : bubbledHosts) {
    bubble = OR(bubble, EQ("host", host));
  }
  query = query.where(bubble);

  PAGINATE(query, Note, publishedClamped);
}
