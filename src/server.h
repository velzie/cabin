#pragma once

#include "nlohmann/json.hpp"
#include <memory>
#ifndef _Server 
struct _Server;
#endif


  

class Server {
  public:
    Server();
    ~Server();
    void Start();
  protected:
    nlohmann::json NodeMeta(std::string version);
    _Server* svr;
};
