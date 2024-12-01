#include "QueryParser.h"
#include <string_view>
#include <vector>
#define USE_DB
#include <router.h>
#include <common.h>
#include "../../schema.h"
#include "../../entities/Note.h"
#include "../../services/follow.h"

GET(account_verify_credentials, "/api/v1/accounts/verify_credentials") {

  string uid = ct->userid;

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE id = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    ERROR(404, "no user");
  }
  u.load(q);

  json r = {
    {"username", u.username},
    {"acct", u.username},
    {"fqn", FMT("{}@{}", u.username, u.host)},
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

  OK(r, MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#get
GET(account, "/api/v1/accounts/:id") {
  string uid (req->getParameter("id"));
  dbg(uid);

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE id = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    ERROR(404, "no account");
  }
  u.load(q);

  OK(u.renderMS(), MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#get
GET(account_lookup, "/api/v1/accounts/lookup") {
  std::stringstream acct (string(req->getQuery("acct")));

  string user, host;
  std::getline(acct, user, '@');
  std::getline(acct, host);

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE username = ? AND host = ? LIMIT 1");
  q.bind(1, user);
  q.bind(2, host);

  if (!q.executeStep()) {
    ERROR(404, "no account");
  }
  u.load(q);

  OK(u.renderMS(), MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#statuses
GET(account_statuses, "/api/v1/accounts/:id/statuses") {
  MSAUTH
  string uid (req->getParameter("id"));

  User u;
  auto q = STATEMENT("SELECT * FROM user WHERE id = ? LIMIT 1");
  q.bind(1, uid);
  if (!q.executeStep()) {
    ERROR(404, "");
  }
  u.load(q);


  auto notes = STATEMENT("SELECT * FROM note WHERE owner = ?");
  notes.bind(1, u.uri);

  json response = json::array();
  while (notes.executeStep()) {
    Note n;
    n.load(notes);
    response.push_back(n.renderMS(authuser));
  }



  OK(response, MIMEJSON);
}

json getRelationship(User &you, User &them) {
  return { 
    {"id", them.id},
    {"following", false},
    {"showing_reblogs", true},
    {"notifying", false},
    {"followed_by", true},
    {"blocking", false},
    {"blocked_by", false},
    {"muting", false},
    {"muting_notifications", false},
    {"requested", false},
    {"domain_blocking", false},
    {"endorsed", false},
    {"note", ""},
  };
}

// https://docs.joinmastodon.org/methods/accounts/#follow
POST(account_follow, "/api/v1/accounts/:id/follow") {
  MSAUTH
  
  auto user = UserService::lookup((string) req->getParameter("id"));
  if (!user.has_value()) {
    ERROR(404, "no such user");
  }

  FollowService::create(authuser, user->uri);

  // rerender, not ideal but whatever
  user = UserService::lookup((string) req->getParameter("id"));
  
  OK(user->renderMS(), MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#featured_tags
// ????
GET(account_featured_tags, "/api/v1/accounts/:id/featured_tags") {
  json j = json::array();

  OK(j, MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#following
GET(account_following, "/api/v1/accounts/:id/following") {
  auto user = UserService::lookup((string) req->getParameter("id"));
  if (!user.has_value()) {
    ERROR(404, "no such user");
  }

  json response = json::array();

  auto following = STATEMENT("SELECT * FROM follow WHERE followee = ? AND pending = 0");
  following.bind(1, user->uri);

  while (following.executeStep()) {
    User u = UserService::lookup_ap(following.getColumn("follower")).value();
    response.push_back(u.renderMS());
  }

  OK(response, MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#relationships
GET(account_relationships, "/api/v1/accounts/relationships") {
  MSAUTH

  json response = json::array();

  string raw("?" + string(req->getQuery()));
  string id;
  std::vector<string> ids;
  while ((id = uWS::getDecodedQueryValue("id[]", raw)) != "") {
    auto p = raw.find("id[]=");
    if (p != std::string::npos) {
      raw.erase(p, 5+id.length()+1);
    }
    ids.push_back(id);
  }

  for (const string userid : ids) {
    auto user = UserService::lookup(userid);
    if (!user.has_value()) {
      ERROR(404, "no such user");
    }

    response.push_back(getRelationship(authuser, user.value()));
  }


  OK(response, MIMEJSON);
}

// https://docs.joinmastodon.org/methods/follow_requests/#get
GET(account_follow_requests, "/api/v1/follow_requests") {
  json j = json::array();

  OK(j, MIMEJSON);
}
