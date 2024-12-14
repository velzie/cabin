#pragma once
#include <common.h>
#include "entities/Bite.h"
#include "entities/Follow.h"
#include "entities/User.h"

namespace BiteService {
  Bite create(User &user, User &biteee);
  Bite ingest(const json data);
}
