cmake_minimum_required(VERSION 3.10)
project(x30_inspection_system
    VERSION 1.0
    DESCRIPTION "X30 Inspection System"
    LANGUAGES CXX
)

# 基础配置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 查找依赖库
find_package(Boost REQUIRED COMPONENTS
    system
    filesystem
    thread
)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_path(rapidxml/rapidxml_utils.hpp REQUIRED)

# 源文件配置
set(SOURCES
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

# 创建主可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# 设置编译选项
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Boost::system
        Boost::filesystem
        Boost::thread
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        pthread
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
