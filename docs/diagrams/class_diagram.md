# 系统组件类图

## 1. 整体类图结构

以下类图展示了机器狗导航控制系统的主要组件及其关系。

### 1.1 系统整体类图

```mermaid
classDiagram
    %% 应用层
    class Application {
        +initialize()
        +run()
        +shutdown()
    }

    class CommandLineInterface {
        -CommandProcessor processor
        +processInput(string)
        +displayOutput(string)
    }

    class CommandProcessor {
        -Map~string, Command~ commands
        +registerCommand(Command)
        +executeCommand(string, CommandArgs)
    }

    class Logger {
        -LogLevel level
        -string logFile
        +log(LogLevel, string)
        +setLogLevel(LogLevel)
    }

    class ConfigManager {
        -ConfigData config
        +loadConfig(string)
        +saveConfig(string)
        +getValue(string)
        +setValue(string, Value)
    }

    %% 业务层
    class NavStateMachine {
        -NavState currentState
        +processEvent(Event)
        +getCurrentState()
        +canTransition(NavState)
    }

    class NavController {
        -NavStateMachine stateMachine
        -PathPlanner pathPlanner
        +startNavigation(Target)
        +stopNavigation()
        +pauseNavigation()
        +resumeNavigation()
    }

    class PathPlanner {
        -Algorithm algorithm
        -Map map
        +planPath(Position, Position)
        +optimizePath(Path)
        +validatePath(Path)
    }

    class EventBus {
        -Map~EventType, List~EventListener~~ subscribers
        +subscribe(EventType, EventListener)
        +unsubscribe(EventType, EventListener)
        +publish(Event)
    }

    class MessageQueue {
        -Queue~Message~ queue
        +enqueue(Message)
        +dequeue() Message
    }

    %% 网络层
    class NetworkManager {
        -TcpClient client
        -ProtocolHandler protocolHandler
        +connect(string, int)
        +disconnect()
        +sendMessage(Message)
    }

    class TcpClient {
        -Socket socket
        -bool connected
        +connect(string, int)
        +disconnect()
        +send(bytes)
        +receive() bytes
    }

    class ProtocolHandler {
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message)
    }

    %% 关系
    Application --> CommandLineInterface
    Application --> NavController
    Application --> NetworkManager
    Application --> ConfigManager
    Application --> Logger

    CommandLineInterface --> CommandProcessor
    CommandProcessor --> NavController

    NavController --> NavStateMachine
    NavController --> PathPlanner
    NavController --> MessageQueue
    NavController --> EventBus

    NetworkManager --> TcpClient
    NetworkManager --> ProtocolHandler
    NetworkManager --> MessageQueue
```

## 2. 分层类图

系统分为三个主要层次，每个层次有各自的组件和职责。

### 2.1 应用层类图

```mermaid
classDiagram
    class Application {
        +initialize()
        +run()
        +shutdown()
    }

    class CommandLineInterface {
        -CommandProcessor processor
        -History history
        -Completer completer
        +processInput(string)
        +displayOutput(string)
        +showHelp()
    }

    class Command {
        <<interface>>
        +getName() string
        +getHelp() string
        +execute(CommandArgs) CommandResult
    }

    class CommandProcessor {
        -Map~string, Command~ commands
        +registerCommand(Command)
        +unregisterCommand(string)
        +executeCommand(string, CommandArgs)
        +getAvailableCommands() List~string~
    }

    class NavStartCommand {
        +getName() string
        +getHelp() string
        +execute(CommandArgs) CommandResult
    }

    class NavStopCommand {
        +getName() string
        +getHelp() string
        +execute(CommandArgs) CommandResult
    }

    class Logger {
        -LogLevel level
        -string logFile
        -LogWriter writer
        +log(LogLevel, string)
        +setLogLevel(LogLevel)
        +setLogFile(string)
        +flush()
    }

    class ConfigManager {
        -ConfigData config
        -ConfigValidator validator
        +loadConfig(string)
        +saveConfig(string)
        +getValue(string)
        +setValue(string, Value)
        +hasKey(string)
    }

    Application --> CommandLineInterface
    Application --> Logger
    Application --> ConfigManager

    CommandLineInterface --> CommandProcessor
    CommandProcessor --> Command

    Command <|-- NavStartCommand
    Command <|-- NavStopCommand
```

### 2.2 业务层类图

