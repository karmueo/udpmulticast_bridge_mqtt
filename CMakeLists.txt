cmake_minimum_required(VERSION 3.10)
project(mqtt_sender)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必需的库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# JSON library for config parsing
find_package(nlohmann_json REQUIRED)

# 添加可执行文件
add_executable(mqtt_sender 
    src/main.cpp
    src/mqtt_client.cpp
    src/config_reader.cpp
    src/udp_receiver.cpp
    src/udp_to_mqtt_forwarder.cpp
)

# 包含头文件目录
target_include_directories(mqtt_sender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MOSQUITTO_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(mqtt_sender
    ${MOSQUITTO_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# 设置编译选项
target_compile_options(mqtt_sender PRIVATE -Wall -Wextra)

# 复制配置文件到构建目录
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json 
               ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)

# 启用测试
enable_testing()

# 添加测试子目录
add_subdirectory(tests)

# 安装规则
include(GNUInstallDirs)

# 安装可执行文件
install(TARGETS mqtt_sender
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装配置文件
install(FILES config.json
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
)

# 安装文档
install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# 安装头文件（可选，用于库开发）
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
