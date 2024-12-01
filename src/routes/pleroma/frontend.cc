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

PUT(put_frontend_settings_default, "/api/v1/akkoma/frontend_settings/:id/default") {

}

GET(frontend_settings_default, "/api/v1/akkoma/frontend_settings/:id/default") {

}

PUT(put_frontend_settings, "/api/v1/akkoma/frontend_settings/:id") {

}

GET(frontend_settings, "/api/v1/akkoma/frontend_settings/:id") {

}
