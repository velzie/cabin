#define USE_DB
#include <router.h>
#include <common.h>
#include "../../schema.h"
#include "../../utils.h"



json MSrenderUser(User &user) {

  string acct;
  URL id(user.apid);
  if (id.host == ct->cfg.domain) {
    acct = user.username;
  } else {
    acct = FMT("{}@{}", user.username, id.host);
  }

   return {
    {"id", user.localid},
    {"username", user.username},
    {"acct", acct},
    {"display_name", user.displayname},
    {"locked", false},
    {"bot", false},
    {"created_at","2016-03-16T14:34:26.392Z"},
    {"note", user.summary},
    {"url", USERPAGE(user.localid)},
    {"avatar", "https://files.mastodon.social/accounts/avatars/000/000/001/original/d96d39a0abb45b92.jpg"},
    {"avatar_static", "https://files.mastodon.social/accounts/avatars/000/000/001/original/d96d39a0abb45b92.jpg"},
    {"header", "https://files.mastodon.social/accounts/headers/000/000/001/original/c91b871f294ea63e.png"},
    {"header_static", "https://files.mastodon.social/accounts/headers/000/000/001/original/c91b871f294ea63e.png"},
    {"followers_count", 320472},
    {"following_count", 453},
    {"statuses_count", 61163},
    {"last_status_at", "2019-12-05T03:03:02.595Z"},
    {"emojis", json::array()},
    {"fields", json::array()},
  };
}

json MSrenderNote(Note &note) {

  User u;
  auto s = STATEMENT("SELECT * FROM user WHERE apid = ?");
  s.bind(1, note.owner);
  s.executeStep();
  u.load(s);

  auto favs = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ?");
  favs.bind(1, note.apid);
  favs.executeStep();
  int fav_count = favs.getColumn(0);

  json j = {
    {"id", note.localid},
    {"created_at", utils::millisToIso(note.published)},
    {"in_reply_to_id", nullptr},
    {"in_reply_to_account_id", nullptr},
    {"sensitive", false},
    {"spoiler_text", ""},
    {"visibility", "public"},
    {"language", "en"},
    {"uri", NOTE(note.localid)},
    {"url", NOTE(note.localid)},
    {"replies_count", 7},
    {"reblogs_count", 98},
    {"favourites_count", fav_count},
    {"favourited", false},
    {"reblogged", false},
    {"muted", false},
    {"bookmarked", false},
    {"content", note.content},
    {"reblog", nullptr},
    {"application", nullptr},
    {"account", MSrenderUser(u)},
    {"media_attachments", json::array()},
    {"mentions", json::array()},
    {"tags", json::array()},
    {"emojis", json::array()},
    {"reactions", json::array()},
    {"card", nullptr},
    {"poll", nullptr},
  };

  return j;
}


