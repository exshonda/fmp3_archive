# AGENTS.md — FMP3 リリースアーカイブ 操作の正本

このリポジトリで作業する AI/人間は、**まずこの文書に従うこと**。`CLAUDE.md` 等は薄いポインタ。

---

## 1. このリポジトリの役割（と非役割）

- **役割**：TOPPERS で公開済みの FMP3 カーネル（個別パッケージ／簡易パッケージ）を、**日付順・タグ付きで再構成した pristine アーカイブ**。
- **役割**：将来の派生版（`fmp3_core`：CMake + Python cfg）の **pristine 上流**。派生版は別リポジトリからこのアーカイブのタグ/コミットを pin する。
- **非役割**：開発はしない（本流開発は内部 SVN）。**このリポジトリでソースを改変・機能追加しない**。

---

## 2. 絶対ルール（HARD RULES）— 違反禁止

1. **pristine 厳守**：FMP3 ソース（`kernel/ include/ syssvc/ library/ cfg/ arch/ target/` 等）を手で編集しない。中身は必ず**公開パッケージの取込**でのみ変化させる。
2. **派生版固有ファイルを混入させない**：`CMakeLists.txt` / `CMakePresets.json` / `cmake/` / Python cfg / `DIVERGENCE_MAP.md` / `AGENTS.md` 以外のエージェント設定などを**このアーカイブに置かない**（それらは派生版リポジトリ側）。
3. **日付順コミット**：取込は必ず公開日の昇順。過去日付の取込はスクリプトが中断する。手で順序を破らない。
4. **簡易パッケージ取込時は非依存部を除外**：`tools/indep_paths.txt` のパスは簡易パッケージから**取り込まない**（上書き防止）。非依存部は**個別パッケージからのみ**取り込む。
5. **タグは不変**：一度打ったタグをリネーム/削除/付け替えしない（派生版の pin が壊れる）。
6. **コミット日付＝公開日**：`GIT_AUTHOR_DATE` / `GIT_COMMITTER_DATE` をパッケージ公開日に固定（ツールが実施）。
7. **手動 `git commit` / `git tag` をしない**：必ず `tools/` のスクリプト経由で取り込む。

---

## 3. リポジトリ構成

```
.
├── AGENTS.md                 # ← この文書（操作の正本）
├── CLAUDE.md                 # 薄いポインタ
├── README.md
├── .gitattributes            # * -text（改行変換無効・バイト保持）
├── .gitignore
├── RELEASES.csv              # リリース台帳（取込の入力＝正）
├── docs/
│   ├── fmp3_git_management_plan.md   # 設計の全体像
│   └── consuming.md                  # 派生版 fmp3_core からの取り込み方
├── tools/
│   ├── import_fmp3.sh        # 単一パッケージ取込
│   ├── import_all.sh         # RELEASES.csv を日付順に一括取込（ブランチ自動作成）
│   ├── new_major_branch.sh   # release/3.(M+1) を前メジャー tip から作成
│   ├── verify_release.sh     # タグ復元物と元パッケージの差分検証
│   ├── indep_paths.txt       # 非依存部パス許可リスト
│   └── infra_paths.txt       # 取込で触れてはいけないリポジトリ運用ファイル
└── packages/                 # 取込元の tarball 置き場（Git 管理外）
```

- `main`：運用ファイル（tools/docs/README/RELEASES.csv）を持つ既定ブランチ。FMP3 ソースは持たない。
- `release/3.M`：`main`（最小メジャー）または前メジャー tip から分岐。FMP3 ソース＋運用ファイルを持つ。

---

## 4. ブランチモデル

- メジャーバージョン `M` ごとに `release/3.M`。
- 新メジャー `release/3.(M+1)` は **`release/3.M` の tip** から分岐（履歴連続）。`import_all.sh` が自動で行う。最小メジャーは `main` から分岐。
- 同一メジャー内では非依存部が `N` を進めても依存部は互換（バージョン規則より）。

---

## 5. タグ規約（annotated tag）

| 対象 | タグ | 例 |
|------|------|-----|
| 非依存部（個別パッケージ） | `v3.M.N` | `v3.1.0` |
| 依存部（簡易パッケージ） | `<target>_<tool>-<date>` | `polarfire_soc_kit_gcc-20241224` |

