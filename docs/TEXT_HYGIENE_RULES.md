# TEXT HYGIENE RULES

更新时间：2026-03-10（Asia/Shanghai）

## 1. 目标
- 避免误写入字面量 `` `r`n `` / `` 'r'n ``。
- 避免 CRLF/LF 混乱导致的 `git diff --check` 报错。
- 统一文本编码和行尾，降低跨工具编辑副作用。

## 2. 仓库强制约束
- `.gitattributes`：统一文本文件行为。
- `.editorconfig`：约束编辑器保存时的编码/换行/尾随空格。
- 规则优先级：`.gitattributes` > Git 配置 > 编辑器默认设置。

## 3. 编码与换行约定
- 默认文本编码：`UTF-8`（不带 BOM）。
- 默认行尾：`LF`。
- Windows 脚本/工程文件保留 `CRLF`：
  - `*.ps1`, `*.bat`, `*.cmd`
  - `*.sln`, `*.vcxproj`, `*.filters`, `*.props`

## 4. 明确禁止项
- 禁止把换行当普通字符串拼接写入文件（例如把 `` `r`n `` 当文本写进去）。
- 禁止使用会隐式改编码或行尾的“无约束写文件”方式。
- 禁止在未检查的情况下做批量替换换行。

## 5. PowerShell 写文件规则
- 优先使用 `apply_patch` 修改源码/文档。
- 如必须用 PowerShell 写文件，优先：
  - `[System.IO.File]::WriteAllText(path, content, (New-Object System.Text.UTF8Encoding($false)))`
- 避免直接 `Set-Content` 做大段文本重写（容易引入编码/行尾偏差）。

## 6. 提交前必做检查
- 先跑：
  - `git diff --check`
- 若失败，先修复尾随空白/异常换行，再提交。
- 可追加检查：
  - `git status --short`
  - 针对关键文件做 `Format-Hex -Count 8` 观察 BOM/行尾异常（必要时）。

## 7. 发现问题时的修复顺序
1. 定位是“字面量误写入”还是“行尾不一致”。
2. 先修语义错误（例如去掉 `` `r`n `` 字面量）。
3. 再统一目标文件行尾与编码。
4. 重新执行 `git diff --check` 确认无告警。
