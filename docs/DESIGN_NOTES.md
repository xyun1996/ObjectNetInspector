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

## 4. UE5.7 事件映射方案
- 数据来源：`INetProfilerProvider`
  - `ReadNames` 生成 name 索引
  - `ReadEventTypes` 生成 event type 索引
  - `ReadObjects` 构建 object 元信息
  - `ReadConnections + EnumeratePackets + EnumeratePacketContentEventsByIndex` 遍历内容事件
- 字段映射：
  - `ObjectId`：优先 `NetObjectId`，缺失回退为 `GameInstanceIndex:ObjectIndex` 组合键
  - `ObjectName`：优先 name 表，缺失回退 `Object_<index>`
  - `ClassName`：当前回退为 `TypeId_0x...`（等待真实类名 API）
  - `TimeSec`：`Packet.TimeStamp`
  - `ConnectionId`：`Connection.ConnectionId`
  - `PacketId`：`SequenceNumber + 1`（避免 0）
  - `BitCount`：`EndPos - StartPos`（仅 End>Start）

## 5. Kind 归因策略（当前）
- 优先按事件名关键词匹配：
  - RPC 类：`rpc/function/netmulticast/server/client`
  - Property 类：`property/prop/rep/state/delta/array/serializer`
- 其次按 level 作保守回退：`EventTypeLevel` 或 `ContentLevel` 较高时归为 Property。
- 仍无法确定时归为 `Unknown`，避免硬编码误判。

## 6. 验证
- 自动化测试：`ObjectNetInspector.Provider.FilteringAndAggregation`
  - 覆盖过滤、聚合、`N/A` 行为。
  - 已在 UE5.7 环境通过（Success）。

## 7. 当前剩余工作
1. 继续提升 `Kind` 归因准确率（降低 Unknown 与误判）
2. 接入真实 `ClassName/ObjectPath`（若 provider 可提供）
3. 视 UE 版本细化 Tab 在 Insights Workspace 菜单中的挂载

## 8. 文档维护约定
- 每次做结构性改动（接口、口径、流程、取舍）时，同步更新本文件。
- 每次迭代结果与待办同步到 `docs/WORKLOG.md`。