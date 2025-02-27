#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查 cppcheck 是否安装
check_dependencies() {
    if ! command -v cppcheck &> /dev/null; then
        echo -e "${RED}错误: cppcheck 未安装${NC}"
        echo "请运行以下命令安装:"
        echo "Ubuntu: sudo apt-get install cppcheck"
        echo "CentOS: sudo yum install cppcheck"
        exit 1
    fi
}

# 运行 cppcheck
run_cppcheck() {
    local src_dir="$1"
    local include_dir="$2"
    local report_file="$3"
    local xml_report="$4"

    echo -e "${BLUE}正在运行 cppcheck 检查...${NC}"

    # 运行 cppcheck 并生成 XML 报告
    cppcheck --enable=all \
             --std=c++17 \
             --suppress=missingIncludeSystem \
             -I "$include_dir" \
             --xml \
             --xml-version=2 \
             "$src_dir" 2> "$xml_report"

    # 运行 cppcheck 并生成人类可读的报告
    cppcheck --enable=all \
             --std=c++17 \
             --suppress=missingIncludeSystem \
             -I "$include_dir" \
             "$src_dir" 2> "$report_file"
}

# 分析结果
analyze_results() {
    local report_file="$1"
    local error_count=0
    local warning_count=0
    local style_count=0
    local performance_count=0

    echo -e "\n${BLUE}分析检查结果...${NC}"

    # 统计各类问题
    while IFS= read -r line; do
        if [[ $line == *"error:"* ]]; then
            ((error_count++))
        elif [[ $line == *"warning:"* ]]; then
            ((warning_count++))
        elif [[ $line == *"style:"* ]]; then
            ((style_count++))
        elif [[ $line == *"performance:"* ]]; then
            ((performance_count++))
        fi
    done < "$report_file"

    # 打印统计结果
    echo -e "\n${BLUE}检查结果统计:${NC}"
    echo -e "${RED}错误: $error_count${NC}"
    echo -e "${YELLOW}警告: $warning_count${NC}"
    echo -e "${BLUE}代码风格问题: $style_count${NC}"
    echo -e "${GREEN}性能优化建议: $performance_count${NC}"

    # 如果有错误或警告，显示详细信息
    if [ $error_count -gt 0 ] || [ $warning_count -gt 0 ]; then
        echo -e "\n${YELLOW}重要问题详情:${NC}"
        grep -E "error:|warning:" "$report_file"
    fi

    # 显示性能优化建议
    if [ $performance_count -gt 0 ]; then
        echo -e "\n${GREEN}性能优化建议:${NC}"
        grep "performance:" "$report_file"
    fi

    # 返回是否有错误
    return $error_count
}

# 主函数
main() {
    local workspace_dir=$(pwd)
    local src_dir="$workspace_dir/src"
    local include_dir="$workspace_dir/include"
    local report_dir="$workspace_dir/cppcheck-reports"
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local report_file="$report_dir/cppcheck_report_$timestamp.txt"
    local xml_report="$report_dir/cppcheck_report_$timestamp.xml"

    # 检查依赖
    check_dependencies

    # 创建报告目录
    mkdir -p "$report_dir"

    # 运行检查
    run_cppcheck "$src_dir" "$include_dir" "$report_file" "$xml_report"

    # 分析结果
    analyze_results "$report_file"
    local check_status=$?

    echo -e "\n${BLUE}检查报告已保存到:${NC}"
    echo "文本报告: $report_file"
    echo "XML报告: $xml_report"

    if [ $check_status -eq 0 ]; then
        echo -e "\n${GREEN}静态代码检查完成，未发现严重问题。${NC}"
    else
        echo -e "\n${RED}静态代码检查完成，发现 $check_status 个错误，请查看报告详情。${NC}"
        exit 1
    fi
}

# 执行主函数
main
