#include "QueryParser.h"
#include <string_view>
#include <vector>
#include <router.h>
#include <common.h>
#include "SQLiteCpp/Statement.h"
#include "database.h"
#include "entities/Note.h"
#include "services/FollowService.h"
#include "services/BiteService.h"
#include "querybuilder.h"
#include "mshelper.h"

GET(account_verify_credentials, "/api/v1/accounts/verify_credentials") {
  MSAUTH


  json r = {
    {"username", authuser.username},
    {"acct", authuser.username},
    {"fqn", authuser.acct(false)},
    {"display_name", authuser.displayname},
    {"locked", false},
    {"created_at", "2024-06-27T03:50:13.833Z"},
    {"followers_count", 0},
    {"following_count", 0},
    {"statuses_count", 0},
    {"note", authuser.summary},
    {"url", USERPAGE(authuser.id)},
    {"uri", USERPAGE(authuser.id)},
    {"avatar", authuser.avatar},
    {"avatar_static", authuser.avatar},
    {"header", "https://fedi.velzie.rip/files/409ad084-6089-410a-83d6-805cc4f8df52.jpg"},
    {"header_static", "https://fedi.velzie.rip/files/409ad084-6089-410a-83d6-805cc4f8df52.jpg"},
    {"moved", nullptr},
    {"bot", false},
    {"discoverable", false},
    {"fields", json::array()},
    {"source", {
      {"language", ""},
      {"note", authuser.summary},
      {"privacy", "public"},
      {"sensitive", false},
      {"fields", json::array()},
      {"follow_requests_count", 0}
    }},
    {"emojis", json::array()},
    {"id", authuser.id},

  };

  OK(r, MIMEJSON);
}

// https://docs.joinmastodon.org/methods/accounts/#get
GET(account, "/api/v1/accounts/:id") {
  string uid (req->getParameter("id"));

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

  QueryBuilder qb;
  auto q = qb.select().where(
    AND(
      EQ("username", user),
      EQ("host", host)
    )
  ).build();


  User u;
  // auto q = STATEMENT("SELECT * FROM user WHERE username = ? AND host = ? LIMIT 1");
  // q.bind(1, user);
  // q.bind(2, host);

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
  auto u = User::lookupid(uid);

  if (!u.has_value()) ERROR(404, "no such user??");

  // TODO: once pinned lists are implemented
  if (!req->getQuery("pinned").empty()) OK(ARR, MIMEJSON);

  QueryBuilder query;
  query = query.select({"*"}).from("note").where(EQ("owner", u->uri)).orderBy("publishedClamped", "DESC");

  PAGINATE(query, Note, publishedClamped);
}

// https://docs.joinmastodon.org/methods/accounts/#search
GET(accounts_search, "/api/v1/accounts/search") {
  MSAUTH
  std::stringstream ss(string(req->getQuery("limit")));
  int limit = 0;
  ss >> limit;
  if (!limit) limit = 40;
  if (limit > 80) limit = 80;

  string query (req->getQuery("q"));

  QueryBuilder q;

  SQLite::Statement s = q.select({"*"})
    .from("user")
    // FIXME: sql injection
    .where(GT(FMT("instr(username, \"{}\")", query), 0))
    .limit(limit)
    .build();

  json response = json::array();
  while (s.executeStep()) {
    User u;
    u.load(s);
    response.push_back(u.renderMS());
  }
  OK(response, MIMEJSON);
}

json getRelationship(User &you, User &them) {
  auto query = STATEMENT("SELECT 1 FROM follow WHERE followee = ? AND follower = ?");

  query.bind(1, them.uri);
  query.bind(2, you.uri);
  bool following = query.executeStep();

  query.reset();

  query.bind(1, you.uri);
  query.bind(2, them.uri);
  bool followed_by = query.executeStep();

  return { 
    {"id", them.id},
    {"following", following},
    {"showing_reblogs", true},
    {"notifying", false},
    {"followed_by", followed_by},
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
  
  auto user = User::lookupid((string) req->getParameter("id"));
  if (!user.has_value()) {
    ERROR(404, "no such user");
  }

  FollowService::create(authuser, user->uri);

  // rerender, not ideal but whatever
  user = User::lookupid((string) req->getParameter("id"));
  
  OK(user->renderMS(), MIMEJSON);
}

POST(account_bite, "/api/v1/users/:id/bite") {
  MSAUTH
  
  auto user = User::lookupid((string) req->getParameter("id"));
  if (!user.has_value()) {
    ERROR(404, "no such user");
  }

  BiteService::create(authuser, user.value());

  user = User::lookupid((string) req->getParameter("id"));
  
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
  auto user = User::lookupid((string) req->getParameter("id"));
  if (!user.has_value()) {
    ERROR(404, "no such user");
  }

  json response = json::array();

  auto following = STATEMENT("SELECT * FROM follow WHERE followee = ? AND pending = 0");
  following.bind(1, user->uri);

  while (following.executeStep()) {
    User u = User::lookupuri(following.getColumn("follower")).value();
    response.push_back(u.renderMS());
  }

  OK(response, MIMEJSON);
}

// this is a pleroma bug everyone say thank you pleroma
GET(account_relationships_pleroma, "/api/v1/accounts/relationships/") {
  REDIRECT("/api/v1/accounts/relationships?" + (string)req->getQuery());
}

// https://docs.joinmastodon.org/methods/accounts/#relationships
GET(account_relationships, "/api/v1/accounts/relationships") {
  MSAUTH

  json response = json::array();

  string raw("?" + string(req->getQuery()));
  string id;
  std::vector<string> ids;

  if (!req->getQuery("id").empty()) {
    // love you pleroma <3
    ids.push_back((string)req->getQuery("id"));
  } else {
    while ((id = uWS::getDecodedQueryValue("id[]", raw)) != "") {
      auto p = raw.find("id[]=");
      if (p != std::string::npos) {
        raw.erase(p, 5+id.length()+1);
      }
      ids.push_back(id);
    }
  }

  for (const string userid : ids) {
    auto user = User::lookupid(userid);
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
