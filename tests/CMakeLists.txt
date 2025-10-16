cmake_minimum_required(VERSION 3.10)
project(udpmulticast_bridge_mqtt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找系统安装的Catch2
find_package(Catch2 3 REQUIRED)

# 测试可执行文件 - Catch2版本
add_executable(udp_receiver_test 
    udp_receiver_simple_test.cpp
    ../src/udp_receiver.cpp
)

target_include_directories(udp_receiver_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(udp_receiver_test PRIVATE Catch2::Catch2WithMain)

target_compile_options(udp_receiver_test PRIVATE -Wall -Wextra)

# 启用测试
enable_testing()
add_test(NAME UdpReceiverTests COMMAND udp_receiver_test)

# MQTT客户端测试
add_executable(mqtt_client_test 
    mqtt_client_test.cpp
    ../src/mqtt_client.cpp
)

target_include_directories(mqtt_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 查找mosquitto库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

target_include_directories(mqtt_client_test PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
target_link_libraries(mqtt_client_test PRIVATE 
    Catch2::Catch2WithMain
    ${MOSQUITTO_LIBRARIES}
)

target_compile_options(mqtt_client_test PRIVATE -Wall -Wextra)

add_test(NAME MqttClientTests COMMAND mqtt_client_test)

# UDP到MQTT转发器测试
add_executable(udp_to_mqtt_forwarder_test 
    udp_to_mqtt_forwarder_test.cpp
    ../src/udp_to_mqtt_forwarder.cpp
    ../src/mqtt_client.cpp
    ../src/udp_receiver.cpp
)

target_include_directories(udp_to_mqtt_forwarder_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_include_directories(udp_to_mqtt_forwarder_test PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
target_link_libraries(udp_to_mqtt_forwarder_test PRIVATE 
    Catch2::Catch2WithMain
    ${MOSQUITTO_LIBRARIES}
)

target_compile_options(udp_to_mqtt_forwarder_test PRIVATE -Wall -Wextra)

add_test(NAME UdpToMqttForwarderTests COMMAND udp_to_mqtt_forwarder_test)
