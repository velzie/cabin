#pragma once

#include "entities/Instance.h"
#include "entities/User.h"

struct MSMeta {
  string token;
  bool isPleroma;
  User &authuser;
};

struct APMeta {
  Instance &out;
};

struct Entity {
  Entity(){};
  virtual void load(SQLite::Statement &statement) = 0;
  virtual int insert() = 0;
  virtual int update() = 0;
  // virtual void renderAP(APMeta&) = 0;
  // virtual void renderMS(MSMeta&) = 0;
};