```mermaid
classDiagram
    class NavStateMachine {
        -NavState currentState
        -Map~NavState, Map~Event, NavState~~ transitions
        -Map~NavState, Action~ entryActions
        -Map~NavState, Action~ exitActions
        +processEvent(Event)
        +getCurrentState()
        +canTransition(NavState)
        +registerTransition(NavState, Event, NavState)
    }

    class NavState {
        <<enumeration>>
        INIT
        PREPARE_ENTER_NAV
        NAV
        DONE
        ERROR
    }

    class Event {
        +EventType type
        +EventId id
        +Timestamp timestamp
    }

    class NavEvent {
        +NavEventType type
        +string targetId
        +PathOptions options
    }

    class NavController {
        -NavStateMachine stateMachine
        -PathPlanner pathPlanner
        -StatusMonitor monitor
        +startNavigation(Target)
        +stopNavigation()
        +pauseNavigation()
        +resumeNavigation()
        +getStatus() NavStatus
    }

    class PathPlanner {
        -Algorithm algorithm
        -Map map
        -Constraints constraints
        +planPath(Position, Position)
        +optimizePath(Path)
        +validatePath(Path)
        +replanPath(Path, Obstacle)
    }

    class EventBus {
        -Map~EventType, List~EventListener~~ subscribers
        -Mutex mutex
        +subscribe(EventType, EventListener)
        +unsubscribe(EventType, EventListener)
        +publish(Event)
        +clear()
    }

    class EventListener {
        <<interface>>
        +onEvent(Event)
    }

    class Message {
        +MessageType type
        +MessageId id
        +Timestamp timestamp
    }

    class MessageQueue {
        -Queue~Message~ queue
        -Mutex mutex
        -ConditionVariable condition
        +enqueue(Message)
        +dequeue() Message
        +tryDequeue() Message
        +size() int
    }

    class MessageHandler {
        <<interface>>
        +handleMessage(Message)
    }

    NavStateMachine --> NavState
    NavStateMachine --> Event
    NavController --> NavStateMachine
    NavController --> PathPlanner

    Event <|-- NavEvent

    EventBus --> EventListener
    EventBus --> Event

    MessageQueue --> Message
    MessageHandler --> Message
```

### 2.3 网络层类图

```mermaid
classDiagram
    class NetworkManager {
        -TcpClient client
        -ProtocolHandler protocolHandler
        -ConnectionMonitor monitor
        +connect(string, int)
        +disconnect()
        +sendMessage(Message)
        +isConnected() bool
        +setReconnectStrategy(ReconnectStrategy)
    }

    class TcpClient {
        <<interface>>
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
    }

    class AsioTcpClient {
        -IoService ioService
        -Socket socket
        -bool connected
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
    }

    class EpollTcpClient {
        -int epollFd
        -Socket socket
        -bool connected
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
    }

    class ProtocolHandler {
        <<interface>>
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message) bool
    }

    class JsonProtocolHandler {
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message) bool
    }

    class BinaryProtocolHandler {
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message) bool
    }

    class ConnectionMonitor {
        -int checkInterval
        -int timeout
        -ConnectionState state
        +checkConnection()
        +onConnectionLost()
        +onConnectionRestored()
    }

    NetworkManager --> TcpClient
    NetworkManager --> ProtocolHandler
    NetworkManager --> ConnectionMonitor

    TcpClient <|-- AsioTcpClient
    TcpClient <|-- EpollTcpClient

    ProtocolHandler <|-- JsonProtocolHandler
    ProtocolHandler <|-- BinaryProtocolHandler
```

## 3. 核心组件详细设计

### 3.1 状态机类设计

状态机是系统的核心组件，负责管理系统状态转换。

```mermaid
classDiagram
    class NavStateMachine {
        -NavState currentState
        -Map~NavState, Map~Event, NavState~~ transitions
        -Map~NavState, Action~ entryActions
        -Map~NavState, Action~ exitActions
        -EventBus eventBus
        +NavStateMachine(EventBus)
        +processEvent(Event) bool
        +getCurrentState() NavState
        +canTransition(NavState) bool
        +registerTransition(NavState, Event, NavState)
        +registerEntryAction(NavState, Action)
        +registerExitAction(NavState, Action)
        -executeEntryAction(NavState)
        -executeExitAction(NavState)
        -notifyStateChange(NavState, NavState)
    }

    class BooostMsmStateMachine {
        -BackEnd backend
        -FrontEnd frontend
        +start()
        +process_event(Event)
        +state() NavState
    }

    class NavState {
        <<enumeration>>
        INIT
        PREPARE_ENTER_NAV
        NAV
        DONE
        ERROR
    }

    class Event {
        +EventType type
        +EventId id
        +Timestamp timestamp
        +getData() void*
    }

    class Action {
        <<interface>>
        +execute(Event, NavState)
    }

    class InitAction {
        +execute(Event, NavState)
    }

    class PrepareNavAction {
        +execute(Event, NavState)
    }

    class StartNavAction {
        +execute(Event, NavState)
    }

    class CompleteNavAction {
        +execute(Event, NavState)
    }

    NavStateMachine --> NavState
    NavStateMachine --> Event
    NavStateMachine --> Action
    NavStateMachine <|-- BooostMsmStateMachine

    Action <|-- InitAction
    Action <|-- PrepareNavAction
    Action <|-- StartNavAction
    Action <|-- CompleteNavAction
```

