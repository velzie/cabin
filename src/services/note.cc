#define USE_DB
#include <common.h>
#include "note.h"
#include "delivery.h"

#include <optional>
#include <stdexcept>
#include "SQLiteCpp/Statement.h"
#include <httplib.h>
#include "../schema.h"
#include "../utils.h"
#include "user.h"
#include "../http.h"

namespace NoteService {

  Note _create(User &owner) {
    string id = utils::genid();
    Note note = {
      .uri = NOTE(id),
      .id = id,
      .local = 1,
      .host = ct->cfg.domain,

      .owner = USERPAGE(owner.id),
      .published = utils::millis(),
    };
    return note;
  }

  Note create(User &owner, string content) {
    Note note = _create(owner);
    note.content = content;
    note.cw = "";
    note.sensitive = false;

    note.insert();


    json activity = {
      {"type", "Create"},
      {"actor", note.owner},
      {"id", NOTE(note.id)+"/activity"},
      {"object", note.renderAP()},
    };

    DeliveryService::Audience au = {
      .actor = owner,
      .aspublic = true,
      .followers = true,
    };
    DeliveryService::QueueDelivery(activity, au);

    return note;
  }

  Note createRenote(User &owner, string renoteUri) {
    Note note = _create(owner);
    note.renoteUri = renoteUri;
    note.insert();

    return note;
  }

  Note ingest(const string uri, const json note) {
    URL url(uri);

    Note n = {
      .uri = uri,
      .local = false,
      .host = url.host,

      .replyToUri = nullopt,
      .content = note["content"],
      .sensitive = false,
      .owner = note["attributedTo"],
      .published = utils::isoToMillis(note["published"]),


      .lastUpdatedAt = utils::millis()
    };

    if (note.contains("sensitive") && note["sensitive"].is_boolean()) {
      n.sensitive = note["sensitive"];
    }
    if (note.contains("inReplyTo") && !note["inReplyTo"].is_null()) {
      n.replyToUri = std::make_optional(note["inReplyTo"]);
    }

    UserService::fetchRemote(note["attributedTo"]);

    if (n.replyToUri.has_value())
      fetchRemote(n.replyToUri.value());

    auto query = STATEMENT("SELECT id FROM note where uri = ?");
    query.bind(1, uri);
    if (query.executeStep()) {
      // note exists, update
      string id = query.getColumn("id");
      
      // TODO orm stuff etc
      auto delq = STATEMENT("DELETE FROM note WHERE uri = ?");
      delq.bind(1, uri);
      delq.exec();

      n.id = id;
      n.insert();
    } else {
      n.id = utils::genid();
      n.insert();
    }

    return n;
  }

  Note fetchRemote(const string uri) {
    trace("fetching note {}", uri);
    auto u = UserService::lookup(ct->userid);
    URL url(uri);
    APClient cli(u.value(), url.host);

    auto response = cli.Get(url.path);
    if (response->status != 200) {
      dbg(response->body);
      throw std::runtime_error("");
    }

    json note = json::parse(response->body);

    Note n = ingest(uri, note);
    return n;
  }

  optional<Note> lookup(const string id) {
    auto q = STATEMENT("SELECT * FROM note WHERE id = ?");
    q.bind(1, id);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    Note n;
    n.load(q);
    return n;
  }

  optional<Note> lookup_ap(const string uri) {
    auto q = STATEMENT("SELECT * FROM note WHERE uri = ?");
    q.bind(1, uri);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    Note n;
    n.load(q);
    return n;
  }
}


json Note::renderMS(User &requester) {
  User uOwner;
  auto s = STATEMENT("SELECT * FROM user WHERE uri = ?");
  s.bind(1, owner);
  s.executeStep();
  uOwner.load(s);


  Note nReplyTo;
  if (replyToUri.has_value()) {
    auto rs = STATEMENT("SELECT * FROM note WHERE uri = ?");
    rs.bind(1, replyToUri.value());
    rs.executeStep();
    nReplyTo.load(rs);
  }

  auto favs = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ?");
  favs.bind(1, uri);
  favs.executeStep();
  int fav_count = favs.getColumn(0);

  auto qFavourited = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ? AND owner = ?");
  qFavourited.bind(1, uri);
  qFavourited.bind(2, requester.uri);
  qFavourited.executeStep();
  bool favourited = (int)qFavourited.getColumn(0) > 0;

  json j = {
    {"id", id},
    {"created_at", utils::millisToIso(published)},
    {"in_reply_to_id", nReplyTo.id},
    {"in_reply_to_account_id", nullptr},
    {"quote_id", nullptr},
    {"quote", nullptr},
    {"sensitive", sensitive},
    {"spoiler_text", ""},
    {"visibility", "public"},
    {"language", "en"},
    {"uri", NOTE(id)},
    {"url", NOTE(id)},
    {"replies_count", 7},
    {"reblogs_count", 98},
    {"favourites_count", fav_count},
    {"favourited", favourited},
    {"reblogged", false},
    {"muted", false},
    {"pinned", false},
    {"bookmarked", false},
    {"content", content},
    {"reblog", nullptr},
    {"application", nullptr},
    {"account", uOwner.renderMS()},
    {"media_attachments", json::array()},
    {"mentions", json::array()},
    {"tags", json::array()},
    {"emojis", json::array()},
    {"emoji_reactions", json::array()},
    {"reactions", json::array()},
    {"card", nullptr},
    {"poll", nullptr},

    {"pleroma", {
      {"local", (bool)local},
      {"context", ""},
      {"content", {
        {"text/plain", ""}
      }},
      {"expires_at", nullptr},
      {"direct_conversation_id", nullptr},
      {"emoji_reactions", ARR},
      {"spoiler_text", {
        {"text/plain", ""}
      }},
      {"pinned_at", nullptr},
      {"conversation_id", 1},
      {"in_reply_to_account_acct", nullptr},
      {"parent_visible", true},
      {"thread_muted", false},
    }},
    {"akkoma", {
      {"source", {
        {"content", "A"},
        {"mediaType", "text/plain"}
      }}
    }},
    {"edited_at", nullptr},
    {"text", nullptr}
  };

  if (replyToUri.has_value())
    j["in_reply_to_account_id"] = nReplyTo.id;

  if (renoteUri.has_value()) {
    auto n = NoteService::lookup_ap(renoteUri.value());
    j["reblog"] = n.value().renderMS(requester);
  }

  return j;
}
