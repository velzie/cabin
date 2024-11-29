#include "router.h"
#include "common.h"

GET(root, "/") {
  res->writeHeader("Content-Type", "text/html");
  res->end(R"(
    <h1>cabin</h1>
    <p>instance hosted using cabin</p>
  )");
}
