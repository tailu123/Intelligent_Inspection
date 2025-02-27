# 贡献指南

感谢您对机器狗导航控制系统的关注！我们欢迎任何形式的贡献，包括但不限于：代码提交、问题报告、功能建议、文档改进等。

## 如何贡献

### 1. 提交问题（Issue）

如果您发现了 bug 或有新功能建议，请先查看是否已有相关的 issue。如果没有，您可以创建新的 issue，并：

- 使用清晰的标题描述问题
- 详细描述问题或建议
- 如果是 bug，请提供：
  - 问题的复现步骤
  - 期望的行为
  - 实际的行为
  - 系统环境信息
  - 相关的日志输出

### 2. 提交代码（Pull Request）

1. Fork 本仓库
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的修改 (`git commit -m '添加某个特性'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 代码规范

我们使用以下工具确保代码质量：

- **静态代码分析**：使用 cppcheck
  ```bash
  ./scripts/static-check.sh
  ```

- **代码格式化**：使用 clang-format
  ```bash
  ./scripts/format-code.sh
  ```

请确保您的代码：
- 遵循 C++17 标准
- 通过所有静态检查
- 包含适当的单元测试
- 有清晰的注释和文档

### 提交信息规范

提交信息应该清晰描述改动，格式如下：

```
<类型>: <描述>

[可选的详细描述]

[可选的关闭 issue]
```

类型包括：
- feat: 新功能
- fix: 修复 bug
- docs: 文档更新
- style: 代码格式（不影响代码运行的变动）
- refactor: 重构（既不是新增功能，也不是修改 bug 的代码变动）
- perf: 性能优化
- test: 增加测试
- chore: 构建过程或辅助工具的变动

### 文档贡献

如果您想改进文档，请注意：
- 使用清晰、简洁的语言
- 提供具体的示例
- 保持文档结构的一致性
- 更新相关的 README 文件

## 开发环境设置

1. 安装依赖：
```bash
./scripts/install_dependencies.sh
```

2. 构建项目：
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

3. 运行测试：
```bash
ctest --output-on-failure
```

## 行为准则

请保持专业和友善。我们希望维护一个开放、包容的社区环境。

## 获取帮助

如果您在贡献过程中需要帮助：
- 查看现有的 issue 和文档
- 在 issue 中提问
- 通过邮件联系维护者

感谢您的贡献！
