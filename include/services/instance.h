#pragma once
#include "entities/Instance.h"

namespace InstanceService {
  optional<Instance> lookup(const string host);
  Instance fetchRemote(const string host);
}
