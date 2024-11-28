#include <optional>
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
      .apid = NOTE(id),
      .localid = id,
      .local = 1,
      .content = content,
      .owner = USERPAGE(userid),
      .published = utils::millis(),
      .sensitive = 0,
    };

    note.insert();

    return note;
  }

  Note fetchRemote(const string apid) {
    auto u = UserService::lookup(ct->userid);
    URL url(apid);
    APClient cli(u.value(), url.host);

    auto response = cli.Get(url.path);
    dbg(response->body);
    assert(response->status == 200);
    json note = json::parse(response->body);


    UserService::fetchRemote(note["attributedTo"]);
    Note n = {
      .apid = apid,
      .local = false,

      .content = note["content"],
      .owner = note["attributedTo"],
      .published = utils::isoToMillis(note["published"]),
      .sensitive = note["sensitive"],
    };

    auto query = STATEMENT("SELECT localid FROM note where apid = ?");
    query.bind(1, apid);
    if (query.executeStep()) {
      // note exists, update
      string localid = query.getColumn("localid");
      
      // TODO orm stuff etc
      auto delq = STATEMENT("DELETE FROM note WHERE apid = ?");
      delq.bind(1, apid);
      delq.exec();

      n.localid = localid;
      n.insert();
    } else {
      n.localid = utils::genid();
      n.insert();
    }

    return n;
  }

  optional<Note> lookup(const string id) {
    auto q = STATEMENT("SELECT * FROM note WHERE localid = ?");
    q.bind(1, id);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    Note n;
    n.load(q);
    return n;
  }
}
