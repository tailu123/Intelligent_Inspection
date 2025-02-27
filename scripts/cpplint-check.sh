#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查依赖
check_dependencies() {
    if ! command -v cpplint &> /dev/null; then
        echo -e "${RED}错误: cpplint 未安装${NC}"
        echo "请运行以下命令安装:"
        echo "pip install cpplint"
        exit 1
    fi
}

# 创建默认的 CPPLINT.cfg 配置文件
create_default_config() {
    local config_file="CPPLINT.cfg"
    if [ ! -f "$config_file" ]; then
        echo "# CPPLINT 配置文件
set noparent
filter=-build/include_order      # 不检查 include 顺序
filter=-build/namespaces        # 不检查 using namespace 语句
filter=-legal/copyright         # 不检查版权声明
filter=-runtime/references      # 不检查引用参数
filter=-build/include_subdir    # 不检查子目录包含
filter=-readability/casting     # 不检查类型转换
filter=-runtime/explicit        # 不检查 explicit 关键字
filter=-whitespace/braces      # 不检查大括号位置
filter=-whitespace/indent      # 不检查缩进
linelength=120                 # 设置行长度限制
root=.                         # 设置项目根目录" > "$config_file"
        echo -e "${BLUE}已创建默认 cpplint 配置文件: $config_file${NC}"
    fi
}

# 运行 cpplint 检查
run_cpplint() {
    local src_dir="$1"
    local include_dir="$2"
    local report_dir="$3"
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local report_file="$report_dir/cpplint_report_$timestamp.txt"
    local error_file="$report_dir/cpplint_error_$timestamp.txt"

    echo -e "${BLUE}正在运行 cpplint 检查...${NC}"

    # 查找所有 C++ 源文件和头文件
    local cpp_files=$(find "$src_dir" "$include_dir" -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.h" -o -name "*.hpp" \))

    # 运行 cpplint 检查
    echo -e "${BLUE}分析源代码...${NC}"
    for file in $cpp_files; do
        echo -e "${BLUE}检查文件: $file${NC}"
        cpplint "$file" 2>> "$report_file"
    done

    # 提取错误信息到单独的文件
    grep -i "error:" "$report_file" > "$error_file" || true
}

# 分析结果
analyze_results() {
    local report_file="$1"
    local error_count=0
    local warning_count=0
    local style_count=0

    echo -e "\n${BLUE}分析检查结果...${NC}"

    # 统计各类问题
    error_count=$(grep -c "error:" "$report_file" || true)
    warning_count=$(grep -c "warning:" "$report_file" || true)
    style_count=$(grep -c "Total errors found:" "$report_file" || true)

    # 提取总错误数
    local total_errors=0
    while read -r line; do
        if [[ $line =~ "Total errors found: "([0-9]+) ]]; then
            total_errors=$((total_errors + BASH_REMATCH[1]))
        fi
    done < "$report_file"

    # 打印统计结果
    echo -e "\n${BLUE}检查结果统计:${NC}"
    echo -e "${RED}错误: $error_count${NC}"
    echo -e "${YELLOW}警告: $warning_count${NC}"
    echo -e "${BLUE}代码风格问题: $total_errors${NC}"

    # 如果有错误或警告，显示详细信息
    if [ $total_errors -gt 0 ]; then
        echo -e "\n${YELLOW}问题详情:${NC}"
        grep -v "Done processing\|Total errors found:" "$report_file" || true
    fi

    # 返回总错误数
    return $total_errors
}

# 生成 HTML 报告
generate_html_report() {
    local report_file="$1"
    local html_report="${report_file%.txt}.html"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')

    echo -e "${BLUE}生成 HTML 报告...${NC}"

    # 创建 HTML 报告头部
    cat > "$html_report" << EOF
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Cpplint 检查报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; line-height: 1.6; }
        .error { color: #d73a49; background-color: #fff5f5; }
        .warning { color: #b08800; background-color: #fffbdd; }
        .style { color: #1b1f23; background-color: #f6f8fa; }
        .diagnostic { margin: 15px 0; padding: 15px; border: 1px solid #e1e4e8; border-radius: 6px; }
        .file-path { color: #586069; font-family: monospace; }
        .message { font-weight: 500; }
        h1 { color: #24292e; border-bottom: 1px solid #e1e4e8; padding-bottom: 0.3em; }
        .summary { background-color: #f6f8fa; padding: 15px; border-radius: 6px; margin: 20px 0; }
        .category { margin: 20px 0; }
        .category h2 { color: #24292e; }
        pre { white-space: pre-wrap; word-wrap: break-word; }
    </style>
</head>
<body>
    <h1>Cpplint 检查报告</h1>
    <div class="summary">
        <p>生成时间: $timestamp</p>
EOF

    # 添加统计信息
    local total_errors=0
    while read -r line; do
        if [[ $line =~ "Total errors found: "([0-9]+) ]]; then
            total_errors=$((total_errors + BASH_REMATCH[1]))
        fi
    done < "$report_file"

    echo "<p>发现问题总数: $total_errors</p>" >> "$html_report"
    echo "</div>" >> "$html_report"

    # 添加文件问题列表
    echo "<div class='category'><h2>详细问题列表</h2>" >> "$html_report"
    local current_file=""
    while IFS= read -r line; do
        if [[ $line == *"Processing"* ]] || [[ $line == *"Total errors found:"* ]]; then
            continue
        fi
        if [[ $line =~ ^[^:]+\.(cpp|cc|h|hpp): ]]; then
            if [[ $current_file != "" ]]; then
                echo "</div>" >> "$html_report"
            fi
            current_file=$line
            echo "<div class='diagnostic'>" >> "$html_report"
            echo "<h3 class='file-path'>$line</h3>" >> "$html_report"
        else
            echo "<pre>$line</pre>" >> "$html_report"
        fi
    done < "$report_file"

    if [[ $current_file != "" ]]; then
        echo "</div>" >> "$html_report"
    fi
    echo "</div>" >> "$html_report"

    # 添加 HTML 报告尾部
    echo "</body></html>" >> "$html_report"

    echo -e "${GREEN}HTML 报告已生成: $html_report${NC}"
}

# 主函数
main() {
    local workspace_dir=$(pwd)
    local src_dir="$workspace_dir/src"
    local include_dir="$workspace_dir/include"
    local report_dir="$workspace_dir/cpplint-reports"

    # 检查依赖
    check_dependencies

    # 创建报告目录
    mkdir -p "$report_dir"

    # 创建默认配置
    create_default_config

    # 运行检查
    run_cpplint "$src_dir" "$include_dir" "$report_dir"

    # 获取最新的报告文件
    local latest_report=$(ls -t "$report_dir"/cpplint_report_*.txt | head -1)

    # 分析结果
    analyze_results "$latest_report"
    local check_status=$?

    # 生成 HTML 报告
    generate_html_report "$latest_report"

    echo -e "\n${BLUE}检查报告已保存到:${NC}"
    echo "文本报告: $latest_report"
    echo "HTML报告: ${latest_report%.txt}.html"

    if [ $check_status -eq 0 ]; then
        echo -e "\n${GREEN}cpplint 检查完成，未发现问题。${NC}"
    else
        echo -e "\n${RED}cpplint 检查完成，发现 $check_status 个问题，请查看报告详情。${NC}"
        exit 1
    fi
}

# 执行主函数
main
