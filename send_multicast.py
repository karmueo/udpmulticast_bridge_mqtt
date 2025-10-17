#!/usr/bin/env python3
"""
UDP Multicast Message Sender
用于向UDP组播地址发送JSON格式的测试消息
"""

import socket
import json
import time
import sys
from argparse import ArgumentParser

def send_multicast_message(message, multicast_addr="239.255.0.1", port=6000, ttl=2, use_current_timestamp=False):
    """
    发送UDP组播消息
    
    Args:
        message: 要发送的消息字符串
        multicast_addr: 组播地址
        port: 端口号
        ttl: TTL (生存时间)，决定消息能传播的范围
        use_current_timestamp: 是否在发送时使用当前时间戳更新JSON消息
    """
    try:
        # 如果需要使用当前时间戳，更新JSON消息
        if use_current_timestamp and message.startswith('{'):
            try:
                msg_data = json.loads(message)
                if 'timestamp' in msg_data:
                    msg_data['timestamp'] = int(time.time() * 1000)  # 使用毫秒级时间戳
                    message = json.dumps(msg_data)
            except json.JSONDecodeError:
                pass  # 如果不是有效的JSON，保持原样
        
        # 创建UDP套接字
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # 设置TTL
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
        
        # 发送消息
        sock.sendto(message.encode(), (multicast_addr, port))
        
        sock.close()
        
        print(f"✓ Message sent to {multicast_addr}:{port}")
        print(f"  Content: {message}")
        
        return True
    except Exception as e:
        print(f"✗ Failed to send message: {e}")
        return False

def main():
    parser = ArgumentParser(description="UDP Multicast Message Sender")
    parser.add_argument("--addr", default="239.255.0.1", 
                        help="Multicast address (default: 239.255.0.1)")
    parser.add_argument("--port", type=int, default=6000,
                        help="Port number (default: 6000)")
    parser.add_argument("--message", "-m", default=None,
                        help="Message to send")
    parser.add_argument("--json", "-j", action="store_true",
                        help="Send as JSON")
    parser.add_argument("--count", "-c", type=int, default=1,
                        help="Number of times to send (default: 1)")
    parser.add_argument("--interval", "-i", type=float, default=1.0,
                        help="Interval between sends in seconds (default: 1.0)")
    parser.add_argument("--ttl", type=int, default=2,
                        help="TTL (Time To Live) for multicast (default: 2)")
    
    args = parser.parse_args()
    
    # 如果没有指定消息，使用默认的测试消息
    if args.message is None:
        if args.json:
            args.message = json.dumps({
                "command": "stop-detect-recording",
                "timestamp": int(time.time() * 1000),  # 使用毫秒级时间戳
                "status": "success"
            })
        else:
            args.message = "Hello from UDP Multicast Test"
    
    print("=" * 60)
    print("UDP Multicast Message Sender")
    print("=" * 60)
    print(f"Target: {args.addr}:{args.port}")
    print(f"Message: {args.message}")
    print(f"Count: {args.count}")
    if args.count > 1:
        print(f"Interval: {args.interval}s")
    print("=" * 60)
    print()
    
    # 发送消息
    success_count = 0
    for i in range(args.count):
        if i > 0:
            time.sleep(args.interval)
        
        if args.count > 1:
            print(f"Sending message {i+1}/{args.count}...")
        
        if send_multicast_message(args.message, args.addr, args.port, args.ttl, True if args.json else False):
            success_count += 1
        
        if args.count > 1 and i < args.count - 1:
            print()
    
    print()
    print(f"Summary: {success_count}/{args.count} messages sent successfully")
    
    return 0 if success_count == args.count else 1

if __name__ == "__main__":
    sys.exit(main())
