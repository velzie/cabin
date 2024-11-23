#pragma once

#include <memory>
#ifndef _Server 
struct _Server;
#endif

class Server {
  public:
    Server();
    ~Server();
    void Start();
  private:
    _Server* svr;
};
