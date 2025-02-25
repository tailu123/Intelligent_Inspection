# 测试规范

## 测试原则

1. **完整性**
   - 功能测试覆盖
   - 边界条件测试
   - 错误处理测试
   - 性能测试

2. **独立性**
   - 测试用例相互独立
   - 避免测试间依赖
   - 每个测试专注一个功能
   - 测试环境隔离

3. **可维护性**
   - 测试代码清晰
   - 测试结构合理
   - 避免重复代码
   - 易于更新维护

## 单元测试

### 1. 测试框架

```cpp
// 使用Google Test框架
#include <gtest/gtest.h>

class NetworkModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前初始化
        network_model_ = std::make_unique<NetworkModel>();
    }

    void TearDown() override {
        // 测试后清理
        network_model_.reset();
    }

    std::unique_ptr<NetworkModel> network_model_;
};
```

### 2. 测试用例编写

```cpp
TEST_F(NetworkModelTest, ConnectSuccess) {
    // 准备测试数据
    const std::string host = "localhost";
    const uint16_t port = 8080;

    // 执行测试
    bool result = network_model_->connect(host, port);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(network_model_->isConnected());
}

TEST_F(NetworkModelTest, ConnectFailure) {
    // 准备测试数据
    const std::string host = "invalid_host";
    const uint16_t port = 8080;

    // 执行测试
    bool result = network_model_->connect(host, port);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(network_model_->isConnected());
}
```

### 3. 模拟对象

```cpp
class MockNetworkModel : public NetworkModel {
public:
    MOCK_METHOD(bool, connect, (const std::string& host, uint16_t port), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
};

TEST(ConnectionManagerTest, HandleConnectionFailure) {
    // 创建模拟对象
    MockNetworkModel mock_model;
    ConnectionManager manager(&mock_model);

    // 设置期望
    EXPECT_CALL(mock_model, connect(_, _))
        .WillOnce(Return(false));

    // 执行测试
    bool result = manager.establishConnection();

    // 验证结果
    EXPECT_FALSE(result);
}
```

## 集成测试

### 1. 测试配置

```cpp
class IntegrationTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // 初始化测试环境
        initializeTestEnvironment();
    }

    static void TearDownTestSuite() {
        // 清理测试环境
        cleanupTestEnvironment();
    }

    void SetUp() override {
        // 每个测试前的准备
        system_ = std::make_unique<InspectionSystem>();
        system_->initialize();
    }

    void TearDown() override {
        // 每个测试后的清理
        system_->shutdown();
        system_.reset();
    }

    std::unique_ptr<InspectionSystem> system_;
};
```

### 2. 测试场景

```cpp
TEST_F(IntegrationTest, CompleteWorkflow) {
    // 1. 系统启动
    ASSERT_TRUE(system_->start());

    // 2. 创建任务
    Task task = createTestTask();
    ASSERT_TRUE(system_->submitTask(task));

    // 3. 执行任务
    EXPECT_TRUE(system_->executeTask(task.getId()));

    // 4. 验证结果
    TaskResult result = system_->getTaskResult(task.getId());
    EXPECT_EQ(result.status, TaskStatus::COMPLETED);
}
```

## 性能测试

### 1. 基准测试

```cpp
#include <benchmark/benchmark.h>

static void BM_NetworkTransfer(benchmark::State& state) {
    NetworkModel model;
    const std::vector<uint8_t> data(state.range(0));

    for (auto _ : state) {
        model.send(data);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                           int64_t(state.range(0)));
}

BENCHMARK(BM_NetworkTransfer)
    ->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);
```

### 2. 负载测试

```cpp
class LoadTest : public ::testing::Test {
protected:
    void TestConcurrentConnections(int num_connections) {
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};

        // 创建多个并发连接
        for (int i = 0; i < num_connections; ++i) {
            threads.emplace_back([&]() {
                if (createConnection()) {
                    ++success_count;
                }
            });
        }

        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }

        // 验证结果
        EXPECT_GE(success_count, num_connections * 0.9);
    }
};
```

## 测试覆盖率

### 1. 覆盖率配置

```cmake
# CMakeLists.txt
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()
```

### 2. 覆盖率报告

```bash
# 生成覆盖率数据
./run_tests

# 生成覆盖率报告
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## 持续集成测试

### 1. Jenkins配置

```groovy
pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh 'cmake -B build -DBUILD_TESTING=ON'
                sh 'cmake --build build'
            }
        }
        stage('Test') {
            steps {
                sh '''
                    cd build
                    ctest --output-on-failure
                    ./run_benchmarks
                '''
            }
            post {
                always {
                    junit 'build/test_results/*.xml'
                    publishHTML(target: [
                        reportDir: 'build/coverage',
                        reportFiles: 'index.html',
                        reportName: 'Coverage Report'
                    ])
                }
            }
        }
    }
}
```

### 2. GitHub Actions配置

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2

    - name: Configure
      run: cmake -B build -DBUILD_TESTING=ON

    - name: Build
      run: cmake --build build

    - name: Test
      run: |
        cd build
        ctest --output-on-failure
        ./run_benchmarks

    - name: Upload Coverage
      uses: codecov/codecov-action@v1
```

## 测试最佳实践

1. **测试设计**
   - 遵循FIRST原则（Fast, Independent, Repeatable, Self-validating, Timely）
   - 一个测试只测试一个功能点
   - 测试用例命名清晰明确
   - 适当使用测试夹具

2. **测试数据**
   - 使用合适的测试数据
   - 考虑边界条件
   - 包含正常和异常情况
   - 避免硬编码测试数据

3. **测试维护**
   - 定期更新测试用例
   - 删除过时的测试
   - 保持测试代码整洁
   - 持续监控测试覆盖率

4. **测试自动化**
   - 集成持续集成系统
   - 自动运行测试套件
   - 自动生成测试报告
   - 配置测试告警机制
