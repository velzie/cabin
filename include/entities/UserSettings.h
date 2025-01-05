#pragma once
#include "database.h"

struct UserSettings {
  string userId;
  string email;

  std::map<string, string> frontendSettings;

  string password;


  ORM(usersettings, userId,
    F(userId)
    F(email)

    JSON(frontendSettings)
    F(password)
  )

  LOOKUPKEY(UserSettings, usersettings, userId)
};
