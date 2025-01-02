#pragma once
#include <variant>
#include "entities/Bite.h"
#include "entities/Note.h"
#include "entities/User.h"

using AnyEntity = std::variant<User, Note, Bite>;

namespace FetchService {
  AnyEntity fetch(const string uri, bool force = false);
  template<typename T> T fetch(const string uri, bool force = false) {
    return get<T>(fetch(uri, force));
  }
}
