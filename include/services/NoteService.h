#pragma once
#include <common.h>
#include "database.h"
#include "entities/Note.h"

namespace NoteService {
  Note create(User &owner, string content, optional<string> contentWarning, optional<Note> replyTo, optional<Note> quote, bool preview, int visibility, vector<string> mediaIds);
  Note createRenote(User &owner, Note &renotee);
}
