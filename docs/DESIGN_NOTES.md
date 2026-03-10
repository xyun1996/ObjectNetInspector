# DESIGN NOTES - ObjectNetInspector

更新时间：2026-03-10（Asia/Shanghai）

## 1. 目标与边界
- 目标：在 Unreal Insights 中提供“对象级”网络事件分析视角，支持按对象快速定位高频 RPC/属性同步热点。
- 边界：只做 trace 可归因 payload 的统计与比较，不承诺网卡层精确流量。

## 2. 核心数据口径
- 对象流量：与对象关联事件中的可归因 `BitCount` 聚合。
- 若 `BitCount` 缺失：
  - 事件仍参与列表与计数
  - UI 显示 `N/A`
  - 聚合仅累计已知 bits

## 3. 架构分层
- `FObjectNetTraceReader`：读取入口，负责 session 读取与 mock 回退。
- `FObjectNetInsightsBridge`：Unreal Insights API 适配边界，隔离 UE 版本差异。
- `FObjectNetAnalyzer`：事件标准化、索引构建（对象/连接/包）。
- `FObjectNetAggregator`：按 Query 做对象聚合统计。
- `FObjectNetProvider`：面向 UI 的协调层（刷新、筛选、选中态、数据源状态）。
- Slate UI：Toolbar + ObjectList + Detail + EventTable。

补充约定（2026-03-10）：
- `TraceReader` 本身不直接依赖具体 Insights 提取 API，只消费可注入的 `SessionReader`；真实会话提取逻辑放在 bridge。
- 当会话读取失败时输出明确日志（是否绑定 reader、模块加载状态），便于定位为何回退到 mock。

## 4. UE5.7 事件映射方案
- 数据来源：`INetProfilerProvider`
  - `ReadNames` 生成 name 索引
  - `ReadEventTypes` 生成 event type 索引
  - `ReadObjects` 构建 object 元信息
  - `ReadConnections + EnumeratePackets + EnumeratePacketContentEventsByIndex` 遍历内容事件
- 字段映射：
  - `ObjectId`：优先 `NetObjectId`，缺失回退为 `GameInstanceIndex:ObjectIndex` 组合键
  - `ObjectName`：优先 name 表，缺失回退 `Object_<index>`；解析时会清理首尾空白/外层引号，并从 path-like 名称提取叶子名
  - `ClassName`：优先对象 `TypeName`（先做规范化，支持 `Class'/Script/...'` 与脚本路径名提取短类名，并剥离 `REINST_/SKEL_/TRASHCLASS_` 等临时前缀）；缺失时尝试从对象名推断（如 `BP_PlayerCharacter_C_0 -> BP_PlayerCharacter_C`）；仍失败再回退 `TypeId_0x...`
  - `ObjectPath` 识别策略：除 `/`、`:` 外，补充 dot-only UE 对象路径识别（如 `BP_Weapon.BP_Weapon_C_1`）
  - `TimeSec`：`Packet.TimeStamp`
  - `ConnectionId`：`Connection.ConnectionId`
  - `PacketId`：`SequenceNumber + 1`（避免 0）
  - `BitCount`：`EndPos - StartPos`（仅 End>Start）
  - 观测指标：bridge 日志输出 `UnknownRatio` 与 `ClassName` 来源分布（TypeName / Inferred / TypeIdFallback）；当映射事件数较大且 `Unknown` 占比过高时输出告警，辅助分类器迭代。

## 5. Kind 归因策略（当前）
- 已抽离到 `FObjectNetEventClassifier`，避免 bridge 内部硬编码导致不可测。
- 分类规则：
  - RPC 关键词：`rpc/function/remotefunction/callremote/sendrpc/netmulticast`
  - Property 关键词：`property/replayout/repstate/rep/pushmodel/changelist/netserialize/iris/fragment/descriptor/state/delta/array/serializer`
  - 弱信号阈值：单侧评分小于 2 时不直接定类（避免 `remote/state` 等弱词导致误判）
  - 混合冲突回退：同时出现 `rep` 与 `rpc/function` 且双方都非强信号时，回退 `Unknown`（例如 `ServerRepFunction`）
  - 关键词都未命中时，按 `EventTypeLevel`/`ContentLevel` 做 Property 回退
  - 仍无信号时归为 `Unknown`

