# Contributing

Thanks for contributing to ObjectNetInspector.

## Workflow
1. Create a feature branch from `main`.
2. Keep each commit focused and atomic.
3. Open a PR with context, screenshots/logs if behavior changed.

## Before opening PR
1. Run automated tests:
   - `pwsh -File .\scripts\Run-ObjectNetTests.ps1 -ProjectPath "<YourProject>.uproject"`
2. Ensure `ObjectNetInspector.` tests are all green.
3. Update docs when needed:
   - `docs/WORKLOG.md` for what changed
   - `docs/DESIGN_NOTES.md` for design decisions/tradeoffs

## Coding guidelines
1. Prefer minimal, maintainable changes.
2. Keep UI responsive: avoid per-tick full scans, prefer cached/revision-based refresh.
3. For classifier/rules updates, add regression tests in:
   - `Source/ObjectNetInspector/Private/Tests/ObjectNetEventClassifierTests.cpp`
4. For parsing/mapping updates, add tests in:
   - `Source/ObjectNetInspector/Private/Tests/ObjectNetMetadataParserTests.cpp`
   - `Source/ObjectNetInspector/Private/Tests/ObjectNetProviderTests.cpp`

## Formatting / text hygiene
Follow:
- `docs/TEXT_HYGIENE_RULES.md`

## Commit message suggestions
- `feat: ...` new capability
- `fix: ...` bug fix
- `test: ...` tests only
- `docs: ...` docs only
- `chore: ...` maintenance

## Reporting issues
Please include:
1. UE version and branch
2. Project path type (source build / installed build)
3. Repro steps
4. Expected vs actual behavior
5. Relevant logs (UnrealInsights + plugin logs)