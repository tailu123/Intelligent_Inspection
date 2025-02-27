#!/bin/bash

# 显示当前工作目录
echo "当前工作目录: $(pwd)"

# 添加调试输出
echo "开始查找需要格式化的文件..."

# 先列出所有要处理的文件
find . \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" \) \
    -not -path "./build/*" \
    -not -path "./third_party/*" \
    -type f \
    -print

# 执行格式化
find . \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" \) \
    -not -path "./build/*" \
    -not -path "./third_party/*" \
    -type f \
    -exec echo "正在格式化: {}" \; \
    -exec clang-format -i {} \;

echo "代码格式化完成!"
