#include "common.h"
#include "database.h"
#include "entity.h"
#include <variant>



using SqlType = std::variant<time_t, bool, string>;

class QueryBuilder;
struct QueryWhere {
  string op;
  string left;
  SqlType right;
  bool isNotNull = false;
  bool isNull = false;
  std::shared_ptr<QueryBuilder> subquery;
};


struct QueryUnion;
using QueryConstraint = std::variant<QueryWhere, QueryUnion>;

struct QueryUnion {
  bool isOr; // otherwise and
  std::shared_ptr<QueryConstraint> left;
  std::shared_ptr<QueryConstraint> right;
};



class QueryBuilder {
  string queryType;
  vector<string> columns;

  optional<string> m_table;
  std::shared_ptr<QueryBuilder> m_tableSubquery;

  optional<string> m_orderBy;
  optional<string> m_direction;
  optional<int> m_limit;
  optional<int> m_offset;

  vector<QueryConstraint> m_constraints;




  public:
  QueryBuilder where(const QueryConstraint value) {
    QueryBuilder q = *this;
    q.m_constraints.push_back(value);
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


  void reset() {
    m_constraints.clear();
    m_table.reset();
    m_tableSubquery.reset();
    m_orderBy.reset();
    m_direction.reset();
    m_limit.reset();
    m_offset.reset();
  }

  void expandUnion(QueryConstraint c, string &query) {
    if (std::holds_alternative<QueryWhere>(c)) {
      auto q = std::get<QueryWhere>(c);
      if (q.isNull) {
        query += q.left + " IS NULL ";
      } else if (q.isNotNull) {
        query += q.left + " IS NOT NULL ";
      } else if (q.subquery) {
        query += q.left + " IN (" + q.subquery->build().getExpandedSQL() + ") ";
      } else {
        query += q.left + " " + q.op + " ? ";
      }
    } else {
      auto q = std::get<QueryUnion>(c);
      query += "(";
      expandUnion(*q.left, query);
      query += q.isOr ? " OR " : " AND ";
      expandUnion(*q.right, query);
      query += ")";
    }
  };

  void expandUnionBind(QueryConstraint &c, SQLite::Statement &q, int &ibind) {
    if (std::holds_alternative<QueryWhere>(c)) {
      auto qwhere = std::get<QueryWhere>(c);
      if (qwhere.isNull || qwhere.isNotNull) {
        return;
      } else if (qwhere.subquery) {
        return;
      } else {
        if (std::holds_alternative<time_t>(qwhere.right)) {
          q.bind(ibind, std::get<time_t>(qwhere.right));
        } else if (std::holds_alternative<bool>(qwhere.right)) {
          q.bind(ibind, std::get<bool>(qwhere.right));
        } else if (std::holds_alternative<string>(qwhere.right)) {
          q.bind(ibind, std::get<string>(qwhere.right));
        }
        ibind++;
      }
    } else {
      auto qunion = std::get<QueryUnion>(c);
      expandUnionBind(*qunion.left, q, ibind);
      expandUnionBind(*qunion.right, q, ibind);
    }
  };

  template<typename T> optional<T> getOne() {
    auto s = build();
    if (!s.executeStep()) return nullopt;
    T e;
    e.load(s);
    return e;
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

      if (!m_constraints.empty()) {
        query += " WHERE ";
      }

      if (!m_constraints.empty()) {
        for (auto c : m_constraints) {
          expandUnion(c, query);
          query += " AND ";
        }
      }


      if (!m_constraints.empty()) 
        query.erase(query.size() - 5);

      if (m_orderBy.has_value()) {
        query += " ORDER BY " + m_orderBy.value() + " " + m_direction.value();
      }

      if (m_limit.has_value()) {
        query += " LIMIT ?"; 
      }

      auto q = STATEMENT(query);
      int ibind = 1;

      if (!m_constraints.empty()) {
        for (auto un : m_constraints) {
          expandUnionBind(un, q, ibind);
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


inline QueryWhere ISNULL(string left) {
  QueryWhere q;
  q.isNull = true;
  q.left = left;
  return q;
}

inline QueryWhere ISNOTNULL(string left) {
  QueryWhere q;
  q.isNotNull = true;
  q.left = left;
  return q;
}

inline QueryWhere IN(string left, QueryBuilder subquery) {
  QueryWhere q;
  q.subquery = std::make_shared<QueryBuilder>(subquery);
  q.left = left;
  return q;
}

inline QueryWhere GT(string left, SqlType right) {
  QueryWhere q;
  q.op = ">";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryWhere LT(string left, SqlType right) {
  QueryWhere q;
  q.op = "<";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryWhere GTE(string left, SqlType right) {
  QueryWhere q;
  q.op = ">=";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryWhere LTE(string left, SqlType right) {
  QueryWhere q;
  q.op = "<=";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryWhere EQ(string left, SqlType right) {
  QueryWhere q;
  q.op = "=";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryWhere NEQ(string left, SqlType right) {
  QueryWhere q;
  q.op = "!=";
  q.left = left;
  q.right = right;
  return q;
}

inline QueryUnion AND(QueryConstraint left, QueryConstraint right) {
  QueryUnion q;
  q.isOr = false;
  q.left = std::make_shared<QueryConstraint>(left);
  q.right = std::make_shared<QueryConstraint>(right);
  return q;
}

inline QueryUnion OR(QueryConstraint left, QueryConstraint right) {
  QueryUnion q;
  q.isOr = true;
  q.left = std::make_shared<QueryConstraint>(left);
  q.right = std::make_shared<QueryConstraint>(right);
  return q;
}
