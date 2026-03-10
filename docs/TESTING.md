# TESTING - ObjectNetInspector

更新时间：2026-03-10（Asia/Shanghai）

## 1. 前置条件
- 插件已放入 UE 工程（例如 Lyra）并能成功编译。
- 本机存在可用 Unreal 引擎目录（脚本会自动探测）。

## 2. 单测名称
- `ObjectNetInspector.Provider.FilteringAndAggregation`
- `ObjectNetInspector.Provider.SearchFields`
- `ObjectNetInspector.Classifier.KindInference`
- `ObjectNetInspector.MetadataParser.ObjectNamePath`

## 3. 推荐跑法（脚本）
在仓库根目录执行：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 -ProjectPath "E:\eworkspace\Lyra"
```

默认行为：
- 强制同步当前插件到目标工程 `Plugins/ObjectNetInspector`
- 构建 `LyraEditor Win64 Development`
- 跑 `ObjectNetInspector.` 前缀下全部自动化测试

可选参数：
- `-EngineRoot`：指定引擎目录（不传时自动探测）
- `-SkipBuild`：跳过构建直接跑测试
- `-TestName`：跑单个测试
- `-TestNames`：跑多个指定测试
- `-ReportOutputPath`：自定义报告目录

示例（单测）：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 `
  -ProjectPath "E:\eworkspace\Lyra" `
  -TestName "ObjectNetInspector.Classifier.KindInference"
```

## 4. 结果判定
满足以下两项即视为通过：
- 日志包含：`Test Completed. Result={Success}`
- 脚本退出码为 `0`

## 5. 编辑器内跑法（备选）
- 打开编辑器
- 打开 `Tools -> Session Frontend -> Automation`
- 搜索对应测试名
- 运行并确认结果为 Success
