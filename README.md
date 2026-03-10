# ObjectNetInspector (MVP)

## 1. 插件用途
`ObjectNetInspector` 是一个 Unreal Insights 扩展面板插件，用于做对象级网络分析。
它聚焦于“某个对象在一段时间内发生了哪些 RPC / 属性同步事件，以及可归因载荷量级”。

## 2. MVP 功能范围
- 在 Insights 中提供 `Object Net Inspector` Nomad Tab 面板
- 按对象名 / 路径 / 类名 / 事件名搜索过滤
- 显示对象聚合列表（事件数、RPC 数、属性数、Known Bytes）
- 显示选中对象详情（Top RPC / Top Property）
- 显示选中对象事件表（时间、方向、连接、类型、包、Bits/Bytes）
- 支持 ConnectionId 过滤
- 支持时间窗过滤
- 支持 RPC / Property 开关及 Outgoing Only 过滤
- 工具栏显示数据源状态（Session / Mock）、当前事件总数、`Unknown%` 与 `PacketRef%`
- 无真实 Trace 接入时自动回落到 `LoadMockDataForTesting()`

## 3. 非目标
- 不做运行时 UMG/HUD
- 不做 socket 抓包与底层协议反解
- 不做网卡层精确流量核算
- 不做完整包头/通道开销还原
- 第一版不强行覆盖所有 Iris/旧复制细节差异

## 4. “对象流量”定义
对象流量 = 与该对象相关 trace 事件中**可归因 payload bits/bytes**的聚合值。

该值用于：
- 调试定位
- 对象间相对比较

该值不等同于：
- 网卡层真实流量
- 完整数据包级精确统计

如果事件拿不到 Bits/Bytes：
- 事件仍展示
- 显示 `N/A`
- 聚合只累计已知 bits/bytes
- 事件计数照常累计

## 5. 当前实现结构
- `FObjectNetTraceReader`：统一读取入口，支持可注入 SessionReader 与 mock 回落
- `FObjectNetInsightsBridge`：UE5.7 兼容的 Insights/NetProfiler 会话读取与事件映射适配边界
- `FObjectNetAnalyzer`：按对象/连接/包建立索引
- `FObjectNetAggregator`：生成对象聚合并排序
- `FObjectNetProvider`：管理 Query、选中对象、刷新和数据源状态
- Slate UI：Toolbar + ObjectList + DetailView + EventTable

## 6. 建议开发顺序
1. 阶段 A：插件骨架（uplugin/build/module）
2. 阶段 B：核心数据结构（Event/Aggregate/Query）
3. 阶段 C：Trace 接入适配层 + Mock 链路
4. 阶段 D：索引与聚合
5. 阶段 E：Slate 面板联动
6. 阶段 F：README + 扩展计划

## 7. 后续扩展方向
- 更精细字节归因（按字段/子系统拆分）
- 与 Networking Insights 包/连接视图联动跳转
- 可选运行时 HUD（调试构建专用）
- Iris 专项适配（协议事件归因与字段映射）

## 8. TODO（明确下一步）
- 继续提升 `FObjectNetInsightsBridge` 的事件类型归因准确率（减少 Unknown/误判）
- 若 UE API 可提供，补充对象真实 `ClassName/ObjectPath` 映射
- 视 UE 版本细化 Tab 在 Insights Workspace 菜单中的挂载
- 增加更多自动化用例（归因分类稳定性、真实会话映射回归）

## 9. 文本编辑规则
- 为避免 CRLF/LF 混乱和误写入 `` `r`n `` 字面量，请遵循：
  - [docs/TEXT_HYGIENE_RULES.md](docs/TEXT_HYGIENE_RULES.md)
