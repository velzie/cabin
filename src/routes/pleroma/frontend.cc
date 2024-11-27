#include <common.h>
#include <router.h>


GET(frontend_configurations, "/api/pleroma/frontend_configurations") {
  json r = {
    {"pleroma_fe", {
      {"loginMethod", "token"},
      {"useStreamingApi", false},
    }},
  };

  res.set_content(r.dump(), "application/json");
}
