#pragma once
#include "../schema.h"
#include <common.h>

namespace NoteService {
  Note create(string userid, string content);
  Note ingest(const string uri, const json note);
  optional<Note> lookup(const string id);
  optional<Note> lookup_ap(const string id);
  Note fetchRemote(const string uri);
}
