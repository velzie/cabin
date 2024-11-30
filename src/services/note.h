#pragma once
#include <common.h>
#include "../schema.h"
#include "../entities/Note.h"

namespace NoteService {
  Note create(string userid, string content);
  Note createRenote(string userid, string renoteUri);
  Note ingest(const string uri, const json note);
  optional<Note> lookup(const string id);
  optional<Note> lookup_ap(const string id);
  Note fetchRemote(const string uri);
}
