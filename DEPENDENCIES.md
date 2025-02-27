# 项目依赖说明

本项目依赖以下库，请确保它们已经正确安装：

## 必需依赖

1. **Boost** (>= 1.71.0)
   - system
   - filesystem
   - thread

2. **spdlog** (>= 1.5.0)
   - 日志库

3. **nlohmann_json** (>= 3.7.0)
   - JSON 处理库

4. **RapidXML** (>= 1.13)
   - XML 解析库

## 快速安装

### Ubuntu/Debian

```bash
# 使用脚本安装
./scripts/install_dependencies.sh

# 或手动安装
sudo apt-get update
sudo apt-get install -y \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libspdlog-dev \
    nlohmann-json3-dev \
    rapidxml-dev
```

### CentOS/RHEL

```bash
# 使用脚本安装
./scripts/install_dependencies.sh

# 或手动安装
sudo yum install -y \
    boost-devel \
    spdlog-devel \
    nlohmann-json-devel \
    rapidxml-devel
```

## 手动安装

如果您的系统包管理器中没有这些包，您可以从源代码安装：

### RapidXML
```bash
wget https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip
unzip rapidxml-1.13.zip
sudo mkdir -p /usr/include/rapidxml
sudo cp rapidxml-1.13/*.hpp /usr/include/rapidxml/
```

## 验证安装

您可以运行以下命令来验证依赖是否正确安装：

```bash
mkdir build && cd build
cmake ..
```

如果没有错误信息，说明所有依赖都已正确安装。
