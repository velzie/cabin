#include "note.h"

#include <optional>
#include <stdexcept>
#define USE_DB
#include "SQLiteCpp/Statement.h"
#include <httplib.h>
#include <common.h>
#include "../schema.h"
#include "../utils.h"
#include "user.h"
#include "../http.h"

namespace NoteService {
  Note create(string userid, string content) {
    string id = utils::genid();

    Note note = {
      .uri = NOTE(id),
      .id = id,
      .local = 1,
      .host = ct->cfg.domain,

      .content = content,
      .cw = "",
      .sensitive = false,

      .owner = USERPAGE(userid),
      .published = utils::millis(),
    };

    note.insert();

    return note;
  }

  Note ingest(const string uri, const json note) {
    URL url(uri);
    Note n = {
      .uri = uri,
      .local = false,
      .host = url.host,

      .replyToUri = note["inReplyTo"].is_null() ? nullopt : std::make_optional(note["inReplyTo"]),

      .content = note["content"],
      .sensitive = note["sensitive"],
      .owner = note["attributedTo"],
      .published = utils::isoToMillis(note["published"]),


      .lastUpdatedAt = utils::millis()
    };

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
    auto u = UserService::lookup(ct->userid);
    URL url(uri);
    APClient cli(u.value(), url.host);

    auto response = cli.Get(url.path);
    if (response->status != 200) {
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
