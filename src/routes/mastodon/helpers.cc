#define USE_DB
#include <router.h>
#include <common.h>
#include "../../schema.h"
#include "../../utils.h"
#include "../../services/note.h"



json MSrenderUser(User &user) {

  string acct;
  URL id(user.uri);
  if (id.host == ct->cfg.domain) {
    acct = user.username;
  } else {
    acct = FMT("{}@{}", user.username, id.host);
  }

   return {
    {"id", user.id},
    {"username", user.username},
    {"acct", acct},
    {"display_name", user.displayname},
    {"locked", false},
    {"bot", false},
    {"created_at","2016-03-16T14:34:26.392Z"},
    {"note", user.summary},
    {"url", USERPAGE(user.id)},
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
  auto s = STATEMENT("SELECT * FROM user WHERE uri = ?");
  s.bind(1, note.owner);
  s.executeStep();
  u.load(s);


  Note replytouser;
  if (note.replyToUri.has_value()) {
    auto rs = STATEMENT("SELECT * FROM note WHERE uri = ?");
    rs.bind(1, note.replyToUri.value());
    rs.executeStep();
    replytouser.load(rs);
  }

  auto favs = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ?");
  favs.bind(1, note.uri);
  favs.executeStep();
  int fav_count = favs.getColumn(0);

  json j = {
    {"id", note.id},
    {"created_at", utils::millisToIso(note.published)},
    {"in_reply_to_id", note.replyToUri},
    {"in_reply_to_account_id", nullptr},
    {"sensitive", note.sensitive},
    {"spoiler_text", ""},
    {"visibility", "public"},
    {"language", "en"},
    {"uri", NOTE(note.id)},
    {"url", NOTE(note.id)},
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

  if (note.replyToUri.has_value())
    j["in_reply_to_account_id"] = replytouser.id;

  if (note.renoteUri.has_value()) {
    auto n = NoteService::lookup_ap(note.renoteUri.value());
    j["reblog"] = MSrenderNote(n.value());
  }

  return j;
}


