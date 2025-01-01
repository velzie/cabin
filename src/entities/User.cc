#include "common.h"
#include "utils.h"
#include "entities/User.h"
#include "querybuilder.h"

User User::ingest(const json user) {
  URL url(user["id"]);

  User u;
  u.uri = user["id"];
  u.local = false;
  u.host = url.host;
    
  u.username = user["preferredUsername"];
  u.summary = JstringOrEmpty(user, "summary");
  u.friendlyUrl = user["url"];

  u.lastUpdatedAt = utils::millis();
  u.isCat = JboolOrFalse(user, "isCat");
  u.speakAsCat = JboolOrFalse(user, "speakAsCat");

  u.inbox = user["inbox"];
  u.featured = JstringOrEmpty(user, "featured");

  if (user.contains("sharedInbox") && user["sharedInbox"].is_string())
    u.sharedInbox = user["sharedInbox"];
  else
    u.sharedInbox = user["inbox"];

  if (user.contains("name") && user["name"].is_string())
    u.displayname = user["name"];
  else
    u.displayname = user["preferredUsername"];

  if (user.contains("icon") && user["icon"].is_object())
    u.avatar = user["icon"]["url"];

  if (user.contains("image") && user["image"].is_object())
    u.banner = user["image"]["url"];

  INSERT_OR_UPDATE(u, uri, id, utils::genid());
  return u;
}

json User::renderMS() {

  QueryBuilder qb;

  auto sFollowers = qb
    .select({"COUNT(*)"})
    .from("follow")
    .where(EQ("followee", uri))
    .build();
  sFollowers.executeStep();
  int followers = sFollowers.getColumn(0);

  auto sFollowing = qb
    .select({"COUNT(*)"})
    .from("follow")
    .where(EQ("follower", uri))
    .build();
  sFollowing.executeStep();
  int following = sFollowing.getColumn(0);

  auto sNotes = qb
    .select({"COUNT(*)"})
    .from("note")
    .where(EQ("owner", uri))
    .build();
  sNotes.executeStep();
  int notes = sNotes.getColumn(0);

   return {
    {"id", id},
    {"username", username},
    {"acct", acct(true)},
    {"display_name", displayname},
    {"locked", false},
    {"bot", false},
    {"created_at", "2016-03-16T14:34:26.392Z"},
    {"note", summary},
    {"url", USERPAGE(id)},
    {"avatar", avatar},
    {"avatar_static", avatar},
    {"header", banner},
    {"header_static", banner},
    {"followers_count", followers},
    {"following_count", following},
    {"statuses_count", notes},
    {"last_status_at", "2019-12-05T03:03:02.595Z"},
    {"emojis", json::array()},
    {"fields", json::array()},
    {"pleroma",{
      {"background_image", nullptr},
      {"skip_thread_containment", false},
      {"is_moderator", false},
      {"is_admin", true},
      {"ap_id", uri},
      {"tags", ARR},
      {"also_known_as", {}},
      {"hide_follows", true},
      {"hide_follows_count", false},
      {"hide_followers", true},
      {"hide_followers_count", false},
      {"is_confirmed", true},
      {"is_suggested", false},
      {"hide_favorites", true},
      {"favicon", FMT("https://{}/favicon.png", host)},
      {"relationship", json::object()},
    }}
  };
}

