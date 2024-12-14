#pragma once
#include <common.h>
#include "entities/User.h"

namespace UserService {
  User create(string userid, string content);
}
