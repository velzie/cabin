#include "common.h"
#include "database.h"

class QueryBuilder {
  string queryType;
  vector<string> columns;

  optional<string> m_table;
  std::shared_ptr<QueryBuilder> m_tableSubquery;

  optional<string> m_orderBy;
  optional<string> m_direction;
  optional<int> m_limit;
  optional<int> m_offset;

  using SqlType = std::variant<time_t, bool, string>;
  std::map<string, SqlType> m_where;
  std::map<string, QueryBuilder> m_wherein;




  public:
  QueryBuilder where(const string key, const SqlType value) {
    QueryBuilder q = *this;
    q.m_where[key] = value;
    return q;
  }

  QueryBuilder select(const vector<string> _columns) {
    QueryBuilder q = *this;
    q.columns = _columns;
    q.queryType = "SELECT";
    q.reset();
    return q;
  }

  QueryBuilder from(const string table) {
    QueryBuilder q = *this;
    q.m_table = table;
    return q;
  }

  QueryBuilder from(QueryBuilder subquery) {
    QueryBuilder q = *this;
    // TODO: this is bad use moves instead
    q.m_tableSubquery = std::make_shared<QueryBuilder>(subquery);
    return q;
  }

  QueryBuilder limit(int limit) {
    QueryBuilder q = *this;
    q.m_limit = limit;
    return q;
  }

  QueryBuilder offset(int _offset) {
    QueryBuilder q = *this;
    q.m_offset = _offset;
    return q;
  }

  QueryBuilder orderBy(const string _orderBy, const string _direction = "ASC") {
    QueryBuilder q = *this;
    q.m_orderBy = _orderBy;
    q.m_direction = _direction;
    return q;
  }

  QueryBuilder whereIn(const string key, QueryBuilder subquery) {
    QueryBuilder q = *this;
    q.m_wherein[key] = std::move(subquery);
    return q;
  }

  void reset() {
    m_where.clear();
    m_wherein.clear();
    m_table.reset();
    m_tableSubquery.reset();
    m_orderBy.reset();
    m_direction.reset();
    m_limit.reset();
    m_offset.reset();
  }

  SQLite::Statement build() {
    string query;
    if (queryType == "SELECT") {
      query = "SELECT ";
      for (auto c : columns) {
        query += c + ",";
      }
      query.pop_back();
      query += " FROM ";

      if (m_table.has_value()) {
        query += m_table.value();
      } else {
        query += "(" + m_tableSubquery->build().getExpandedSQL() + ")";
      }

      if (!m_where.empty() || !m_wherein.empty()) {
        query += " WHERE ";
      }

      if (!m_where.empty()) {
        for (auto [key, value] : m_where) {
          query += key + "? AND ";
        }
      }

      if (!m_wherein.empty()) {
        for (auto [key, subquery] : m_wherein) {
          query += key + " IN (" + subquery.build().getExpandedSQL() + ") AND ";
        }
      }

      if (!m_where.empty() || !m_wherein.empty()) 
        query.erase(query.size() - 5);

      if (m_orderBy.has_value()) {
        query += " ORDER BY " + m_orderBy.value() + " " + m_direction.value();
      }

      if (m_limit.has_value()) {
        query += " LIMIT ?"; 
      }

      auto q = STATEMENT(query);
      int ibind = 1;

      if (!m_where.empty()) {
        for (auto [key, value] : m_where) {
          if (std::holds_alternative<time_t>(value)) {
            q.bind(ibind, std::get<time_t>(value));
          } else if (std::holds_alternative<bool>(value)) {
            q.bind(ibind, std::get<bool>(value));
          } else if (std::holds_alternative<string>(value)) {
            q.bind(ibind, std::get<string>(value));
          }
          ibind++;
        }
      }

      if (m_limit.has_value()) {
        q.bind(ibind, m_limit.value());
      }

      return q;
    }
    ASSERT(false);
  }
};
