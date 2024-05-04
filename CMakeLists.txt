# 设置最低版本
cmake_minimum_required(VERSION 3.25)
project(raftKV) # 项目名


set(CMAKE_CXX_STANDARD 20) #c++20标准
set(CMAKE_BUILD_TYPE "Debug") # 编译的是debug版本

# 设置项目的可执行文件输出路径，一般就是根目录的bin目录，也就是${PROJECT_SOURCE_DIR}/bin
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置输出的库文件路径，一般就是lib
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置头文件（是头文件）搜索路径，也就是g++ 的-I
include_directories(${PROJECT_SOURCE_DIR}/src/common/include)
include_directories(${PROJECT_SOURCE_DIR}/src/rpc/include)
include_directories(${PROJECT_SOURCE_DIR}/src/skipList/include)
include_directories(${PROJECT_SOURCE_DIR}/test_cases)

# 设置项目库文件搜索路径 -L（每个编译出来之后，依赖都在前一个编译的lib找，所以上面要有顺序）
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 源代码
add_subdirectory(src)
add_subdirectory(test_cases)

# 依赖的子目录的编译lib，起一个名称
add_library(raft STATIC ${src_rpc} ${src_test_rpc})
# 链接的外部库/usr/local/lib, dl是链接库，一定要依赖
target_link_libraries(raft muduo_net muduo_base pthread dl)
