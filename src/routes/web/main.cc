#include "common.h"
#include "router.h"

GET(root, "/") {
  res.set_content(R"(
    <h1>cottage server</h1>
    <p>instance hosted using cabin</p>
  )", "text/html");
}
