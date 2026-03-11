# ObjectNetInspector

`ObjectNetInspector` 是一个 Unreal Insights 扩展面板插件，用于对象级网络分析（RPC / 属性同步 / 可归因载荷）。

## 作者与维护
- 第一作者：Codex (GPT-5, OpenAI)
- 项目发起与主维护：xianyun
- 说明：本仓库采用「AI + 人类协作开发」模式，所有关键改动均经过可运行脚本与自动化回归验证。

## 项目目标
- 在 Unreal Insights 中提供 `Object Net Inspector` 面板
- 按对象查看事件聚合（Events / RPCs / Properties / Known Bytes）
- 支持搜索、连接过滤、时间窗过滤、方向过滤
- 展示对象详情与事件明细（Time / Direction / Connection / Type / Packet / Bits/Bytes）
- 在无 active session 时自动回退 Mock 数据，保证面板始终可用

## 当前状态（MVP）
- UE5.7 Editor + UnrealInsights Program 侧已打通
- Program 模块支持：`EditorAndProgram` + `ProgramAllowList=UnrealInsights`
- 已有自动化测试与一键脚本
- 已完成主要性能与交互修复（列表排序、选择稳定性、大样本响应优化）

## 快速开始

### 1) 准备
- Windows + PowerShell 7
- UE5.7 源码或已安装引擎
- 一个 `.uproject` 工程（示例：Lyra）

### 2) 一键运行自动化
```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 -ProjectPath "G:\workspace\ue5\Lyra\Lyra.uproject"
```

### 3) 启动 Unreal Insights（自动同步插件）
```powershell
pwsh -File .\scripts\Launch-UnrealInsights.ps1 `
  -ProjectPath "G:\workspace\ue5\Lyra\Lyra.uproject" `
  -TraceFile "C:\Users\<you>\AppData\Local\UnrealEngine\Common\UnrealTrace\Store\001\sample.utrace"
```

### 4) 在 Insights 中使用
- 打开 `Object Net Inspector` 面板
- 点击 `Refresh`
- 用搜索框或过滤条件缩小范围
- 选中左侧对象查看右侧详情/事件
- 可点击 `Networking` 按钮跳到 Networking Insights 做交叉分析

## 如何采集可分析的 NetTrace
- 在 PIE/Standalone 运行前，确保网络追踪已启用（如 `NetTrace.SetTraceVerbosity 1`）
- 生成 `.utrace` 后用上面的启动脚本打开
- 面板顶部若显示 `Source: Session` 表示已读取真实会话数据

## 核心口径（务必统一）
- 对象流量 = 该对象关联事件中的**可归因 payload bits/bytes**聚合值
- 若事件缺失 `BitCount`：
- 事件仍保留
- `Bits/Bytes` 显示 `N/A`
- 聚合仅累计已知 bits

## 项目结构（给二次开发者）
- `Source/ObjectNetInspector/Private/Analysis/ObjectNetInsightsBridge.cpp`
  - UE Trace/NetProfiler API 适配层（版本敏感）
- `Source/ObjectNetInspector/Private/Analysis/ObjectNetEventClassifier.cpp`
  - 事件分类器（Rpc/Property/PacketRef/Unknown）
- `Source/ObjectNetInspector/Private/Analysis/ObjectNetProvider.cpp`
  - UI 数据提供者（Query、刷新、缓存、revision）
- `Source/ObjectNetInspector/Private/UI/`
  - Slate 面板实现（Toolbar/ObjectList/Detail/EventTable）
- `Source/ObjectNetInspector/Private/Tests/`
  - 自动化回归测试
- `scripts/`
  - 启动、测试、smoke 校验脚本

## 二次开发指南

### 1) 扩展分类规则（最常见）
1. 修改 `ObjectNetEventClassifier.cpp` 词典/权重
2. 在 `ObjectNetEventClassifierTests.cpp` 增加回归用例
3. 执行 `Run-ObjectNetTests.ps1`，确保 `ObjectNetInspector.` 全绿

### 2) 扩展对象元数据映射
1. 先看 `ObjectNetInsightsBridge.cpp` 里的 `FNetProfilerObjectInstance` 映射
2. 如 UE 新版本暴露更多字段（ClassPath/ObjectPath），优先走真实字段
3. 保留当前回退链路（TypeName -> 推断 -> TypeIdFallback）

### 3) 扩展 UI 交互
1. 优先在 `Provider` 层加能力，再让 UI 订阅 revision 刷新
2. 避免在 Tick 做全量扫描，优先使用缓存+版本号

## 质量与提交流程
- 提交前至少跑：
```powershell
pwsh -File .\scripts\Run-ObjectNetTests.ps1 -ProjectPath "<YourProject>.uproject"
```
- 文档同步更新：
- `docs/WORKLOG.md`：记录本次做了什么
- `docs/DESIGN_NOTES.md`：记录设计取舍
- 文本/换行规则：
- `docs/TEXT_HYGIENE_RULES.md`

## 故障排查
- UE5.7 常见问题与修复步骤：
- [docs/UE57_TROUBLESHOOTING.md](docs/UE57_TROUBLESHOOTING.md)
- 测试与脚本说明：
- [docs/TESTING.md](docs/TESTING.md)

## Roadmap
- 基于真实样本继续降低 `Unknown%`
- 跟进 UE API，补全更强的真实对象元数据
- 完成 UE5.6/5.7 跨版本回归矩阵

## License
当前仓库未单独声明 License。若计划公开发布到 GitHub，建议先补充 `LICENSE` 文件（如 MIT/Apache-2.0）。
