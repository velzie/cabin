#define PAGINATE(query, entity, paginateBy)\
  std::stringstream ss(string(req->getQuery("limit")));\
  int limit = 0;\
  ss >> limit;\
  if (!limit) limit = 20;\
  if (limit > 20) limit = 20;\
  query = query.limit(limit);\
  string max_id (req->getQuery("max_id"));\
  string min_id (req->getQuery("min_id"));\
  string since_id (req->getQuery("since_id"));\
  if (!max_id.empty()) {\
    /*start at max_id and paginated down*/ \
    entity upperEnt = entity::lookupid(max_id).value();\
    query = query.where(LT(#paginateBy, upperEnt.paginateBy)).orderBy(#paginateBy, "DESC");\
  }\
  if (!min_id.empty()) {\
    /* start at low id, paginate up*/\
    entity lowerEnt = entity::lookupid(min_id).value();\
    query = query.where(GT(#paginateBy, lowerEnt.paginateBy)).orderBy("publishedClamped", "DESC");\
  }\
  if (!since_id.empty()) {\
    /* start at most recent date, paginate down but don't go further than since_id */\
    entity lowerNote = entity::lookupid(since_id).value();\
    query = query\
      .select({"*"})\
      .from(\
          query\
          .orderBy(#paginateBy, "DESC")\
      )\
      .orderBy(#paginateBy);\
  }\
  json response = json::array();\
  string ret_max_id;\
  string ret_min_id;\
  auto q = query.build();\
  while (q.executeStep()) {\
    entity n;\
    n.load(q);\
    if (ret_min_id.empty()) ret_min_id = n.id;\
    ret_max_id = n.id;\
    response.push_back(n.renderMS(authuser));\
  }\
  res->writeHeader("Link",\
      FMT("<{}>; rel=\"next\",<{}>; rel=\"prev\"",\
        FMT("https://{}{}?max_id={}", cfg.domain, req->getUrl(), ret_max_id),\
        FMT("https://{}{}?min_id={}", cfg.domain, req->getUrl(), ret_min_id)\
      )\
  );\
  OK(response, MIMEJSON);\

