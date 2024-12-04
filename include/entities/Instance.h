#pragma once
#include "../schema.h"

struct Instance {
  string host;

  std::time_t lastUpdatedAt;
  int remoteUsersCount;
  int remoteNotesCount;

  string description;
  string name;

  ORM(instance,
    F(host)

    F(lastUpdatedAt)
    F(remoteUsersCount)
    F(remoteNotesCount)

    F(description)
    F(name)
  )
};
