# fmp3_archive

TOPPERS で公開済みの **FMP3 カーネル**（個別パッケージ／簡易パッケージ）を、日付順・タグ付きで再構成した **pristine アーカイブ**。将来の派生版（`fmp3_core`：CMake + Python cfg）の**上流**として pin される。

- 公開元：<https://www.toppers.jp/fmp3-kernel.html>
- 設計の全体像：[docs/fmp3_git_management_plan.md](docs/fmp3_git_management_plan.md)
- 派生版 `fmp3_core` からの取り込み方：[docs/consuming.md](docs/consuming.md)
- 作業規約・手順の正本：[AGENTS.md](AGENTS.md)

## 構成

- `release/3.M` … メジャーバージョン毎のブランチ（`main` または前メジャー tip から分岐）
- タグ … 非依存部 `v3.M.N`、依存部 `<target>_<tool>-<date>`（例 `polarfire_soc_kit_gcc-20241224`）

## クイックスタート

```bash
# 1) 取込元 tarball を packages/ に置く
# 2) RELEASES.csv に1行ずつ追記（日付昇順）
# 3) 一括取込
tools/import_all.sh            # dry-run
tools/import_all.sh --execute  # 実行
```

詳細・単一取込・検証は [AGENTS.md](AGENTS.md) を参照。

## ライセンス

各パッケージは [TOPPERS ライセンス](https://www.toppers.jp/license.html)。本リポジトリは配布物のアーカイブであり、内容の改変は行わない。
