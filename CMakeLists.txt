
cmake_minimum_required(VERSION 3.22)
project(NewParser)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_SHARED_LIBS ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

set(DISABLE_TESTS ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/hiredis-1.1.0/hiredis)

set(HIREDIS_HEADER ${CMAKE_SOURCE_DIR}/third_party/hiredis-1.1.0)
set(HIREDIS_LIB hiredis::hiredis)
set(REDIS_PLUS_PLUS_BUILD_STATIC OFF)
set(REDIS_PLUS_PLUS_BUILD_TEST OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/redis-plus-plus-1.3.7)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/fmt-9.1.0)

set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/abseil-cpp-20230125.0)

set(RE2_BUILD_TESTING OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/re2-2023-02-01)
