# ObjectNetInspector Sampling Template

更新时间：2026-03-10（Asia/Shanghai）

## 1. 采样目标
- 收集真实 NetTrace 样本，定位 `Unknown` 高占比来源。
- 基于高频未知事件做“定向分类器迭代”，每轮可回归验证。

## 2. 采样前准备
1. 打开 `UnrealInsights.exe`。
2. 启动 Live Trace（或连接目标会话）。
3. 在游戏里准备要覆盖的场景（移动/战斗/高密度对象）。

## 3. 建议采样场景
1. `Movement`：常规移动与交互（30-90 秒）
2. `Combat`：高频技能/射击/RPC（30-90 秒）
3. `DenseActors`：高对象密度或多人同步（30-90 秒）

## 4. Bookmarks（可选但推荐）
在 UE 控制台使用：
- `Trace.Bookmark ONI_<Scene>_Start`
- `Trace.Bookmark ONI_<Scene>_End`

示例：
- `Trace.Bookmark ONI_Combat_Start`
- `Trace.Bookmark ONI_Combat_End`

## 5. 文件命名建议
- `oni_<project>_movement_YYYY-MM-DD.utrace`
- `oni_<project>_combat_YYYY-MM-DD.utrace`
- `oni_<project>_dense_YYYY-MM-DD.utrace`

## 6. 采样记录表（填写）

| Trace File | Scene | Date | Duration(s) | Time Window (sec) | Connection Filter | Total Events | Unknown Events | Unknown % | PacketRef % | Top Unknown #1 | Top Unknown #2 | Top Unknown #3 | Notes |
|---|---|---|---:|---|---|---:|---:|---:|---:|---|---|---|---|
|  | Movement |  |  |  | Any / ID |  |  |  |  |  |  |  |  |
|  | Combat |  |  |  | Any / ID |  |  |  |  |  |  |  |  |
|  | DenseActors |  |  |  | Any / ID |  |  |  |  |  |  |  |  |

## 7. Top Unknown 详细清单（可多条）

| Trace File | Unknown Event Name | Count (Estimate) | Direction | ConnectionId | Suspected Kind (Rpc/Property/PacketRef/Unknown) | Comment |
|---|---|---:|---|---|---|---|
|  |  |  | Outgoing/Incoming |  |  |  |
|  |  |  | Outgoing/Incoming |  |  |  |
|  |  |  | Outgoing/Incoming |  |  |  |

## 8. 交付给 Codex 的最小信息
至少给出每个 trace 的：
1. `Unknown%`
2. `PacketRef%`
3. Top 3 Unknown 事件名
4. 是否集中在某个 `ConnectionId`

有了这些信息，就可以直接进入下一轮：
- 分类词典扩展
- 冲突规则微调
- 自动化回归补充
- 结果对比记录
