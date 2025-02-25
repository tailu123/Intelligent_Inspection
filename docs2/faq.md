# 常见问题

## 系统相关

### 1. 系统无法启动

Q: 系统启动时报错 "Failed to initialize network"
A: 可能的原因：
- 网络配置错误
- 端口被占用
- 权限不足
- 依赖库缺失

解决方法：
```bash
# 检查网络配置
cat /etc/inspection/config.json

# 检查端口占用
netstat -tulpn | grep 8080

# 检查权限
ls -l /usr/local/bin/inspection_system

# 检查依赖
ldd /usr/local/bin/inspection_system
```

### 2. 性能问题

Q: 系统运行缓慢，CPU使用率高
A: 可能的原因：
- 配置参数不合理
- 数据量过大
- 内存泄漏
- 日志过多

解决方法：
```bash
# 检查系统负载
top -p $(pgrep inspection_system)

# 检查内存使用
ps aux | grep inspection_system

# 检查日志大小
du -sh /var/log/inspection/*

# 优化配置参数
vim /etc/inspection/config.json
```

## 网络相关

### 1. 连接问题

Q: 无法连接到机器人
A: 可能的原因：
- 网络不通
- IP/端口错误
- 防火墙拦截
- 协议不匹配

解决方法：
```bash
# 测试网络连通性
ping <robot_ip>

# 检查防火墙规则
sudo iptables -L | grep 8080

# 测试端口连通性
telnet <robot_ip> <port>

# 检查协议配置
cat /etc/inspection/protocol.json
```

### 2. 通信异常

Q: 通信经常断开或不稳定
A: 可能的原因：
- 网络不稳定
- 超时设置不合理
- 心跳包配置错误
- 缓冲区溢出

解决方法：
```bash
# 监控网络状态
iperf3 -c <robot_ip>

# 调整超时参数
vim /etc/inspection/network.json

# 检查心跳配置
vim /etc/inspection/heartbeat.json

# 查看网络统计
netstat -s | grep -i retransmit
```

## 任务相关

### 1. 任务执行失败

Q: 任务创建成功但执行失败
A: 可能的原因：
- 任务参数错误
- 资源不足
- 状态异常
- 权限问题

解决方法：
```bash
# 检查任务配置
cat /var/lib/inspection/tasks/<task_id>.json

# 查看任务日志
tail -f /var/log/inspection/task.log

# 检查系统状态
inspection_system --status

# 验证执行权限
sudo -u inspection whoami
```

### 2. 任务状态异常

Q: 任务状态显示不正确或更新不及时
A: 可能的原因：
- 状态同步失败
- 数据库异常
- 缓存问题
- 并发冲突

解决方法：
```bash
# 检查状态日志
tail -f /var/log/inspection/state.log

# 重置任务状态
inspection_system --reset-task <task_id>

# 清理缓存
inspection_system --clear-cache

# 检查数据一致性
inspection_system --check-state
```

## 配置相关

### 1. 配置加载失败

Q: 系统报错 "Failed to load configuration"
A: 可能的原因：
- 配置文件格式错误
- 文件权限问题
- 路径错误
- 配置项缺失

解决方法：
```bash
# 验证配置文件格式
jsonlint /etc/inspection/config.json

# 检查文件权限
ls -l /etc/inspection/

# 检查配置路径
echo $INSPECTION_CONFIG_PATH

# 使用默认配置
cp /etc/inspection/config.json.default /etc/inspection/config.json
```

### 2. 配置更新不生效

Q: 修改配置后系统行为未改变
A: 可能的原因：
- 未重启服务
- 配置未保存
- 缓存未更新
- 配置覆盖问题

解决方法：
```bash
# 重启服务
sudo systemctl restart inspection

# 验证配置是否保存
md5sum /etc/inspection/config.json

# 强制重新加载配置
inspection_system --reload-config

# 检查配置优先级
inspection_system --show-config
```

## 开发相关

### 1. 编译错误

Q: 编译时报错 "undefined reference"
A: 可能的原因：
- 依赖库缺失
- 链接顺序错误
- 编译器版本不匹配
- CMake配置错误

解决方法：
```bash
# 安装依赖
sudo apt install -y libboost-all-dev

# 检查CMake配置
cat CMakeLists.txt

# 清理构建目录
rm -rf build && mkdir build

# 使用特定编译器版本
CC=gcc-9 CXX=g++-9 cmake ..
```

### 2. 调试问题

Q: 如何调试系统问题
A: 可以使用以下方法：
- 启用调试日志
- 使用GDB调试
- 添加调试点
- 使用性能分析工具

解决方法：
```bash
# 启用调试日志
sed -i 's/INFO/DEBUG/g' /etc/inspection/log.json

# GDB调试
gdb --args /usr/local/bin/inspection_system

# 性能分析
perf record -g /usr/local/bin/inspection_system

# 内存检查
valgrind --leak-check=full /usr/local/bin/inspection_system
```

## 维护相关

### 1. 日志管理

Q: 日志文件占用空间过大
A: 可能的原因：
- 日志级别过低
- 轮转配置不当
- 清理策略不合理
- 磁盘空间不足

解决方法：
```bash
# 检查日志级别
grep "level" /etc/inspection/log.json

# 配置日志轮转
vim /etc/logrotate.d/inspection

# 手动清理旧日志
find /var/log/inspection -name "*.log.*" -mtime +30 -delete

# 压缩历史日志
gzip /var/log/inspection/*.log.*
```

### 2. 备份恢复

Q: 如何备份和恢复系统数据
A: 系统提供以下备份方案：
- 配置备份
- 数据备份
- 状态备份
- 完整备份

解决方法：
```bash
# 备份配置
tar czf config_backup.tar.gz /etc/inspection/

# 备份数据
tar czf data_backup.tar.gz /var/lib/inspection/

# 备份状态
inspection_system --backup-state

# 恢复系统
inspection_system --restore-backup backup.tar.gz
```