### 3.2 消息队列与事件总线类设计

消息队列和事件总线是系统的通信骨干，实现组件间的解耦。

```mermaid
classDiagram
    class Message {
        +MessageType type
        +MessageId id
        +Timestamp createdAt
        +getPriority() int
        +isExpired() bool
        +getPayload() void*
    }

    class CommandMessage {
        +string command
        +CommandArgs args
        +getCommand() string
        +getArgs() CommandArgs
    }

    class NavRequestMessage {
        +NavRequestType requestType
        +string targetId
        +PathOptions options
        +getRequestType() NavRequestType
        +getTargetId() string
    }

    class NavResponseMessage {
        +NavResponseType responseType
        +NavStatus status
        +ErrorCode errorCode
        +getResponseType() NavResponseType
        +getStatus() NavStatus
    }

    class StatusUpdateMessage {
        +NavState previousState
        +NavState currentState
        +string reason
        +getPreviousState() NavState
        +getCurrentState() NavState
    }

    class MessageQueue {
        -Queue~Message~ queue
        -Mutex mutex
        -ConditionVariable condition
        -int maxSize
        +MessageQueue(int maxSize)
        +enqueue(Message) bool
        +dequeue() Message
        +tryDequeue() Message
        +size() int
        +isEmpty() bool
        +isFull() bool
        +clear()
    }

    class PriorityMessageQueue {
        -PriorityQueue~Message~ queue
        +enqueue(Message, Priority) bool
        +dequeueHighestPriority() Message
    }

    class Event {
        +EventType type
        +EventId id
        +Timestamp timestamp
        +getData() void*
    }

    class NavStateChangeEvent {
        +NavState oldState
        +NavState newState
        +string reason
        +getOldState() NavState
        +getNewState() NavState
    }

    class CommandResultEvent {
        +string command
        +bool success
        +string result
        +getCommand() string
        +isSuccess() bool
    }

    class EventBus {
        -Map~EventType, List~EventListener~~ subscribers
        -Mutex mutex
        -bool active
        +EventBus()
        +subscribe(EventType, EventListener) SubscriptionId
        +unsubscribe(SubscriptionId)
        +publish(Event) int
        +clearSubscriptions()
        +start()
        +stop()
    }

    class EventListener {
        <<interface>>
        +onEvent(Event)
    }

    Message <|-- CommandMessage
    Message <|-- NavRequestMessage
    Message <|-- NavResponseMessage
    Message <|-- StatusUpdateMessage

    MessageQueue <|-- PriorityMessageQueue

    Event <|-- NavStateChangeEvent
    Event <|-- CommandResultEvent

    EventBus --> EventListener
    EventBus --> Event
```

### 3.3 网络通信类设计

网络通信模块负责与机器狗设备的数据交换。

