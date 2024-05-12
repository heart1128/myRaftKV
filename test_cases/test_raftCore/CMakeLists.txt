set(SRC_LIST raftKvDBServer.cpp)
add_executable(raftCoreServer ${SRC_LIST})
target_link_libraries(raftCoreServer raft rpc_lib protobuf muduo_net muduo_base pthread)


###
set(SRC_LIST2 caller.cpp)
add_executable(caller ${src_raftClerk} ${SRC_LIST2} ${src_common})
target_link_libraries(caller raft protobuf pthread)