# TESTING - ObjectNetInspector

更新时间：2026-03-10（Asia/Shanghai）

## 1. 前置条件
- 插件已放入 UE 工程（例如 Lyra）并能成功编译。
- 已启用 UE5.7（默认路径：`C:\Program Files\Epic Games\UE_5.7`）。

## 2. 单测名称
- `ObjectNetInspector.Provider.FilteringAndAggregation`
- `ObjectNetInspector.Classifier.KindInference`

## 3. 推荐跑法（脚本）
在仓库根目录执行：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 -UProject "G:\path\to\YourProject.uproject"
```

可选参数：
- `-EngineRoot`：自定义引擎目录
- `-ReportOutputPath`：自定义报告目录
- `-TestName`：自定义测试过滤名

示例：

```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 `
  -UProject "G:\workspace\lyra\LyraStarterGame.uproject" `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.7"
```

## 4. 结果判定
满足以下两项即视为通过：
- 日志包含：`completed with result 'Success'`
- 退出码为 `0`

## 5. 编辑器内跑法（备选）
- 打开编辑器
- 打开 `Tools -> Session Frontend -> Automation`
- 搜索对应测试名
- 运行并确认结果为 Success