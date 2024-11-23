cmake_minimum_required(VERSION 3.10)
project(cottage C CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenSSL REQUIRED)

add_subdirectory(third-party/json)
add_subdirectory(third-party/fmt)
add_subdirectory(third-party/sqlitecpp)

include_directories(include)

set(SOURCES 
  src/main.cc
  src/http.cc
  src/httplib.cc
  src/server.cc
)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(cottage ${SOURCES})

target_link_libraries(cottage nlohmann_json::nlohmann_json OpenSSL::SSL fmt::fmt SQLiteCpp)
