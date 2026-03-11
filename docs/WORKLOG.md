# WORKLOG - ObjectNetInspector

更新时间：2026-03-10（Asia/Shanghai）

## 1. 项目目标（MVP）
在 Unreal Insights 中提供对象级网络分析面板：
- 以对象为主维度查看 RPC / 属性同步事件
- 支持搜索、连接过滤、时间窗过滤
- 提供对象聚合统计（事件数、RPC、属性、包命中、Known Bits/Bytes）
- 提供选中对象事件明细
- 未知 BitCount 事件显示 N/A，但计数保留

## 2. 已完成内容
### 阶段 A/B/C/D/E/F（当前仓库状态）
- 插件骨架与模块配置已完成（uplugin / Build.cs / Module）
- 核心类型、查询、分析器、聚合器、Provider 已实现
- Mock 数据链路可跑通（3 个对象，多连接，部分无 BitCount）
- Slate MVP UI 已完成：
  - Toolbar
  - Object List
  - Detail View
  - Event Table
- 模块已注册 Nomad Tab：`Object Net Inspector`
- 会话读取采用桥接适配层：
  - `FObjectNetTraceReader` 支持注入 SessionReader
  - `FObjectNetInsightsBridge` 已接入 active session API
  - 已实现 UE5.7 兼容 NetProfiler -> `FObjectNetEvent` 映射（`EnumeratePackets + EnumeratePacketContentEventsByIndex`）
  - 已实现增强归因：`Kind` 多规则识别 + `Unknown` 回退，并输出 `TypeId` 作为 `ClassName` 回退值
- 已新增最小自动化验证样例：
  - `ObjectNetInspector.Provider.FilteringAndAggregation`
  - `ObjectNetInspector.Provider.SearchFields`
  - `ObjectNetInspector.Classifier.KindInference`
  - `ObjectNetInspector.MetadataParser.ObjectNamePath`
- 该自动化样例已在 UE5.7 编辑器中执行成功（Success）
- 已提供脚本化测试入口：`scripts/Run-ObjectNetTests.ps1` + `docs/TESTING.md`（默认跑 `ObjectNetInspector.` 全量测试）
- 工具栏已显示数据源状态：`Session/Mock` + 当前事件总数

## 3. 当前核心口径（必须保持）
- “对象流量” = 可归因 payload bits/bytes 聚合值（调试与相对比较用途）
- 非网卡层精确流量，不承诺完整包级还原
- BitCount 缺失时：
  - 事件仍显示
  - Bits/Bytes 显示 `N/A`
  - 聚合只累计已知 bits
  - 事件计数正常累计

## 4. 关键文件入口
- 模块入口：`Source/ObjectNetInspector/Private/ObjectNetInspectorModule.cpp`
- Tab 入口声明：`Source/ObjectNetInspector/Public/ObjectNetInspectorTab.h`
- 主面板组装：`Source/ObjectNetInspector/Private/UI/SObjectNetInspectorTab.cpp`
- Provider：`Source/ObjectNetInspector/Public/ObjectNetProvider.h`
- Trace Reader：`Source/ObjectNetInspector/Public/ObjectNetAnalyzer.h`（含 `FObjectNetTraceReader` 声明）
- 会话桥接：
  - `Source/ObjectNetInspector/Public/ObjectNetInsightsBridge.h`
  - `Source/ObjectNetInspector/Private/Analysis/ObjectNetInsightsBridge.cpp`
- 自动化样例：
  - `Source/ObjectNetInspector/Private/Tests/ObjectNetProviderTests.cpp`
  - `Source/ObjectNetInspector/Private/Tests/ObjectNetEventClassifierTests.cpp`
  - `Source/ObjectNetInspector/Private/Tests/ObjectNetMetadataParserTests.cpp`

## 5. 下一步任务
1. 基于分类器与回归测试继续提升 `Kind` 归因准确率（减少 `Unknown` 与误判）
2. 若 UE API 可提供，补充对象真实 `ClassName`（目前已增加对象名启发式回退，仍优先真实 API）
3. Tab 挂载已加编译期守卫（有 Workspace API 则挂 Profiling，否则回退 Nomad）

