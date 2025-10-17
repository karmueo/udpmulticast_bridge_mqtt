- [MQTT JSON Sender](#mqtt-json-sender)
  - [功能特性](#功能特性)
  - [依赖config.json项](#依赖configjson项)
    - [在Ubuntu/Debian上安装依赖](#在ubuntudebian上安装依赖)
  - [配置文件](#配置文件)
    - [配置项说明](#配置项说明)
  - [运行](#运行)
  - [测试](#测试)
  - [项目结构](#项目结构)
  - [自定义消息](#自定义消息)

# MQTT JSON Sender

这是一个使用C++实现的MQTT客户端，用于向Mosquitto broker发送JSON消息。

## 功能特性

- 通过配置文件配置MQTT连接参数
- 通过配置文件配置发送的JSON消息内容
- 支持自定义broker地址、端口和topic
- 使用CMake构建系统

## 依赖config.json项

- CMake (>= 3.10)
- libmosquitto (Mosquitto客户端库)
- C++17编译器
- Catch2 (仅用于单元/集成测试)
GoogleTest / GoogleMock (已移除：项目现在仅使用 Catch2 进行测试)

### 在Ubuntu/Debian上安装依赖

```bash
sudo apt-get update
sudo apt-get install -y cmake g++ libmosquitto-dev catch2 nlohmann-json3-dev
```

> 提示：如果发行版的 Catch2 版本较旧，可从 [Catch2 Releases](https://github.com/catchorg/Catch2/releases) 获取 v3 源码自行编译。
```

> 提示：如果需要 GoogleTest/GMock 用于其他项目，可单独安装 `gtest-devel`/`gmock-devel`；本仓库已统一使用 Catch2。

## 编译

```bash
mkdir build
cd build
cmake ..
make
```

## 配置文件

配置文件 `config.json` 包含以下参数：

```json
{
  "mqtt": {
    "broker": "localhost",
    "port": 1883,
    "topic": "test/topic",
    "qos": 1,
    "client_id": "mqtt_sender_client"
  },
  "message": {
    "command": "stop-detect-recording"
  }
}
```

### 配置项说明

- `broker`: MQTT broker的地址
- `port`: MQTT broker的端口号（默认1883）
- `topic`: 发布消息的主题
- `qos`: 消息质量等级（0, 1, 或 2）
- `client_id`: MQTT客户端ID
- `message`: 要发送的JSON消息内容（可以是任意JSON对象）

## 运行

编译完成后，在build目录下运行：

```bash
./mqtt_sender
```

或者指定配置文件路径：

```bash
./mqtt_sender /path/to/config.json
```

## 测试

1. 启动Mosquitto broker（如果还没有运行）：

```bash
mosquitto
```

2. 在另一个终端订阅主题以查看消息：

```bash
mosquitto_sub -h localhost -t "test/topic"
```

3. 运行mqtt_sender程序发送消息

## 项目结构

```
udpmulticast_bridge_mqtt/
├── CMakeLists.txt
├── config.json
├── README.md
├── include/
│   ├── mqtt_client.h
│   └── config_reader.h
└── src/
    ├── main.cpp
    ├── mqtt_client.cpp
    └── config_reader.cpp
```

## 自定义消息

你可以修改 `config.json` 中的 `message` 字段来发送不同的JSON消息。例如：

```json
{
  "mqtt": {
    "broker": "192.168.1.100",
    "port": 1883,
    "topic": "device/control",
    "qos": 1,
    "client_id": "mqtt_sender_client"
  },
  "message": {
    "command": "start-recording",
    "device_id": "camera_01",
    "timestamp": "2025-10-14T10:30:00Z"
  }
}
```
