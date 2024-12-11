#pragma once
#include <common.h>
#include <exception>


// indicates an activity should not be retried
class InvalidActivityError : public std::exception {
  string reason;
public:
  InvalidActivityError(string r) : reason("InvalidActivity: " + r){};
    const char* what() const noexcept override {
        return reason.c_str();
    }
};

class FetchError : public std::exception {
  string reason;
public:
  FetchError(int status) : reason(FMT("FetchError: {}", status)){};
    const char* what() const noexcept override {
        return reason.c_str();
    }
};
