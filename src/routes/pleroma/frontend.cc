#include <router.h>
#include <common.h>


GET(frontend_configurations, "/api/pleroma/frontend_configurations") {
  json r = {
    {"pleroma_fe", {
      {"loginMethod", "token"},
      {"useStreamingApi", false},
    }},
  };

  OK(r, MIMEJSON);
}
