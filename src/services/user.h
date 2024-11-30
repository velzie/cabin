#pragma once
#include "../schema.h"
#include "../entities/User.h"
#include <common.h>

namespace UserService {
  User create(string userid, string content);
  optional<User> lookup(const string id);
  optional<User> lookup_ap(const string uri);
  User fetchRemote(const string uri);
}
