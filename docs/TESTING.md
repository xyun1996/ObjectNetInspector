# TESTING - ObjectNetInspector

更新时间：2026-03-11（Asia/Shanghai）

## 1. 前置条件
- 插件已放入 UE 工程（例如 Lyra）并能成功编译。
- 本机存在可用 Unreal 引擎目录（脚本会自动探测）。

## 2. 单测名称
- `ObjectNetInspector.Provider.FilteringAndAggregation`
- `ObjectNetInspector.Provider.SearchFields`
- `ObjectNetInspector.Provider.FilterEdgeCases`
- `ObjectNetInspector.Classifier.KindInference`
- `ObjectNetInspector.Classifier.QualityGuard`
- `ObjectNetInspector.MetadataParser.ObjectNamePath`

## 3. 推荐跑法（脚本）
在仓库根目录执行：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 -ProjectPath "G:\workspace\ue5\Lyra"
```

默认行为：
- 强制同步当前插件到目标工程 `Plugins/ObjectNetInspector`
- 构建 `LyraEditor Win64 Development`
- 跑 `ObjectNetInspector.` 前缀下全部自动化测试

可选参数：
- `-EngineRoot`：指定引擎目录（不传时自动探测）
- `-SkipBuild`：跳过构建直接跑测试（要求目标工程已存在可加载的插件二进制）
- `-TestName`：跑单个测试
- `-TestNames`：跑多个指定测试
- `-ReportOutputPath`：自定义报告目录

示例（单测）：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 `
  -ProjectPath "G:\workspace\ue5\Lyra" `
  -TestName "ObjectNetInspector.Classifier.KindInference"
```

## 4. 结果判定
满足以下两项即视为通过：
- 日志包含：`Test Completed. Result={Success}`
- 脚本退出码为 `0`

脚本已内置报告校验：
- 读取 `Saved/AutomationReports/index.json`
- 要求 `failed == 0`
- 要求至少有一个成功（`succeeded > 0` 或 `succeededWithWarnings > 0`）

脚本还会在每次运行前清空报告目录，避免读到历史结果。
当使用 `-SkipBuild` 时，脚本会预检 `UnrealEditor-ObjectNetInspector.dll`，缺失会直接报错提示先跑一次完整构建。

## 5. 编辑器内跑法（备选）
- 打开编辑器
- 打开 `Tools -> Session Frontend -> Automation`
- 搜索对应测试名
- 运行并确认结果为 Success

## 6. Unreal Insights 程序侧验证（手工）
用于验证插件是否能在 `UnrealInsights.exe` 中加载并注册面板：

1. 先编译 Program 目标（至少一次）：

```powershell
G:\workspace\repo\github\UnrealEngine\Engine\Build\BatchFiles\Build.bat `
  UnrealInsights Win64 Development `
  -Project="G:\workspace\ue5\Lyra\Lyra.uproject" `
  -EnablePlugins=ObjectNetInspector `
  -WaitMutex -FromMsBuild
```

2. 打开 trace：

```powershell
G:\workspace\repo\github\UnrealEngine\Engine\Binaries\Win64\UnrealInsights.exe `
  -project="G:\workspace\ue5\Lyra\Lyra.uproject" `
  -OpenTraceFile="G:\workspace\ue5\Lyra\Saved\Profiling\20260310_215400_829D80.utrace" `
  -log
```

3. 日志判定（`Engine\Programs\UnrealInsights\Saved\Logs\UnrealInsights.log`）：
- 应出现：`LogPluginManager: Mounting Project plugin ObjectNetInspector`
- 应出现：`LogObjectNetInspector: ObjectNetInspector module started and tab spawner registered.`

4. UI 判定：
- 打开 `Object Net Inspector` 面板后点击 `Refresh`；
- 若读取到 active session，顶部状态应为 `Source: Session`；
- 否则会回落 `Source: Mock`（这代表会话读取链路未命中，而不是面板不可用）。

## 7. 一键启动脚本
新增脚本：`scripts/Launch-UnrealInsights.ps1`

默认行为：
- 自动解析 `ProjectPath`
- 自动探测引擎目录
- 默认同步插件到项目 `Plugins/ObjectNetInspector`
- 未指定 `-TraceFile` 时自动选择 `Saved/Profiling` 下最新 `.utrace`

示例：

```powershell
pwsh -File .\scripts\Launch-UnrealInsights.ps1
```

常用参数：
- `-BuildInsights:$true`：启动前先编译 `UnrealInsights`
- `-SyncPlugin:$false`：跳过插件同步
- `-TraceFile "G:\path\to\sample.utrace"`：指定 trace 文件
- `-AutoQuit`：打开并自动退出（适合脚本验证）
- `-DisableOtherPlugins`：仅启用 ObjectNetInspector（排查插件冲突）
- `-NoAutoTraceScan`：不自动扫描最新 `.utrace`；仅在显式 `-TraceFile` 时打开 trace
- 脚本默认会追加 `-EnablePlugins=ObjectNetInspector`，避免 Program 路径插件启用状态不稳定。

## 8. 一键 Smoke 验证（程序侧）
新增脚本：`scripts/Smoke-ObjectNetInsights.ps1`

用途：
- 自动调用 `Launch-UnrealInsights.ps1`（`-AutoQuit`）
- 校验 UnrealInsights 日志关键标记是否出现：
  - `Mounting Project plugin ObjectNetInspector`
  - `ObjectNetInspector module started and tab spawner registered.`

示例：

```powershell
pwsh -File .\scripts\Smoke-ObjectNetInsights.ps1 `
  -ProjectPath "G:\workspace\ue5\Lyra\Lyra.uproject" `
  -TraceFile "C:\Users\xianyun\AppData\Local\UnrealEngine\Common\UnrealTrace\Store\001\20260311_111505.utrace"
```

说明：
- 脚本会输出 Net 相关日志提示（`NetProfiler/Networking Insights/NetTraceVersion`）用于辅助判断。
- trace 是否真的包含 NetProfiler 数据，仍以 `Session Info -> Analysis Modules` 为准。
