#include "workers/BubbleFetcher.h"
#include <cpptrace/from_current.hpp>
#include <error.h>
#include "utils.h"
#include "services/FetchService.h"


BubbleFetcher::BubbleFetcher(const string host) : cli(host) {
  cli.set_follow_location(true);
}


void BubbleFetcher::Update() {
  string path = "/api/v1/timelines/public?local=true";
  if (!min_id.empty()) {
    path += "&min_id=" + min_id;
  }

  auto req = cli.Get(path);
  ASSERT(req);
  if (req->status != 200) throw FetchError(req->status);
  json posts = json::parse(req->body);
  ASSERT(posts.is_array());
  if (posts.size() == 0) return;

  min_id = posts[0]["id"];
  for (const json post : posts) {
    string id = post["id"];
    string uri = post["uri"];

    Note n = FetchService::fetch<Note>(uri);
    debug("pulled in note {}", n.uri);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void BubbleFetcher::Launch() {
  for (const string host : cfg.bubbledHosts) {
    std::thread t([host](){
      BubbleFetcher fetcher(host);
      while (true) {
        CPPTRACE_TRY {
          fetcher.Update();
        } CPPTRACE_CATCH(const FetchError &e) { 
          // we're probably rate limited or the server is down
          warn("BubbleFetcher({}): {} sleeping for 30 minutes", e.what(), host);
          std::this_thread::sleep_for(std::chrono::minutes(30));
        } catch (const std::exception &e) {
          error("bubble fetch {} {}", host, e.what());
          utils::getStackTrace();
        }

        std::this_thread::sleep_for(std::chrono::seconds(30));
      }
    });
    t.detach();
  }
}
