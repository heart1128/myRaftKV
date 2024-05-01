# 获取当前源文件所在目录的绝对路径
get_filename_component(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

# 目录下所有源文件添加到SRC_LIST变量中
aux_source_directory(${SRC_DIR} SRC_LIST)
# 编译出来的lib
add_library(rpc_lib ${SRC_LIST} ${src_common})
# 依赖的外部库
target_link_libraries(rpc_lib boost_serialization)
# 设置这个变量，最外层的CMakefile文件能用这个变量
set(src_rpc ${SRC_LIST} CACHE INTERNAL "Description of the variable")