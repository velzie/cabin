#include "common.h"
#include "database.h"

class QueryBuilder {
  string queryType;
  vector<string> columns;

  optional<string> m_table;
  optional<QueryBuilder*> m_tableSubquery;

  optional<string> m_orderBy;
  optional<string> m_direction;
  optional<int> m_limit;
  optional<int> m_offset;

  std::map<string, string> m_where;
  std::map<string, QueryBuilder> m_wherein;




  public:
  QueryBuilder where(const string key, const string value) {
    m_where[key] = value;
    return *this;
  }

  QueryBuilder select(const vector<string> _columns) {
    columns = _columns;
    queryType = "SELECT";
    return *this;
  }

  QueryBuilder from(const string table) {
    this->m_table = table;
    return *this;
  }

  QueryBuilder from(QueryBuilder subquery) {
    this->m_tableSubquery = &subquery;
    return *this;
  }

  QueryBuilder limit(int limit) {
    m_limit = limit;
    return *this;
  }

  QueryBuilder offset(int _offset) {
    m_offset = _offset;
    return *this;
  }

  QueryBuilder orderBy(const string _orderBy, const string _direction) {
    m_orderBy = _orderBy;
    m_direction = _direction;
    return *this;
  }

  QueryBuilder whereIn(const string key, QueryBuilder subquery) {
    m_wherein[key] = std::move(subquery);
    return *this;
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
        query += "(" + m_tableSubquery.value()->build().getExpandedSQL() + ")";
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

      dbg(query);
      auto q = STATEMENT(query);
      int ibind = 1;

      if (!m_where.empty()) {
        for (auto [key, value] : m_where) {
          q.bind(ibind, value);
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
