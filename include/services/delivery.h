#pragma once
#include <common.h>
#include "../entities/User.h"

namespace DeliveryService {

  struct Audience {
    optional<User> actor;

    std::vector<string> mentions;
    bool aspublic;
    bool followers;
    bool forcefederate;

    bool showcc;
  };

  void QueueDelivery(json activity, Audience audience);
}
