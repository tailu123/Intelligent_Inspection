1003协议改成交互式，表示收到了，回答接受/拒绝
1004协议改成交互式，表示收到了，回答接受/拒绝
1007协议提高反馈值，客户可以专注于 1007 协议的反馈值进行增加逻辑

既然 1007 是定时一定频率获取的，是不是没必要通过 req/resp 的方式，pub/sub 的方式更好？

include/
├── app/                            # 应用层（原application中的系统入口）
│   └── x30_inspection_system.hpp   # 系统主入口
│
├── common/                         # 通用基础组件
│   ├── event/
│   │   └── event_bus.hpp          # 从application移出的事件总线
│   └── message/
│       └── message_queue.hpp       # 从application移出的消息队列
│
├── procedure/                      # 流程控制层（新增）
│   ├── base_procedure.hpp          # 流程基类
│   └── nav_procedure/             # 导航流程（从application移出）
│       ├── nav_procedure.hpp       # 导航流程实现
│       └── nav_context.hpp         # 导航上下文
│
├── state/                         # 状态管理层（保持但优化）
│   └── nav/                      # 导航状态相关
│       ├── nav_state_machine.hpp  # 导航状态机
│       ├── nav_states.hpp         # 状态定义
│       └── nav_actions.hpp        # 动作定义
│
├── network/                       # 网络层（整合原communication和protocol）
│   ├── communication/            # 通信实现
│   │   └── x30_communication.hpp
│   └── protocol/                # 协议定义
│       └── x30_protocol.hpp
