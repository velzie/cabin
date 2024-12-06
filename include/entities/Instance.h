#pragma once
#include "database.h"

struct Instance {
  string host;
  string id;

  std::time_t lastUpdatedAt;
  int remoteUsersCount;
  int remoteNotesCount;

  string themecolor;
  string faviconurl;
  string iconurl;

  string description;
  string name;

  ORM(instance, host,
    F(host)
    F(id)

    F(themecolor)
    F(faviconurl)
    F(iconurl)

    F(lastUpdatedAt)
    F(remoteUsersCount)
    F(remoteNotesCount)

    F(description)
    F(name)
  )
};
