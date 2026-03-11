# UE5.7 排障手册（ObjectNetInspector + UnrealInsights）

更新时间：2026-03-11（Asia/Shanghai）

## 1. 适用范围
- UE5.7
- `ObjectNetInspector` 以 `EditorAndProgram` 方式挂到 `UnrealInsights.exe`
- 项目示例：Lyra

## 2. 常见问题与解决方案

### 问题 A：`Plugin 'ObjectNetInspector' failed to load because module 'ObjectNetInspector' could not be found`
现象：
- 启动 UnrealInsights 弹框提示找不到 `ObjectNetInspector` 模块。

常见根因：
- Program 侧插件二进制缺失（`UnrealInsights-ObjectNetInspector.dll` 未生成）。
- UnrealInsights 启动时未正确带上 `-project=...`，导致只搜索 Engine 插件目录。

解决：
1. 先编译一次 UnrealInsights（必须带插件启用）：
```powershell
G:\workspace\repo\github\UnrealEngine\Engine\Build\BatchFiles\Build.bat `
  UnrealInsights Win64 Development `
  -Project="G:\workspace\ue5\Lyra\Lyra.uproject" `
  -EnablePlugins=ObjectNetInspector `
  -WaitMutex -FromMsBuild
```
2. 确认存在：
`G:\workspace\ue5\Lyra\Plugins\ObjectNetInspector\Binaries\Win64\UnrealInsights-ObjectNetInspector.dll`
3. 启动 UnrealInsights 时带上：
```powershell
G:\workspace\repo\github\UnrealEngine\Engine\Binaries\Win64\UnrealInsights.exe `
  -project="G:\workspace\ue5\Lyra\Lyra.uproject" `
  -log
```
4. 若使用脚本，优先用仓库脚本：
`scripts/Launch-UnrealInsights.ps1`

---

### 问题 B：菜单里看不到 `Object Net Inspector`
现象：
- 插件似乎加载了，但 `Menu -> Windows/Insights Tools` 中没有面板入口。

常见根因：
- UE5.7 程序侧菜单挂载路径差异，部分入口不稳定。

解决：
- 当前实现已在 Program 路径自动 `TryInvokeTab(ObjectNetInspector.MainTab)`。
- 若仍未自动弹出，先确认日志中有：
  - `Mounting Project plugin ObjectNetInspector`
  - `ObjectNetInspector module started and tab spawner registered.`

---

### 问题 C：`Networking Insights` 灰色，`Object Net Inspector` 显示 0 events
现象：
- Trace 能打开，但网络分析模块不可用或事件数为 0。

常见根因：
- 只开了 Trace 但没启用 NetTrace 运行时 verbosity（UE5.7 默认 `GNetTraceRuntimeVerbosity = 0`）。
- 运行方式是单机无网络复制流量。

解决步骤（Editor 控制台按顺序执行）：
```text
Trace.Stop
NetTrace.SetTraceVerbosity 1
Trace.File C:/Users/<User>/AppData/Local/UnrealEngine/Common/UnrealTrace/Store/001/net_test.utrace NetChannel,FrameChannel,cpu,frame,bookmark
```
然后：
1. 使用 `Play As ListenServer`，并连上至少 1 个客户端。
2. 制造 RPC/属性复制流量（移动、开火、技能、属性变更）。
3. 停止采样：`Trace.Stop`
4. 用 UnrealInsights 打开该 `.utrace`

验证点：
- `Session Info -> Analysis Modules` 包含 `NetProfiler`
- `Networking Insights` 不再灰色
- `Object Net Inspector` 点击 `Refresh` 后 events > 0

---

### 问题 D：编译报 `Unable to find plugin 'TraceInsights'` / `Could not find definition for module 'NetworkingInsights'`
现象：
- 旧依赖写法在 UE5.7 下构建失败。

常见根因：
- 把 `TraceInsights` 当作默认必装插件，或直接依赖不存在/改名的模块。

解决：
- 移除对 `TraceInsights` 的硬依赖配置。
- 使用 UE5.7 可用的 `TraceServices` 读取 NetProfiler 数据。
- 在 `Build.cs` 做 Editor/Program 分层依赖，避免 Editor-only 模块污染 Program 构建链。

---

### 问题 E：UE5.7 API 变更导致编译错误（`ReadPacket`、`Object.Level`、`SHeaderRow::OnSort` 等）
现象：
- 典型错误：
  - `INetProfilerProvider::ReadPacket` 不存在
  - `FNetProfilerObjectInstance` 无 `Level`
  - `OnSort` 签名不匹配

解决：
- NetProfiler 读取改为 UE5.7 接口：
  - `EnumeratePackets`
  - `EnumeratePacketContentEventsByIndex`
- 移除已不存在字段访问（如 `Object.Level`）。
- `SHeaderRow::OnSort` 按 UE5.7 签名修正：
  - `void OnSortRequested(EColumnSortPriority::Type, const FName&, EColumnSortMode::Type)`
- 相关 Slate 构造链（`ChildSlot[...] + SHeaderRow::Column(...)`）保持正确语法，避免连锁语法错误。

## 3. 推荐启动脚本
- 使用：
  - `scripts/Launch-UnrealInsights.ps1`
- 已支持：
  - `-ProjectPath`（目录或 `.uproject`）
  - `-TraceFile`（可传文件或目录；目录会自动选最新 `.utrace`）
  - 自动兜底扫描最新 trace（项目 `Saved/Profiling` 与本机 `UnrealTrace/Store`）

示例：
```powershell
.\scripts\Launch-UnrealInsights.ps1 `
  -ProjectPath "G:\workspace\ue5\Lyra\Lyra.uproject" `
  -TraceFile "C:\Users\xianyun\AppData\Local\UnrealEngine\Common\UnrealTrace\Store\001\20260311_111505.utrace"
```

## 4. 最小自检清单
1. `Lyra.uproject` 已启用 `ObjectNetInspector`
2. `Lyra\Plugins\ObjectNetInspector\Binaries\Win64\UnrealInsights-ObjectNetInspector.dll` 存在
3. `UnrealInsights.exe` 用 `-project=...` 启动
4. 采样前执行 `NetTrace.SetTraceVerbosity 1`
5. 会话里有真实网络复制流量（ListenServer + Client）
