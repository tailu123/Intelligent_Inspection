# 机器狗导航控制系统 - 用户手册

## 1. 简介

机器狗导航控制系统是一款专为四足机器人设计的导航控制软件，通过命令行界面提供直观的操作方式。本手册详细介绍系统的安装、配置和使用方法。

## 2. 系统要求

### 2.1 硬件要求
- CPU: 双核处理器，2.0GHz以上
- 内存: 4GB RAM以上
- 存储: 500MB可用空间
- 网络: 有线或无线网络连接

### 2.2 软件要求
- 操作系统: Linux (Ubuntu 18.04+/CentOS 7+)
- 依赖库: Boost 1.65+, OpenSSL 1.1+
- 编译器: GCC 7+ 或 Clang 5+
- 构建工具: CMake 3.15+

## 3. 安装指南

### 3.1 从源代码安装

```bash
# 克隆仓库
git clone https://github.com/yourusername/robot-dog-navigation.git
cd robot-dog-navigation

# 安装依赖 (Ubuntu)
sudo apt update
sudo apt install -y build-essential cmake libboost-all-dev libssl-dev

# 构建项目
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### 3.2 使用Docker

```bash
# 拉取镜像
docker pull yourusername/robot-dog-navigation:latest

# 运行容器
docker run -it --network host yourusername/robot-dog-navigation:latest
```

## 4. 快速入门

### 4.1 启动程序

```bash
# 基本启动
robot_nav

# 指定配置文件
robot_nav --config /path/to/config.json

