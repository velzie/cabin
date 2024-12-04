#pragma once
#include <common.h>
#include "entities/User.h"

namespace UserService {
  User create(string userid, string content);
  optional<User> lookup(const string id);
  optional<User> lookup_ap(const string uri);
  User fetchRemote(const string uri);
}