```mermaid
classDiagram
    class NetworkManager {
        -TcpClient client
        -ProtocolHandler protocolHandler
        -ConnectionMonitor monitor
        -bool autoReconnect
        -int reconnectInterval
        -EventBus eventBus
        +NetworkManager(TcpClient, ProtocolHandler)
        +connect(string, int) bool
        +disconnect()
        +sendMessage(Message) bool
        +isConnected() bool
        +setAutoReconnect(bool, int)
        +start()
        +stop()
        -onMessageReceived(Message)
        -reconnect()
    }

    class TcpClient {
        <<interface>>
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
        +setReceiveCallback(Callback)
    }

    class AsioTcpClient {
        -IoService ioService
        -Socket socket
        -Strand strand
        -bool connected
        -ReceiveCallback callback
        -Mutex mutex
        +AsioTcpClient()
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
        +setReceiveCallback(Callback)
        -handleConnect(Error)
        -handleSend(Error, size_t)
        -handleReceive(Error, size_t)
        -startReceive()
    }

    class EpollTcpClient {
        -int epollFd
        -Socket socket
        -bool connected
        -ReceiveCallback callback
        -Thread eventLoop
        +EpollTcpClient()
        +connect(string, int) bool
        +disconnect()
        +send(bytes) bool
        +receive() bytes
        +isConnected() bool
        +setReceiveCallback(Callback)
        -eventLoopFunc()
        -handleEvent(Event)
    }

    class ProtocolHandler {
        <<interface>>
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message) bool
    }

    class JsonProtocolHandler {
        -JsonSerializer serializer
        -JsonValidator validator
        +JsonProtocolHandler()
        +serialize(Message) bytes
        +deserialize(bytes) Message
        +validateMessage(Message) bool
        -formatJson(Json) string
        -parseJson(string) Json
    }

    class ConnectionMonitor {
        -int checkInterval
        -int timeout
        -ConnectionState state
        -EventBus eventBus
        -Thread monitorThread
        +ConnectionMonitor(int, int, EventBus)
        +start()
        +stop()
        +getState() ConnectionState
        -monitorFunc()
        -checkConnection()
        -notifyConnectionChange(ConnectionState)
    }

    NetworkManager --> TcpClient
    NetworkManager --> ProtocolHandler
    NetworkManager --> ConnectionMonitor

    TcpClient <|-- AsioTcpClient
    TcpClient <|-- EpollTcpClient

    ProtocolHandler <|-- JsonProtocolHandler
```

## 4. 跨层交互类图

展示三个层次的组件如何协同工作。

```mermaid
classDiagram
    %% 跨层组件
    class MessageQueue {
        +enqueue(Message)
        +dequeue() Message
    }

    class EventBus {
        +subscribe(EventType, EventListener)
        +publish(Event)
    }

    %% 应用层组件
    class CommandProcessor {
        +executeCommand(string, CommandArgs)
    }

    class CommandResultListener {
        +onEvent(CommandResultEvent)
    }

    %% 业务层组件
    class NavController {
        +startNavigation(Target)
        +getStatus() NavStatus
    }

    class NavCommandHandler {
        +handleMessage(CommandMessage)
    }

    class NavStateMachine {
        +processEvent(Event)
        +getCurrentState()
    }

    %% 网络层组件
    class NetworkManager {
        +sendMessage(Message)
    }

    class NetworkMessageHandler {
        +handleMessage(NavRequestMessage)
    }

    %% 关系
    CommandProcessor --> MessageQueue: 放入命令消息
    CommandProcessor --> EventBus: 订阅命令结果
    CommandResultListener --> CommandProcessor: 返回命令结果

    NavCommandHandler --> MessageQueue: 从队列获取命令
    NavCommandHandler --> NavController: 调用导航控制
    NavController --> NavStateMachine: 更新状态
    NavController --> MessageQueue: 放入导航请求

    NetworkMessageHandler --> MessageQueue: 从队列获取请求
    NetworkMessageHandler --> NetworkManager: 发送网络消息
    NetworkManager --> MessageQueue: 放入响应消息

    EventBus --> CommandResultListener: 通知命令结果
    EventBus --> NavController: 通知状态变更
```

## 5. 设计模式应用

系统中使用了多种设计模式，提高了代码的可维护性和扩展性。

### 5.1 观察者模式（事件总线）

```mermaid
classDiagram
    class Subject {
        +attach(Observer)
        +detach(Observer)
        +notify()
    }

    class Observer {
        <<interface>>
        +update(Subject)
    }

    class EventBus {
        -Map subscribers
        +subscribe(EventType, EventListener)
        +unsubscribe(EventType, EventListener)
        +publish(Event)
    }

    class EventListener {
        <<interface>>
        +onEvent(Event)
    }

    class NavStateChangeListener {
        +onEvent(NavStateChangeEvent)
    }

    class CommandResultListener {
        +onEvent(CommandResultEvent)
    }

    Subject <|-- EventBus
    Observer <|-- EventListener
    EventListener <|.. NavStateChangeListener
    EventListener <|.. CommandResultListener
    Subject --> Observer
```

### 5.2 命令模式（命令处理器）

