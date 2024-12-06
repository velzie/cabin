#pragma once
#include <common.h>
#include "database.h"
#include "entities/Note.h"

namespace NoteService {
  Note create(User &owner, string content, optional<Note> replyTo);
  Note createRenote(User &owner, Note &renotee);
  Note ingest(const string uri, const json note);
  optional<Note> lookup(const string id);
  optional<Note> lookup_ap(const string id);
  Note fetchRemote(const string uri);
}