## 6. 今天新增进展（2026-03-10）
- `Kind` 归因规则升级为“加权评分 + 冲突回退”，降低 server/client 字样导致的 RPC 误判，并新增边界回归用例。
- 解决 UE5.7 编译兼容问题（UI OnSort 签名、头文件路径、桥接 API 适配）。
- 在 UE5.7 下恢复真实事件映射链路（不再仅 session 元信息）。
- 抽离 `FObjectNetMetadataParser`，统一对象名/路径解析，并新增类名启发式回退（用于真实 TypeName 不可用时）。
- 扩展 `Kind` 分类词典（`RepLayout/PushModel/ChangeList/CallRemoteFunction` 等）并补充回归用例，降低 UE 常见命名下的 `Unknown`。
- `Kind` 分类新增“弱信号阈值 + 混合冲突回退”，降低 `RemoteHandle`、`PrepareData`、`ServerRepFunction` 等边界命名误判。
- 修复搜索过滤一致性问题：`SearchText` 在 aggregate 阶段也匹配事件名（`RpcCounts/PropertyCounts`），避免“按事件名搜索命中事件但被聚合过滤掉”。
- 清理 `TraceReader` 过时 TODO，并补充会话读取/回退日志（reader 是否绑定、模块加载状态），提高诊断效率。
- bridge 日志新增 `UnknownRatio` 输出，并在高占比场景给出告警，便于后续按 trace 样本迭代分类规则。
- ClassName 映射增强：对 provider 的 `TypeName` 先做规范化（支持 `Class'/Script/...'`、脚本路径短名提取），再进行对象名推断回退。
- 新增文本卫生规则：补充 `.gitattributes`、`.editorconfig` 与 `docs/TEXT_HYGIENE_RULES.md`，约束编码/换行并明确提交流程检查项。
- 元数据解析增强：`ObjectPath` 识别补充 dot-only 形式，并对输入做外层引号/空白清理，降低真实 trace 名称格式差异导致的映射波动。
- bridge 日志补充 `ClassName` 来源统计（TypeName / Inferred / TypeIdFallback），用于跟踪映射质量迭代效果。
- `Kind` 分类补充 UE5.7/Iris 常见信号词（`NetSerialize`、`Iris`、`Fragment`、`Descriptor`）并新增回归用例，继续压降 `Unknown`。
- `ClassName` 规范化补充临时前缀清洗（`REINST_`、`SKEL_`、`TRASHCLASS_`），减少编辑器态类名噪声对对象聚合可读性的影响。
- 自动化测试 `ObjectNetInspector.Provider.FilteringAndAggregation`、`ObjectNetInspector.Provider.SearchFields`、`ObjectNetInspector.Classifier.KindInference`、`ObjectNetInspector.MetadataParser.ObjectNamePath` 运行结果：Success。
- `Kind` 归因新增网络信号词（`ProcessRemoteFunction`、`DispatchRpc`、`NetField`、`Retirement`、`Lifetime`、`SerializeProperty`），并下调 `function` 权重，减少非网络 function 命名误判。
- 增加“编辑器 function 语境抑制”规则（`FunctionGraph/FunctionTable/CompileFunction/FunctionLibrary`），进一步降低 RPC 误报。
- 新增回归用例覆盖上述边界：`ProcessRemoteFunctionForChannel`、`FunctionGraphCompilePass`、`BuildFunctionTable`、`NetFieldRetirementState`。
- bridge 事件归因输入增强：分类时组合 `EventTypeName + ContentName`，显示名与分类文本解耦，降低因单字段缺失/泛化命名导致的 `Unknown`。
- 元数据解析回归补充：`TRASHCLASS_` 前缀规范化新增自动化覆盖。
- 自动化测试再次执行（2026-03-10 20:21 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- 测试脚本增强：新增报告强校验（解析 `index.json`，要求 `failed==0` 且存在成功项），并在每次运行前清空报告目录，防止历史结果污染。
- 修复 `SkipBuild` 流程：跳过构建时不再删除目标插件目录，保留已有 `Binaries/Intermediate`，支持快速单测回归。
- 脚本验证结果（2026-03-10 20:28 CST）：
  - 全量：`pwsh -File .\scripts\Run-ObjectNetTests.ps1` -> Success（4/4）
  - 单测：`pwsh -File .\scripts\Run-ObjectNetTests.ps1 -SkipBuild -TestName "ObjectNetInspector.Classifier.KindInference"` -> Success（1/1）
