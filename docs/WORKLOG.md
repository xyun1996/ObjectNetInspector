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
- 自动化测试 `ObjectNetInspector.Provider.FilteringAndAggregation`、`ObjectNetInspector.Provider.SearchFields`、`ObjectNetInspector.Classifier.KindInference`、`ObjectNetInspector.MetadataParser.ObjectNamePath` 运行结果：Success。

## 7. 文档约定
- 开发过程中同步维护 `docs/DESIGN_NOTES.md`，记录设计思路与关键取舍。