# 启用详细日志
robot_nav --verbose
```

### 4.2 连接机器狗

在程序启动后，需要先连接到机器狗设备：

```
> connect --ip 192.168.1.100 --port 8080
```

连接成功后会显示：

```
[INFO] 成功连接到机器狗设备 (192.168.1.100:8080)
```

## 5. 命令参考

机器狗导航控制系统提供了丰富的命令行指令，所有命令均支持`help`选项获取详细帮助。

### 5.1 基本命令

| 命令 | 描述 | 示例 |
|------|------|------|
| help | 显示帮助信息 | `help` 或 `help <command>` |
| exit | 退出程序 | `exit` |
| status | 显示当前状态 | `status` |
| version | 显示版本信息 | `version` |

### 5.2 连接管理

| 命令 | 描述 | 示例 |
|------|------|------|
| connect | 连接到机器狗 | `connect --ip 192.168.1.100 --port 8080` |
| disconnect | 断开连接 | `disconnect` |
| reconnect | 重新连接 | `reconnect` |

### 5.3 导航控制

| 命令 | 描述 | 示例 |
|------|------|------|
| nav:start | 开始导航 | `nav:start --target "kitchen"` |
| nav:stop | 停止导航 | `nav:stop` |
| nav:pause | 暂停导航 | `nav:pause` |
| nav:resume | 恢复导航 | `nav:resume` |
| nav:status | 查询导航状态 | `nav:status` |

### 5.4 路径规划

| 命令 | 描述 | 示例 |
|------|------|------|
| path:plan | 规划路径 | `path:plan --from "lobby" --to "kitchen"` |
| path:show | 显示当前路径 | `path:show` |
| path:save | 保存路径 | `path:save --file "path1.json"` |
| path:load | 加载路径 | `path:load --file "path1.json"` |

### 5.5 配置管理

| 命令 | 描述 | 示例 |
|------|------|------|
| config:show | 显示当前配置 | `config:show` |
| config:set | 设置配置项 | `config:set nav.speed 1.5` |
| config:save | 保存配置 | `config:save --file "config.json"` |
| config:load | 加载配置 | `config:load --file "config.json"` |

### 5.6 日志管理

| 命令 | 描述 | 示例 |
|------|------|------|
| log:level | 设置日志级别 | `log:level debug` |
| log:show | 显示最近日志 | `log:show --lines 50` |
| log:save | 保存日志 | `log:save --file "nav.log"` |

## 6. 配置文件

系统使用JSON格式的配置文件，默认位置为`~/.config/robot_nav/config.json`。

### 6.1 配置示例

```json
{
  "network": {
    "connection_timeout": 5000,
    "retry_interval": 1000,
    "max_retries": 3
  },
  "navigation": {
    "default_speed": 1.0,
    "max_speed": 2.0,
    "planning_algorithm": "a_star",
    "safety_distance": 0.5
  },
  "system": {
    "log_level": "info",
    "log_file": "/var/log/robot_nav.log",
    "performance_mode": false
  }
}
```

### 6.2 主要配置项

| 配置项 | 描述 | 默认值 |
|--------|------|--------|
| network.connection_timeout | 连接超时时间(ms) | 5000 |
| network.retry_interval | 重试间隔(ms) | 1000 |
| network.max_retries | 最大重试次数 | 3 |
| navigation.default_speed | 默认导航速度(m/s) | 1.0 |
| navigation.max_speed | 最大导航速度(m/s) | 2.0 |
| navigation.planning_algorithm | 路径规划算法 | "a_star" |
| navigation.safety_distance | 安全距离(m) | 0.5 |
| system.log_level | 日志级别 | "info" |
| system.log_file | 日志文件路径 | "/var/log/robot_nav.log" |
| system.performance_mode | 性能模式 | false |

## 7. 状态机说明

系统使用状态机管理导航过程，了解状态机有助于更好地使用系统。

### 7.1 状态说明

- **Init**: 初始状态，系统刚启动或重置后处于此状态
- **PrepareEnterNav**: 准备导航状态，检查连接、加载地图、规划路径
- **Nav**: 导航状态，正在执行导航任务
- **Done**: 完成状态，导航任务已完成

### 7.2 状态转换说明

- **Init -> PrepareEnterNav**: 执行`nav:start`命令后
- **PrepareEnterNav -> Nav**: 准备工作完成后自动转换
- **Nav -> Done**: 到达目标位置后自动转换
- **任意状态 -> Init**: 执行`nav:stop`命令或发生严重错误

## 8. 错误处理

### 8.1 常见错误代码

| 错误代码 | 描述 | 解决方案 |
|---------|------|---------|
| E001 | 连接失败 | 检查网络和目标IP地址 |
| E002 | 连接超时 | 增加超时时间或检查网络质量 |
| E003 | 认证失败 | 检查认证凭据 |
| E004 | 导航失败 | 检查目标点是否有效 |
| E005 | 路径规划失败 | 检查地图数据是否完整 |

### 8.2 日志级别

- **ERROR**: 严重错误，影响系统正常运行
- **WARNING**: 警告信息，可能影响某些功能
- **INFO**: 一般信息，记录系统正常操作
- **DEBUG**: 调试信息，用于开发和故障排除

## 9. 最佳实践

### 9.1 性能优化建议

- 在资源受限环境下设置`system.performance_mode=true`
- 生产环境设置日志级别为INFO或WARNING
- 定期清理日志文件避免磁盘占用过大

### 9.2 安全建议

- 使用专用网络连接机器狗设备
- 定期更新系统和依赖库
- 限制访问控制软件的用户权限

### 9.3 故障排除

当系统出现问题时，可以尝试以下步骤：

1. 检查日志文件寻找错误信息
2. 重启软件尝试恢复正常状态
3. 尝试不同的网络连接方式
4. 使用`--verbose`选项启动获取更多日志

## 10. 附录

### 10.1 命令行选项

| 选项 | 描述 | 示例 |
|------|------|------|
| --help, -h | 显示帮助信息 | `robot_nav --help` |
| --version, -v | 显示版本信息 | `robot_nav --version` |
| --config, -c | 指定配置文件 | `robot_nav --config my-config.json` |
| --verbose | 启用详细日志 | `robot_nav --verbose` |
| --log-level | 设置日志级别 | `robot_nav --log-level debug` |
| --no-color | 禁用彩色输出 | `robot_nav --no-color` |

### 10.2 支持的地图格式

- JSON地图格式 (.json)
- 二进制地图格式 (.nav)
- 图像地图格式 (.png, .jpg)

### 10.3 联系与支持

- 项目主页: https://github.com/yourusername/robot-dog-navigation
- 问题报告: https://github.com/yourusername/robot-dog-navigation/issues
- 技术支持: support@example.com