`date` は `YYYYMMDD`（公開日）。

---

## 6. 手順（コマンド）

### 6.1 一括取込（推奨・通常はこれ）

`RELEASES.csv` を日付順に処理し、ブランチ作成・取込・タグ付けまで自動化する。

```bash
tools/import_all.sh                 # 全件を dry-run 表示
tools/import_all.sh --execute       # 実行
```

### 6.2 単一パッケージの取込

```bash
# 個別パッケージ（非依存部）
git switch release/3.1
tools/import_fmp3.sh indep  --date 20240701 --version 3.1.0 \
    packages/pkg_fmp3-3.1.0.tar.gz

# 簡易パッケージ（依存部のみ・非依存部は自動除外）
git switch release/3.1
tools/import_fmp3.sh simple --date 20241224 \
    --target polarfire_soc_kit --tool gcc \
    packages/polarfire_soc_kit_gcc-20241224.tar.gz
```

同一ターゲットの再リリースでファイル削除を反映したい場合のみ `--target-paths` を付与：
```bash
tools/import_fmp3.sh simple --date 20250310 --target polarfire_soc_kit --tool gcc \
    --target-paths "arch/rv64/polarfire target/polarfire_soc_kit" \
    packages/polarfire_soc_kit_gcc-20250310.tar.gz
```

### 6.3 新メジャーブランチ

```bash
tools/new_major_branch.sh 3.2       # release/3.1 tip から release/3.2 を作成
```

### 6.4 検証

```bash
tools/verify_release.sh polarfire_soc_kit_gcc-20241224 \
    packages/polarfire_soc_kit_gcc-20241224.tar.gz
```
差分ゼロが期待。**非依存部に差分が出たら**、間にマイナー版があった等のシグナル。取込順・`indep_paths.txt` を見直す。

---

## 7. RELEASES.csv 形式（取込の正）

ヘッダ＋1リリース1行。`#` 始まりと空行は無視。**日付昇順で並べる**（`import_all.sh` 内でも安定ソートする）。

```csv
date,type,version,target,tool,package,sha256,branch,tag
20240701,indep,3.1.0,,,pkg_fmp3-3.1.0.tar.gz,,release/3.1,v3.1.0
20241224,simple,,polarfire_soc_kit,gcc,polarfire_soc_kit_gcc-20241224.tar.gz,,release/3.1,polarfire_soc_kit_gcc-20241224
```

- `type`：`indep`（個別＝非依存部）/ `simple`（簡易＝依存部）。
- `version`：`indep` のとき必須（`3.M.N`）。`simple` は空。
- `target` / `tool`：`simple` のとき必須。
- `package`：`packages/` 相対 or 絶対パス。
- `sha256`：任意（provenance）。空可。
- `branch`：`release/3.M`。
- `tag`：期待タグ（監査用。スクリプトが導出した実タグと突き合わせる）。

---

## 8. やってはいけないこと

- ソースの手編集、機能追加、フォーマット変更。
- 非依存部を簡易パッケージから取り込むこと。
- 日付順を破る取込／過去日付の後入れ（スクリプトが止めるが、`--force` 等で回避しない）。
- タグのリネーム/削除、履歴の rebase/amend（公開後）。
- 派生版のファイル（CMake / Python cfg 等）をここに置くこと。
- `packages/` の tarball をコミットすること（Git 管理外）。

---

## 9. 完了条件チェックリスト（PR/作業前に自己確認）

- [ ] 取込は `tools/` スクリプト経由のみ。手 commit/tag していない。
- [ ] コミット日付＝公開日になっている（`git log --format='%ad %s' --date=short`）。
- [ ] `simple` 取込で `indep_paths.txt` のパスに差分が出ていない（`git show --stat` で確認）。
- [ ] 日付が単調増加（`git log --date=short --format=%ad` が昇順）。
- [ ] `verify_release.sh` が差分ゼロ。
- [ ] `release/3.M` に派生版固有ファイルが無い。
- [ ] `RELEASES.csv` の `tag` と実タグが一致。
