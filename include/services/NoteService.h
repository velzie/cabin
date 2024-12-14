#pragma once
#include <common.h>
#include "database.h"
#include "entities/Note.h"

namespace NoteService {
  Note create(User &owner, string content, optional<Note> replyTo, optional<Note> quote, bool preview);
  Note createRenote(User &owner, Note &renotee);
}
