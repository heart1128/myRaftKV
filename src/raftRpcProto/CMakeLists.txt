get_filename_component(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)  # 当前目录的文件绝对路径

aux_source_directory(${SRC_DIR} SRC_LIST)

set(src_raftRpcProto ${SRC_LIST} CACHE INTERNAL "Description of the variable")