- Provider/Toolbar 新增归因质量可视化：显示 `Unknown` 占比（`Unknown%`），便于根据真实会话数据持续调优分类规则。
- 自动化测试再次执行（2026-03-10 20:32 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- `Provider.FilteringAndAggregation` 测试新增 `Unknown` 事件样本，并断言 `GetLastUnknownEventCount()/GetLastUnknownRatio()`，为归因质量指标提供回归保护。
- 自动化测试再次执行（2026-03-10 20:34 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- ClassName 回退链路增强：在 `TypeName` 与对象名都不足时，新增“从事件名作用域推断类名”（如 `AbilitySystemComponent::ServerTryActivateAbility`）。
- `ObjectNetMetadataParser` 新增 `TryInferClassNameFromEventName`，并增加对应自动化回归用例。
- 自动化测试再次执行（2026-03-10 20:37 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- 脚本 `-SkipBuild` 体验增强：新增插件 DLL 预检，若二进制缺失会明确提示“先跑一次不带 `-SkipBuild` 的完整构建”。
- 单测验证（2026-03-10 20:39 CST）：`ObjectNetInspector.Provider.FilteringAndAggregation` Success（1/1）。
- bridge 诊断日志增强：`ClassName` 来源统计细分为 `TypeName / ObjectName / EventScope / TypeIdFallback`，便于定向评估哪条回退链路贡献最大。
- 自动化测试再次执行（2026-03-10 20:38 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- `Kind` 归因新增 `PacketRef` 识别（`packet/bunch/ack/controlchannel` 等），用于将包级上下文事件从 `Unknown` 中分离，辅助 `Unknown%` 持续下降。
- 分类器补充属性同步信号词：`ChangeMask/BaseState/Dirty`，提升 Iris/复制状态类命名下的归因覆盖。
- 新增回归用例：`ChannelBunchPacket`、`NetPacketAck`（期望 `PacketRef`）以及 `ReplicatedPropertyPacket`（验证属性信号优先于包级弱信号）。
- 自动化测试再次执行（2026-03-10 20:44 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- Provider/Toolbar 质量指标扩展：新增 `PacketRef` 事件计数与占比（`PacketRef%`），便于和 `Unknown%` 一起追踪归因演进。
- `ObjectNetInspector.Provider.FilteringAndAggregation` 新增 `PacketRef` 样本与断言（事件数、`PacketRefCount`、`PacketRefRatio`）。
- 自动化测试再次执行（2026-03-10 20:47 CST）：`ObjectNetInspector.` 全部 Success（4/4，failed=0）。
- 为解决“Editor 中可见、UnrealInsights 中不可见”的问题，启动 UnrealInsights 程序侧改造（`EditorAndProgram`）。
- `ObjectNetInspector.uplugin` 调整为：
  - `SupportedPrograms = UnrealInsights`
  - 模块 `Type = EditorAndProgram`
  - `ProgramAllowList = UnrealInsights`
- `Build.cs` 依赖分层完成：
  - Editor-only 依赖（`EditorStyle/ToolMenus/WorkspaceMenuStructure`）仅在 `TargetType.Editor` 引入
  - Program 路径保留 Trace/Slate 核心依赖，避免 Editor-only 链接问题
- 模块代码增加 `WITH_EDITOR` 守卫：
  - Editor 独有 `Window` 菜单注入逻辑不进入 Program 编译路径
  - Program 路径保留 tab spawner 与数据刷新能力
- Program 侧实机验证：
  - 编译 `UnrealInsights Win64 Development -Project=Lyra` 成功
  - 日志确认：`Mounting Project plugin ObjectNetInspector`
  - 日志确认：`ObjectNetInspector module started and tab spawner registered.`
- Editor 回归验证（2026-03-10 22:29 CST）：
  - `pwsh -File .\scripts\Run-ObjectNetTests.ps1` -> Success（4/4，failed=0）
- 解决 UnrealInsights 插件加载报错（`module 'ObjectNetInspector' could not be found`）：
  - 确认 Program 侧二进制必须存在：`UnrealInsights-ObjectNetInspector.dll`
  - 构建命令补充 `-EnablePlugins=ObjectNetInspector`
  - `Launch-UnrealInsights.ps1` 增加启动前 DLL 预检与明确报错提示
  - `Lyra.uproject` 增加 `ObjectNetInspector` 启用项；脚本新增 `-ForceEnablePluginArg` 以支持命令行强制启用模式

## 7. 文档约定
- 开发过程中同步维护 `docs/DESIGN_NOTES.md`，记录设计思路与关键取舍。

## 8. Unreal Insights 程序侧改造计划（2026-03-10）
目标：让 `Object Net Inspector` 在 `UnrealInsights.exe` 中作为 Insights 扩展面板运行，并读取 active session（不再依赖 Editor 进程内回落）。

阶段计划：
1. 模块形态切换
   - `uplugin` 调整为 `EditorAndProgram`，限定 `ProgramAllowList=UnrealInsights`。
   - 保留 Editor 路径，确保现有项目工作流不回归。
2. 依赖分层
   - `Build.cs` 按 `Editor` / `Program` 条件裁剪依赖。
   - Editor 专属依赖（`ToolMenus`、`EditorStyle` 等）不进入 Program 构建链。
3. Tab 与菜单策略
   - 保留通用 Tab 注册（Insights/Editor 均可用）。
   - Editor 独有 `Window` 菜单入口做编译期守卫；Insights 侧依赖 Workspace/Nomad 入口。
4. 会话读取验证
   - 在 `UnrealInsights.exe` 打开 `.utrace` 验证 `Source: Session`。
   - 会话不可得时仍保持 mock fallback（`Source: Mock`）与日志告警。
5. 回归与文档
   - Editor 自动化测试必须保持全绿。
   - 补充 README/TESTING 的 Insights 启动与验证步骤。
