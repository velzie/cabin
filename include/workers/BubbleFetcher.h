#pragma once
#include "common.h"
#include "http.h"



class BubbleFetcher {
  httplib::Client cli;
  string min_id;
  public:
  BubbleFetcher(const string host);
  void Update();
  static void Launch();
};
