add_library(semver semver.cpp)
target_link_libraries(semver PRIVATE fmt::fmt)
target_link_libraries(semver PRIVATE absl::strings)
target_link_libraries(semver PRIVATE re2::re2)

find_package(CURL REQUIRED)

add_executable(PeerChecker can-place-dep.cpp dataset.cpp edge.cpp idealTree.cpp main.cpp node.cpp place-dep.cpp)

target_include_directories(PeerChecker PRIVATE ${CMAKE_SOURCE_DIR}/third_party/json-3.11.2)
target_include_directories(PeerChecker PRIVATE ${CMAKE_SOURCE_DIR}/third_party/redis-plus-plus-1.3.7/src)
target_include_directories(PeerChecker PRIVATE ${CURL_INCLUDE_DIR})

target_link_libraries(PeerChecker PRIVATE redis++::redis++)
target_link_libraries(PeerChecker PRIVATE semver)
target_link_libraries(PeerChecker PRIVATE fmt::fmt)
target_link_libraries(PeerChecker PRIVATE ${CURL_LIBRARIES})