```mermaid
classDiagram
    class Command {
        <<interface>>
        +execute()
    }

    class Invoker {
        -Command command
        +setCommand(Command)
        +executeCommand()
    }

    class ConcreteCommand {
        -Receiver receiver
        +execute()
    }

    class Receiver {
        +action()
    }

    class CommandProcessor {
        -Map commands
        +registerCommand(Command)
        +executeCommand(string, args)
    }

    class NavCommand {
        <<interface>>
        +getName() string
        +execute(args) result
    }

    class StartNavCommand {
        -NavController controller
        +execute(args)
    }

    class StopNavCommand {
        -NavController controller
        +execute(args)
    }

    class NavController {
        +startNavigation()
        +stopNavigation()
    }

    Command <|-- ConcreteCommand
    Invoker --> Command
    ConcreteCommand --> Receiver

    Invoker <|-- CommandProcessor
    Command <|-- NavCommand
    NavCommand <|.. StartNavCommand
    NavCommand <|.. StopNavCommand
    StartNavCommand --> NavController
    StopNavCommand --> NavController
```

### 5.3 状态模式（导航状态机）

```mermaid
classDiagram
    class Context {
        -State state
        +setState(State)
        +request()
    }

    class State {
        <<interface>>
        +handle(Context)
    }

    class ConcreteStateA {
        +handle(Context)
    }

    class ConcreteStateB {
        +handle(Context)
    }

    class NavStateMachine {
        -NavState currentState
        +processEvent(Event)
    }

    class NavState {
        <<interface>>
        +onEntry(Event)
        +onExit(Event)
        +canHandle(Event) bool
    }

    class InitState {
        +onEntry(Event)
        +onExit(Event)
        +canHandle(Event) bool
    }

    class NavState {
        +onEntry(Event)
        +onExit(Event)
        +canHandle(Event) bool
    }

    Context --> State
    State <|.. ConcreteStateA
    State <|.. ConcreteStateB

    Context <|-- NavStateMachine
    State <|-- NavState
    NavState <|.. InitState
    NavState <|.. NavState
```

### 5.4 策略模式（路径规划算法）

```mermaid
classDiagram
    class Context {
        -Strategy strategy
        +setStrategy(Strategy)
        +executeStrategy()
    }

    class Strategy {
        <<interface>>
        +execute()
    }

    class ConcreteStrategyA {
        +execute()
    }

    class ConcreteStrategyB {
        +execute()
    }

    class PathPlanner {
        -PlanningAlgorithm algorithm
        +setAlgorithm(PlanningAlgorithm)
        +planPath(start, end)
    }

    class PlanningAlgorithm {
        <<interface>>
        +plan(start, end) Path
    }

    class AStarAlgorithm {
        +plan(start, end) Path
    }

    class DijkstraAlgorithm {
        +plan(start, end) Path
    }

    Context --> Strategy
    Strategy <|.. ConcreteStrategyA
    Strategy <|.. ConcreteStrategyB

    Context <|-- PathPlanner
    Strategy <|-- PlanningAlgorithm
    PlanningAlgorithm <|.. AStarAlgorithm
    PlanningAlgorithm <|.. DijkstraAlgorithm
```

## 6. 类设计原则

系统设计遵循了以下面向对象设计原则：

### 6.1 单一职责原则 (SRP)

每个类都有明确的单一职责：
- `NavController` - 仅负责导航控制逻辑
- `NetworkManager` - 仅负责网络通信
- `Logger` - 仅负责日志记录

### 6.2 开放封闭原则 (OCP)

系统设计对扩展开放，对修改封闭：
- 通过接口和抽象类允许不同实现（如`TcpClient`接口）
- 使用策略模式实现算法替换（如`PlanningAlgorithm`）
- 命令模式允许添加新命令而不修改命令处理器

### 6.3 依赖倒置原则 (DIP)

高层模块不依赖低层模块，而是依赖抽象：
- `NavController`依赖`PathPlanner`接口而非具体实现
- `NetworkManager`依赖`TcpClient`接口而非具体的网络实现
- `EventBus`依赖`EventListener`接口而非具体订阅者

### 6.4 接口隔离原则 (ISP)

使用细粒度的专用接口而非宽泛的通用接口：
- 消息处理使用不同的处理器接口
- 事件监听使用专门的事件监听器
- 命令执行使用特定的命令接口

### 6.5 组合优于继承

系统优先使用组合而非继承：
- `NavController`组合了`NavStateMachine`而非继承
- `NetworkManager`组合了`TcpClient`和`ProtocolHandler`
- `CommandProcessor`组合了多个`Command`对象
