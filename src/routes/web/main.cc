#include "common.h"
#include "router.h"

GET(root, "/") {
  res.set_content(R"(
    <h1>cabin</h1>
    <p>instance hosted using cabin</p>
  )", "text/html");
}
