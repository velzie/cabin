#include "common.h"
#include <regex>

URL::URL(string s) {
  std::regex urlRegex(
        R"(([^:]+)://([^:/?#]+)(?::([0-9]+))?([^?#]*)(\?([^#]*))?(#(.*))?)"
    );
  std::smatch urlMatch;

    if (regex_match(s, urlMatch, urlRegex)) {
        // Assign parts of the URL
        scheme = urlMatch[1];
        host = urlMatch[2];
        port = urlMatch[3];
        path = urlMatch[4];
        query = urlMatch[6];
        frag = urlMatch[8];

        // Parse query into key-value pairs
        if (!query.empty()) {
            parseQuery(query);
        }
    } else {
        throw std::invalid_argument("Invalid URL format");
    }
}
void URL::parseQuery(const string& queryStr) {
    std::stringstream ss(queryStr);
        string pair;

        while (getline(ss, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != string::npos) {
                string key = pair.substr(0, eqPos);
                string value = pair.substr(eqPos + 1);
                queryMap[key] = value;
            } else {
                // Key without a value
                queryMap[pair] = "";
            }
        }
    }
