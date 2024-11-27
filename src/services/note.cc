#include <optional>
#define USE_DB
#include "SQLiteCpp/Statement.h"
#include <httplib.h>
#include <common.h>
#include "../schema.h"
#include "../utils.h"

namespace NoteService {
  Note create(string userid, string content) {
    string id = utils::genid();

    Note note = {
      .apid = NOTE(id),
      .localid = id,
      .content = content,
      .owner = USERPAGE(userid),
      .published = utils::millis(),
      .local = 1,
      .sensitive = 0,
    };

    note.insert();

    return note;
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
