#pragma once
#include <common.h>

namespace DeliveryService {

  struct Audience {
    std::vector<string> mentions;
    bool aspublic;
    bool followers;
    bool forcefederate;

    bool showcc;
  };

  void QueueDelivery(json activity, Audience audience);
}
