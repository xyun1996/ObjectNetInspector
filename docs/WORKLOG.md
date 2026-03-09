# WORKLOG - ObjectNetInspector

更新时间：2026-03-09（Asia/Shanghai）

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
  - `FObjectNetInsightsBridge` 作为真实 Insights API 接入边界（当前安全 stub）
- 工具栏已显示数据源状态：`Session/Mock` + 当前事件总数
- README 已同步更新

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

## 5. 最近主要提交（按时间倒序）
- `e204432` Finalize provider status UX and tab spawn interface
- `10b9807` Add insights bridge stub and toolbar source status
- `354c8aa` Register inspector tab and add session reader adapter
- `a61dadb` Harden formatting API usage and refresh behavior
- `a18719b` Add README draft and shared formatting utilities
- `54d6277` Move plugin files to repository root layout
- `151e421` Implement ObjectNetInspector analysis core and MVP Slate UI
- `667d6dd` Add ObjectNetInspector plugin skeleton and core public headers

## 6. 下一步任务（明天接着做）
1. 在 `FObjectNetInsightsBridge` 中接入已验证的 active session API（优先保证能拿到会话/时间范围/连接信息）
2. 将网络事件映射到 `FObjectNetEvent`（先保证对象关联正确，再补更细 bits 归因）
3. 增加最小验证样例（过滤、聚合、N/A 行为）
4. 视 UE 版本细化 Tab 在 Insights Workspace 菜单中的挂载

## 7. 跨设备继续工作的建议
- 公司电脑拉取仓库后，先读：
  1) `README.md`
  2) `docs/WORKLOG.md`（本文件）
- 再从 `ObjectNetInsightsBridge.cpp` 开始继续真实会话接入
