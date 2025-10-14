cmake_minimum_required(VERSION 3.10)
project(mqtt_sender)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必需的库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# 添加可执行文件
add_executable(mqtt_sender 
    src/main.cpp
    src/mqtt_client.cpp
    src/config_reader.cpp
)

# 包含头文件目录
target_include_directories(mqtt_sender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MOSQUITTO_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(mqtt_sender
    ${MOSQUITTO_LIBRARIES}
)

# 设置编译选项
target_compile_options(mqtt_sender PRIVATE -Wall -Wextra)

# 复制配置文件到构建目录
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json 
               ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
