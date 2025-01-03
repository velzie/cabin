#pragma once
#include <common.h>
#include "database.h"
#include "entities/Follow.h"
#include "entities/User.h"

namespace FollowService {
  Follow create(User &user, string followee);
  void undo(User &user, Follow &follow);
  Follow ingest(const string uri, const json follow);
  void ingestReject(const string id, const json activity);
  void ingestAccept(const string id, const json activity);
}
