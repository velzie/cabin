#pragma once
#include <migration.h>

extern std::map<int, MigrationHandler> migrations_up;
extern std::map<int, MigrationHandler> migrations_down;
