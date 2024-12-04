#pragma once
#include "database.h"

struct Instance {
  string host;

  std::time_t lastUpdatedAt;
  int remoteUsersCount;
  int remoteNotesCount;

  string description;
  string name;

  ORM(instance, host,
    F(host)

    F(lastUpdatedAt)
    F(remoteUsersCount)
    F(remoteNotesCount)

    F(description)
    F(name)
  )
};
