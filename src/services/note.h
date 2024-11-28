#pragma once
#include "../schema.h"
#include <common.h>

namespace NoteService {
  Note create(string userid, string content);
  optional<Note> lookup(const string localid);
  optional<Note> lookup_ap(const string localid);
  Note fetchRemote(const string apid);
}
