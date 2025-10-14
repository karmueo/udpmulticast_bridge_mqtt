#!/bin/bash

# MQTT发送器 - 快速使用指南

echo "=== MQTT JSON Sender 使用指南 ==="
echo ""
echo "1. 编译项目:"
echo "   ./build.sh"
echo ""
echo "2. 使用默认配置发送消息:"
echo "   cd build && ./mqtt_sender"
echo ""
echo "3. 使用自定义配置文件:"
echo "   cd build && ./mqtt_sender /path/to/config.json"
echo ""
echo "4. 订阅消息进行测试:"
echo "   mosquitto_sub -h localhost -t \"test/topic\""
echo ""
echo "=== 配置文件示例 ==="
echo "查看 config.json (默认配置) 和 config.example.json (示例配置)"
echo ""
