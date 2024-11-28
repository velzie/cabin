#pragma once
#include "../schema.h"
#include <common.h>

namespace UserService {
  User create(string userid, string content);
  optional<User> lookup(const string localid);
  optional<User> lookup_ap(const string apid);
  User fetchRemote(const string apid);
}
