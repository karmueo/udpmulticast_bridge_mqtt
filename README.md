- [MQTT JSON Sender](#mqtt-json-sender)
  - [功能特性](#功能特性)
  - [依赖config.json项](#依赖configjson项)
    - [在Ubuntu/Debian上安装依赖](#在ubuntudebian上安装依赖)
  - [配置文件](#配置文件)
    - [配置项说明](#配置项说明)
  - [运行](#运行)
  - [系统服务配置](#系统服务配置)
    - [将mqtt\_sender设置为系统服务](#将mqtt_sender设置为系统服务)
    - [服务管理命令](#服务管理命令)
    - [故障排除](#故障排除)
  - [测试](#测试)
    - [MQTT消息测试](#mqtt消息测试)
    - [UDP组播测试](#udp组播测试)
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

## 系统服务配置

### 将mqtt_sender设置为系统服务

1. **安装程序和服务文件**

   首先安装程序到系统目录：

   ```bash
   sudo cmake --install build
   ```

   这将安装：
   - 可执行文件到 `/usr/local/bin/mqtt_sender`
   - 配置文件到 `/usr/local/etc/config.json`
   - 文档到 `/usr/local/share/doc/mqtt_sender/`

2. **创建systemd服务文件**

   创建服务文件 `/etc/systemd/system/mqtt_sender.service`：

   ```bash
   sudo vim /etc/systemd/system/mqtt_sender.service
   ```
   添加以下内容

   ```bash
   [Unit]
   Description=MQTT Sender Service
   After=network.target mosquitto.service
   Wants=mosquitto.service

   [Service]
   Type=simple
   ExecStart=/usr/local/bin/mqtt_sender /usr/local/etc/config.json
   Restart=always
   RestartSec=5
   User=mqtt
   Group=mqtt

   [Install]
   WantedBy=multi-user.target
   ```

3. **创建专用用户（可选但推荐）**

   ```bash
   sudo useradd -r -s /bin/false mqtt
   ```

4. **设置配置文件权限**

   ```bash
   sudo chown mqtt:mqtt /usr/local/etc/config.json
   sudo chmod 600 /usr/local/etc/config.json
   ```

5. **重新加载systemd并启动服务**

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl start mqtt_sender
   sudo systemctl enable mqtt_sender
   ```

6. **检查服务状态**

   ```bash
   sudo systemctl status mqtt_sender
   ```

7. **查看服务日志**

   ```bash
   sudo journalctl -u mqtt_sender -f
   ```

### 服务管理命令

```bash
# 启动服务
sudo systemctl start mqtt_sender

# 停止服务
sudo systemctl stop mqtt_sender

# 重启服务
sudo systemctl restart mqtt_sender

# 启用开机自启动
sudo systemctl enable mqtt_sender

# 禁用开机自启动
sudo systemctl disable mqtt_sender

# 查看服务状态
sudo systemctl status mqtt_sender

# 查看服务日志
sudo journalctl -u mqtt_sender -f
```

### 故障排除

如果服务启动失败，检查：

1. **配置文件权限**：
   ```bash
   ls -la /usr/local/etc/config.json
   ```

2. **Mosquitto服务状态**：
   ```bash
   sudo systemctl status mosquitto
   ```

3. **网络连接**：
   ```bash
   sudo journalctl -u mqtt_sender -n 50
   ```

4. **手动测试**：
   ```bash
   sudo -u mqtt /usr/local/bin/mqtt_sender /usr/local/etc/config.json
   ```

## 测试

### MQTT消息测试

1. 启动Mosquitto broker（如果还没有运行）：

```bash
mosquitto
```

2. 在另一个终端订阅主题以查看消息：

```bash
mosquitto_sub -h localhost -t "test/topic"
```

3. 运行mqtt_sender程序发送消息

### UDP组播测试

项目还包含一个Python UDP组播测试工具，可以用来测试UDP组播消息发送：

```bash
# 发送单条JSON消息
python3 send_multicast.py -j

# 发送100条JSON消息（每条间隔1秒）
python3 send_multicast.py -j -c 100

# 发送100条JSON消息（每条间隔0.1秒）
python3 send_multicast.py -j -c 100 -i 0.1

# 自定义组播地址和端口
python3 send_multicast.py -j -c 10 --addr 239.255.0.1 --port 6000
```

UDP组播测试工具支持以下参数：
- `-j, --json`: 发送JSON格式消息
- `-c, --count`: 发送消息数量（默认1）
- `-i, --interval`: 发送间隔秒数（默认1.0）
- `--addr`: 组播地址（默认239.255.0.1）
- `--port`: 端口号（默认6000）
- `-m, --message`: 自定义消息内容

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
