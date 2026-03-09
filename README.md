# ObjectNetInspector (MVP)

## 1. 插件用途
`ObjectNetInspector` 是一个 Unreal Insights 扩展面板插件，用于做对象级网络分析。
它聚焦于“某个对象在一段时间内发生了哪些 RPC / 属性同步事件，以及可归因载荷量级”。

## 2. MVP 功能范围
- 在 Insights 中提供 `Object Net Inspector` 面板（当前代码包含最小 Slate 面板骨架）
- 按对象名 / 路径 / 类名 / 事件名搜索过滤
- 显示对象聚合列表（事件数、RPC 数、属性数、Known Bytes）
- 显示选中对象详情（Top RPC / Top Property）
- 显示选中对象事件表（时间、方向、连接、类型、包、Bits/Bytes）
- 支持 ConnectionId 过滤
- 支持时间窗过滤
- 支持 RPC / Property 开关及 Outgoing Only 过滤
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

## 5. 建议开发顺序
1. 阶段 A：插件骨架（uplugin/build/module）
2. 阶段 B：核心数据结构（Event/Aggregate/Query）
3. 阶段 C：Trace 接入适配层 + Mock 链路
4. 阶段 D：索引与聚合
5. 阶段 E：Slate 面板联动
6. 阶段 F：README + 扩展计划

## 6. 当前实现说明
- `FObjectNetTraceReader::InitializeFromActiveSession()` 目前为占位适配入口，返回 `false`
- `LoadMockDataForTesting()` 提供 3 个对象、多连接、部分无 BitCount 的测试事件
- 对象列表默认按 Known Bytes（已知位数换算）降序
- 事件表默认按时间升序
- 格式化工具位于 `Private/Utils/ObjectNetFormatting.cpp`

## 7. 后续扩展方向
- 更精细字节归因（按字段/子系统拆分）
- 与 Networking Insights 包/连接视图联动跳转
- 可选运行时 HUD（调试构建专用）
- Iris 专项适配（协议事件归因与字段映射）

## 8. TODO（明确下一步）
- 接入真实 Insights/Trace 会话 API，替换 Reader stub
- 在模块启动时注册 Insights Tab Spawner
- 将 UI 局部类迁移到独立头文件并整理 include
- 增加自动化用例：Query 过滤、聚合正确性、无 BitCount 行为
