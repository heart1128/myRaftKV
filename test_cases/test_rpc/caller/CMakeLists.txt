# set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_LIST callFriendService.cpp ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer rpc_lib protobuf pthread) # fixbug，这里不加pthread会出问题，导致protobuf反射出错
