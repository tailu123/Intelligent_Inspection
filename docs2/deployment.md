# 部署指南

## 系统要求

### 1. 硬件要求

- CPU：Intel Core i5 或更高
- 内存：8GB 或更高
- 磁盘空间：20GB 可用空间
- 网络：千兆以太网

### 2. 软件要求

- 操作系统：Ubuntu 20.04 LTS 或更高版本
- 编译器：GCC 9+ 或 Clang 10+
- CMake：3.15 或更高版本
- Boost：1.71 或更高版本
- 其他依赖库：
  - libprotobuf-dev
  - libjsoncpp-dev
  - libspdlog-dev
  - libfmt-dev
  - libssl-dev

## 安装步骤

### 1. 安装依赖

```bash
# 更新系统
sudo apt update
sudo apt upgrade -y

# 安装基础开发工具
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config

# 安装依赖库
sudo apt install -y \
    libboost-all-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libjsoncpp-dev \
    libspdlog-dev \
    libfmt-dev \
    libssl-dev
```

### 2. 获取源码

```bash
# 克隆仓库
git clone https://github.com/your-org/intelligent-inspection.git
cd intelligent-inspection

# 更新子模块
git submodule update --init --recursive
```

### 3. 编译安装

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      ..

# 编译
make -j$(nproc)

# 安装
sudo make install
```

## 配置说明

### 1. 系统配置

```json
{
    "system": {
        "name": "InspectionSystem",
        "version": "1.0.0",
        "log_level": "INFO",
        "log_path": "/var/log/inspection",
        "data_path": "/var/lib/inspection"
    },
    "network": {
        "host": "0.0.0.0",
        "port": 8080,
        "timeout": 5000,
        "max_connections": 100,
        "keep_alive": true
    }
}
```

### 2. 服务配置

```bash
# 创建系统用户
sudo useradd -r -s /bin/false inspection

# 创建必要目录
sudo mkdir -p /etc/inspection
sudo mkdir -p /var/log/inspection
sudo mkdir -p /var/lib/inspection

# 设置权限
sudo chown -R inspection:inspection /etc/inspection
sudo chown -R inspection:inspection /var/log/inspection
sudo chown -R inspection:inspection /var/lib/inspection
```

### 3. 系统服务

```ini
# /etc/systemd/system/inspection.service
[Unit]
Description=Intelligent Inspection System
After=network.target

[Service]
Type=simple
User=inspection
Group=inspection
ExecStart=/usr/local/bin/inspection_system
WorkingDirectory=/var/lib/inspection
ConfigurationDirectory=inspection
LogsDirectory=inspection
StateDirectory=inspection

# 资源限制
LimitNOFILE=65535
LimitNPROC=65535

# 重启策略
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

## 启动和停止

### 1. 系统服务管理

```bash
# 启动服务
sudo systemctl start inspection

# 停止服务
sudo systemctl stop inspection

# 重启服务
sudo systemctl restart inspection

# 查看状态
sudo systemctl status inspection

# 设置开机启动
sudo systemctl enable inspection
```

### 2. 日志查看

```bash
# 查看系统日志
sudo journalctl -u inspection

# 查看应用日志
sudo tail -f /var/log/inspection/system.log

# 查看错误日志
sudo tail -f /var/log/inspection/error.log
```

## 监控和维护

### 1. 系统监控

```bash
# 检查进程状态
ps aux | grep inspection_system

# 检查端口状态
netstat -tulpn | grep inspection_system

# 检查资源使用
top -p $(pgrep inspection_system)
```

### 2. 数据备份

```bash
# 备份配置
sudo tar czf /backup/inspection_config_$(date +%Y%m%d).tar.gz /etc/inspection

# 备份数据
sudo tar czf /backup/inspection_data_$(date +%Y%m%d).tar.gz /var/lib/inspection
```

### 3. 日志管理

```bash
# 配置日志轮转
sudo tee /etc/logrotate.d/inspection << EOF
/var/log/inspection/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0640 inspection inspection
    sharedscripts
    postrotate
        systemctl reload inspection
    endscript
}
EOF
```

## 故障处理

### 1. 常见问题

1. **服务无法启动**
   - 检查配置文件
   - 检查权限设置
   - 查看错误日志
   - 验证依赖项

2. **连接问题**
   - 检查网络配置
   - 验证防火墙规则
   - 测试网络连通性
   - 检查端口占用

3. **性能问题**
   - 检查资源使用
   - 优化配置参数
   - 分析系统日志
   - 监控系统负载

### 2. 故障恢复

```bash
# 1. 停止服务
sudo systemctl stop inspection

# 2. 备份数据
sudo tar czf /backup/inspection_backup_$(date +%Y%m%d).tar.gz \
    /etc/inspection \
    /var/lib/inspection

# 3. 检查并修复数据
sudo inspection_system --check-data
sudo inspection_system --repair-data

# 4. 重启服务
sudo systemctl start inspection
```

## 升级指南

### 1. 版本升级

```bash
# 1. 备份当前版本
sudo tar czf /backup/inspection_backup_$(date +%Y%m%d).tar.gz \
    /usr/local/bin/inspection_system \
    /etc/inspection \
    /var/lib/inspection

# 2. 停止服务
sudo systemctl stop inspection

# 3. 编译新版本
cd intelligent-inspection
git pull
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 4. 安装新版本
sudo make install

# 5. 更新配置
sudo inspection_system --update-config

# 6. 启动服务
sudo systemctl start inspection
```

### 2. 回滚操作

```bash
# 1. 停止服务
sudo systemctl stop inspection

# 2. 恢复备份
sudo tar xzf /backup/inspection_backup_*.tar.gz -C /

# 3. 启动服务
sudo systemctl start inspection
```

## 安全建议

1. **系统安全**
   - 及时更新系统
   - 最小权限原则
   - 定期安全审计
   - 启用防火墙

2. **数据安全**
   - 定期备份数据
   - 加密敏感信息
   - 访问控制
   - 日志监控

3. **网络安全**
   - 使用SSL/TLS
   - 限制访问来源
   - 监控网络流量
   - 定期安全扫描
