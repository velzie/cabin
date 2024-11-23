#include "nlohmann/json.hpp"
#include <httplib.h>

class APClient {
  std::string instance;
  httplib::Client cli; 

  public:
    APClient(std::string hostname);
    httplib::Result Get(std::string pathname);
    httplib::Result Post(std::string pathname, nlohmann::json data);
};


