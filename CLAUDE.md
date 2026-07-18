# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

このリポジトリでの作業規約・手順の**正本は [AGENTS.md](./AGENTS.md)**。作業前に必ず読むこと。以下は要点のみ。

## このリポジトリの性質

TOPPERS FMP3 カーネルの**公開済みリリース物**を、日付順・タグ付きで再構成した pristine アーカイブ。通常の意味でのソースコード開発リポジトリではない — ビルド・lint・テストの概念はない。唯一の「作業」は `tools/` のスクリプトを使ったパッケージの取込（import）と検証。**ソースを手で編集しない**（AGENTS.md §2 の HARD RULES を参照）。

`RELEASES.csv` は現時点では取込例のみコメントアウトされた状態（実データ未投入）。実データを追記する際は日付昇順を厳守。

## 主なコマンド

```bash
tools/import_all.sh                 # RELEASES.csv を日付順に一括取込（dry-run）
tools/import_all.sh --execute       # 実行

tools/import_fmp3.sh indep  --date <YYYYMMDD> --version <3.M.N> <package.tar.gz>   # 個別パッケージ（非依存部）
tools/import_fmp3.sh simple --date <YYYYMMDD> --target <target> --tool <tool> <package.tar.gz>  # 簡易パッケージ（依存部のみ）

tools/new_major_branch.sh 3.2       # release/3.1 tip から release/3.2 を作成

tools/verify_release.sh <tag> <package.tar.*>   # タグ復元物と元パッケージのバイト差分検証（差分ゼロが期待）
```

手動 `git commit` / `git tag` は禁止。取込は必ず上記スクリプト経由。

## 構造上の要点

- `main`：運用ファイル（`tools/` `docs/` `README.md` `RELEASES.csv` など）のみ。FMP3 ソースは持たない。
- `release/3.M`：メジャーバージョンごとのブランチ。FMP3 ソース＋運用ファイルを持つ。新メジャーは前メジャー tip から分岐（履歴連続）。
- `tools/indep_paths.txt`：非依存部（`kernel/ include/ syssvc/ library/ cfg/`）のパス許可リスト。simple 取込ではこれらを除外（上書き防止）、indep 取込ではこれらのみ rm+copy。
- `tools/infra_paths.txt`：取込スクリプトが絶対に触れないリポジトリ運用ファイル一覧。
- `packages/`：取込元 tarball の置き場。Git 管理外（`.gitignore` 参照、`sha256` は `RELEASES.csv` で provenance 管理）。
- タグ規約：非依存部 `v3.M.N`、依存部 `<target>_<tool>-<date>`（例 `polarfire_soc_kit_gcc-20241224`）。タグは不変（派生版 `fmp3_core` が pin するため）。

## 完了条件（作業前後の自己確認）

作業前に AGENTS.md §9 の完了条件チェックリストを確認すること（コミット日付＝公開日、日付単調増加、`verify_release.sh` 差分ゼロ、タグ不変、派生版固有ファイル混入なし、等）。
