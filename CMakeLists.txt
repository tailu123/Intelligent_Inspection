cmake_minimum_required(VERSION 3.10)
project(x30_inspection_system VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置编译选项
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# 添加 libhv 子模块
# add_subdirectory(third_party/libhv)

# 添加编译选项
add_compile_options(-Wall -Wextra)


# 查找Boost库
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
# 查找nlohmann_json库
find_package(nlohmann_json)
# 查找fmt库
find_package(fmt)

# 添加include目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
    ${Boost_INCLUDE_DIRS}
    # ${libhv_INCLUDE_DIRS}
)

# 添加源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# 排除测试文件
list(FILTER SOURCES EXCLUDE REGEX ".*test/.*\\.cpp$")

# 创建主可执行文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/application/x30_inspection_system.cpp
    src/common/message_queue.cpp
    src/procedure/nav_procedure/nav_procedure.cpp
    src/network/network_model_manager.cpp
    src/network/asio_network_model.cpp
    src/network/epoll_network_model.cpp
    src/network/libhv_network_model.cpp
    src/network/network_model_factory.cpp
    src/protocol/x30_protocol.cpp
    src/protocol/protocol_header.cpp
    src/common/event_bus.cpp
    src/common/utils.cpp
    src/state/nav/nav_actions.cpp
    src/state/nav/nav_guards.cpp
)

# 链接主程序
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${Boost_LIBRARIES}
    pthread
    nlohmann_json::nlohmann_json
    fmt
)

# 可选：设置 C++ 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