## 6. 验证
- 已新增 `FObjectNetMetadataParser`，集中处理对象名/路径拆分与类名启发式推断，并配套回归测试。
- 已修复聚合阶段搜索匹配口径：除对象字段外，也匹配 `RpcCounts/PropertyCounts` 中的事件名键，保证按事件名搜索不会在 aggregate 过滤阶段丢失结果。
- 自动化测试：
  - `ObjectNetInspector.Provider.FilteringAndAggregation`
  - `ObjectNetInspector.Provider.SearchFields`
  - `ObjectNetInspector.Classifier.KindInference`
  - `ObjectNetInspector.MetadataParser.ObjectNamePath`
- 标准执行入口：`scripts/Run-ObjectNetTests.ps1`（说明见 `docs/TESTING.md`）。

## 7. 当前剩余工作
1. 继续提升 `Kind` 归因准确率（降低 Unknown 与误判）
2. 接入真实 `ClassName/ObjectPath`（若 provider 可提供）
3. 已在 UE5.7 接入 Workspace Profiling 分类，后续做跨版本验证

## 8. 文档维护约定
- 每次做结构性改动（接口、口径、流程、取舍）时，同步更新本文件。
- 每次迭代结果与待办同步到 `docs/WORKLOG.md`。

## 9. 2026-03-10 归因强化补充
- `Kind` 分类新增信号词：`processremotefunction / dispatchrpc / serializeproperty / netfield / retirement / lifetime / replicationcondition`。
- 将 `function` 从强信号降为弱信号，降低“包含 function 但并非网络 RPC”的误判概率。
- 新增编辑器语境抑制：`FunctionGraph/FunctionTable/CompileFunction/FunctionLibrary` 命中时下调 RPC 分，避免工具链/编辑器事件误判为 RPC。
- 保持“冲突回退 Unknown”策略不变，优先避免错判而不是过度归类。
- 新增 `PacketRef` 分类通道：当 `packet/bunch/ack/controlchannel` 等包级信号足够强，且 RPC/Property 信号不足时，归为 `PacketRef` 而不是 `Unknown`。
- 属性同步词典补充 `ChangeMask/BaseState/Dirty`，提升复制状态命名下的命中率。

补充（同日）：
- bridge 在分类阶段不再只依赖单一名称字段：会组合 `EventTypeName + ContentName` 作为分类输入，提升命中率；UI 显示名保持可读优先（优先非泛化 EventTypeName，其次 ContentName）。
- 新增泛化事件名识别（`Event/NetEvent/ContentEvent/Payload/(UnknownEvent)`），避免这些标签污染分类结果。
- Provider 增加 `PacketRef` 计数/占比指标，并在 Toolbar 文本中展示 `Unknown% + PacketRef%`，用于快速观察归因质量变化。

## 10. UnrealInsights 程序侧改造（2026-03-10）
- 插件描述调整：
  - `SupportedPrograms: ["UnrealInsights"]`
  - 模块 `Type` 从 `Editor` 调整为 `EditorAndProgram`
  - 增加 `ProgramAllowList: ["UnrealInsights"]`
- 构建依赖分层：
  - 通用依赖保留 `Slate/TraceInsights/TraceServices/TraceAnalysis`
  - `Engine` 仅在 `Target.bCompileAgainstEngine` 时添加
  - `EditorStyle/ToolMenus/WorkspaceMenuStructure` 仅 `TargetType.Editor` 添加
- 模块代码防护：
  - `ToolMenus` 和 `LevelEditor.MainMenu.Window` 注入逻辑只在 `WITH_EDITOR` 编译路径启用
  - Program 路径仅保留 tab spawner 与数据链路，不依赖 Editor-only API
- 已验证结果：
  - `LyraEditor` 自动化测试保持全绿
  - `UnrealInsights` Program 目标可成功编译，并产出 `UnrealInsights-ObjectNetInspector.dll`
  - 运行日志可见插件挂载与模块启动日志
