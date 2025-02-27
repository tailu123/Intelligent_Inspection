#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查依赖
check_dependencies() {
    if ! command -v clang-tidy &> /dev/null; then
        echo -e "${RED}错误: clang-tidy 未安装${NC}"
        echo "请运行以下命令安装:"
        echo "Ubuntu: sudo apt-get install clang-tidy"
        echo "CentOS: sudo yum install clang-tidy"
        exit 1
    fi

    if ! command -v jq &> /dev/null; then
        echo -e "${YELLOW}警告: jq 未安装，JSON 格式化将不可用${NC}"
    fi
}

# 创建默认的 clang-tidy 配置文件
create_default_config() {
    local config_file=".clang-tidy"
    if [ ! -f "$config_file" ]; then
        echo "---
Checks: '
  *,
  -fuchsia-*,
  -google-*,
  -zircon-*,
  -abseil-*,
  -modernize-use-trailing-return-type,
  -llvm-*,
  -llvmlibc-*
'
WarningsAsErrors: ''
HeaderFilterRegex: ''
AnalyzeTemporaryDtors: false
FormatStyle: none
CheckOptions:
  - key: readability-identifier-length.MinimumVariableNameLength
    value: 2
  - key: readability-identifier-length.MinimumParameterNameLength
    value: 2
" > "$config_file"
        echo -e "${BLUE}已创建默认 clang-tidy 配置文件: $config_file${NC}"
    fi
}

# 运行 clang-tidy 检查
run_clang_tidy() {
    local src_dir="$1"
    local build_dir="$2"
    local report_dir="$3"
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local report_file="$report_dir/clang_tidy_report_$timestamp.txt"
    local json_report="$report_dir/clang_tidy_report_$timestamp.json"

    echo -e "${BLUE}正在运行 clang-tidy 检查...${NC}"

    # 确保构建目录存在编译命令数据库
    if [ ! -f "$build_dir/compile_commands.json" ]; then
        echo -e "${RED}错误: 未找到 compile_commands.json${NC}"
        echo "请确保在构建时启用了 CMAKE_EXPORT_COMPILE_COMMANDS=ON"
        exit 1
    fi

    # 查找所有 C++ 源文件
    local cpp_files=$(find "$src_dir" -type f -name "*.cpp" -o -name "*.cc" -o -name "*.cxx")

    # 运行 clang-tidy 检查
    echo -e "${BLUE}分析源代码...${NC}"
    for file in $cpp_files; do
        echo -e "${BLUE}检查文件: $file${NC}"
        clang-tidy "$file" \
            -p="$build_dir" \
            --quiet \
            --export-fixes="$json_report" \
            2>> "$report_file"
    done
}

# 分析结果
analyze_results() {
    local report_file="$1"
    local error_count=0
    local warning_count=0
    local note_count=0

    echo -e "\n${BLUE}分析检查结果...${NC}"

    # 统计各类问题
    error_count=$(grep -c "error:" "$report_file" || true)
    warning_count=$(grep -c "warning:" "$report_file" || true)
    note_count=$(grep -c "note:" "$report_file" || true)

    # 打印统计结果
    echo -e "\n${BLUE}检查结果统计:${NC}"
    echo -e "${RED}错误: $error_count${NC}"
    echo -e "${YELLOW}警告: $warning_count${NC}"
    echo -e "${BLUE}提示: $note_count${NC}"

    # 如果有错误或警告，显示详细信息
    if [ $error_count -gt 0 ] || [ $warning_count -gt 0 ]; then
        echo -e "\n${YELLOW}重要问题详情:${NC}"
        grep -E "error:|warning:" "$report_file" || true
    fi

    # 返回是否有错误
    return $error_count
}

# 生成 HTML 报告
generate_html_report() {
    local json_report="$1"
    local html_report="${json_report%.json}.html"

    if command -v jq &> /dev/null; then
        echo -e "${BLUE}生成 HTML 报告...${NC}"

        # 创建 HTML 报告头部
        cat > "$html_report" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Clang-Tidy 检查报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .error { color: red; }
        .warning { color: orange; }
        .note { color: blue; }
        .diagnostic { margin: 10px 0; padding: 10px; border: 1px solid #ddd; }
    </style>
</head>
<body>
    <h1>Clang-Tidy 检查报告</h1>
    <p>生成时间: $(date)</p>
EOF

        # 转换 JSON 到 HTML
        if [ -f "$json_report" ]; then
            jq -r '.[] | "<div class=\"diagnostic\">
                <h3>" + .DiagnosticName + "</h3>
                <p>" + .Message + "</p>
                <p>文件: " + .FilePath + ":" + (.FileOffset | tostring) + "</p>
                </div>"' "$json_report" >> "$html_report"
        fi

        # 添加 HTML 报告尾部
        echo "</body></html>" >> "$html_report"

        echo -e "${GREEN}HTML 报告已生成: $html_report${NC}"
    else
        echo -e "${YELLOW}警告: 未安装 jq，无法生成 HTML 报告${NC}"
    fi
}

# 主函数
main() {
    local workspace_dir=$(pwd)
    local src_dir="$workspace_dir/src"
    local build_dir="$workspace_dir/build"
    local report_dir="$workspace_dir/clang-tidy-reports"

    # 检查依赖
    check_dependencies

    # 创建报告目录
    mkdir -p "$report_dir"

    # 创建默认配置
    create_default_config

    # 运行检查
    run_clang_tidy "$src_dir" "$build_dir" "$report_dir"

    # 获取最新的报告文件
    local latest_report=$(ls -t "$report_dir"/clang_tidy_report_*.txt | head -1)
    local latest_json=$(ls -t "$report_dir"/clang_tidy_report_*.json | head -1)

    # 分析结果
    analyze_results "$latest_report"
    local check_status=$?

    # 生成 HTML 报告
    generate_html_report "$latest_json"

    echo -e "\n${BLUE}检查报告已保存到:${NC}"
    echo "文本报告: $latest_report"
    echo "JSON报告: $latest_json"

    if [ $check_status -eq 0 ]; then
        echo -e "\n${GREEN}clang-tidy 检查完成，未发现严重问题。${NC}"
    else
        echo -e "\n${RED}clang-tidy 检查完成，发现 $check_status 个错误，请查看报告详情。${NC}"
        exit 1
    fi
}

# 执行主函数
main
