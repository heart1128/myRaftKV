set(SRC_LIST test.cpp)
add_executable(test_skipList ${SRC_LIST})
target_link_libraries(test_skipList protobuf)