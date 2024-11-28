#pragma once
#include "../schema.h"
#include <common.h>

namespace UserService {
  User create(string userid, string content);
  optional<User> lookup(const string id);
}
void update_remote_user(string apid);
