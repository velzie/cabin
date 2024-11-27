#define USE_DB
#include <common.h>
#include <router.h>
#include "../../schema.h"
#include "local.h"

GET(account_verify_credentials, "/api/v1/accounts/verify_credentials") {

  string uid = "gyat";

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE localid = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    res.status = 404;
    return;
  }
  u.load(q);

  json r = {
    {"username", u.username},
    {"acct", u.username},
    {"fqn", FMT("{}@{}", u.username, ct->cfg.domain)},
    {"display_name", u.displayname},
    {"locked", false},
    {"created_at", "2024-06-27T03:50:13.833Z"},
    {"followers_count", 0},
    {"following_count", 0},
    {"statuses_count", 0},
    {"note", u.summary},
    {"url", USERPAGE(uid)},
    {"uri", USERPAGE(uid)},
    {"avatar", "https://fedi.velzie.rip/files/28a63a3f-c3e7-4dba-b556-02dad8470212.png"},
    {"avatar_static", "https://fedi.velzie.rip/files/28a63a3f-c3e7-4dba-b556-02dad8470212.png"},
    {"header", "https://fedi.velzie.rip/files/409ad084-6089-410a-83d6-805cc4f8df52.jpg"},
    {"header_static", "https://fedi.velzie.rip/files/409ad084-6089-410a-83d6-805cc4f8df52.jpg"},
    {"moved", nullptr},
    {"bot", false},
    {"discoverable", false},
    {"fields", json::array()},
    {"source", {
      {"language", ""},
      {"note", u.summary},
      {"privacy", "public"},
      {"sensitive", false},
      {"fields", json::array()},
      {"follow_requests_count", 0}
    }},
    {"emojis", json::array()},
    {"id", uid},

  };

  res.set_content(r.dump(), "application/json");
}

// https://docs.joinmastodon.org/methods/accounts/#get
GET(account, "/api/v1/accounts/:id") {
  string uid = req.path_params.at("id");
  dbg(uid);

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE localid = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    res.status = 404;
    return;
  }
  u.load(q);

  auto j = renderUser(u);

  res.set_content(j.dump(), "application/json");
}

// https://docs.joinmastodon.org/methods/accounts/#statuses
GET(account_statuses, "/api/v1/accounts/:id/statuses") {
  string uid = req.path_params.at("id");

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE localid = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    res.status = 404;
    return;
  }
  u.load(q);


  auto notes = STATEMENT("SELECT * FROM note WHERE owner = ?");
  notes.bind(1, u.apid);

  json response = json::array();
  while (notes.executeStep()) {
    Note n;
    n.load(notes);
    response.push_back(renderNote(n));
  }



  res.set_content(response.dump(), "application/json");
}

// https://docs.joinmastodon.org/methods/accounts/#featured_tags
// ????
GET(account_featured_tags, "/api/v1/accounts/:id/featured_tags") {
  json j = json::array();

  res.set_content(j.dump(), "application/json");
}

// https://docs.joinmastodon.org/methods/accounts/#relationships
// stub obviously
GET(account_relationships, "/api/v1/accounts/relationships") {
  json j = json::array();

  res.set_content(j.dump(), "application/json");
}
