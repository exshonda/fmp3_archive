# fmp3_core からの取り込み方法（consuming `fmp3_archive`）

`fmp3_archive`（本リポジトリ＝pristine 上流）を、派生版 `fmp3_core`（CMake + Python cfg、`asp3_core` 相当）から取り込む手順。

---

## 1. 位置づけ

```
fmp3_archive (pristine 上流)          fmp3_core (派生版)
  release/3.M                          ├─ CMake / Python cfg / CI …（派生版固有）
  tags: v3.M.N                         ├─ UPSTREAM_VERSION      … どの v3.M.N か
        <target>_<tool>-<date>   ─pin→ ├─ UPSTREAM_PRISTINE.txt … 固定コミット SHA
                                       ├─ DIVERGENCE_MAP.md     … pristine への改変台帳
                                       └─ fmp3/  (pristine を submodule or vendor)
```

- `fmp3_core` は `fmp3_archive` の**特定タグ/コミットを pin** して取り込む。
- `fmp3_core` は全ターゲットの部分集合だけ対応すればよい（archive のツリーは累積なので、必要ターゲットだけ使う）。

---

## 2. pin 対象の選び方

- **非依存部**：`v3.M.N` タグを一次参照（→ `UPSTREAM_VERSION` に記録）。
- **依存部**：`fmp3_core` が対応するターゲット分。archive のツリーは「非依存部 + その日付までの全ターゲット依存部」を累積で持つので、**1つのコミット/タグを指すだけ**で必要なファイルが揃う。
- **推奨 pin**：対応ターゲットの最新 `<target>_<tool>-<date>` タグ、または明示コミット。SHA を `UPSTREAM_PRISTINE.txt` に記録（`asp3_core` と同じ方式）。

> 例：`v3.1.0`（非依存部）＋ `polarfire_soc_kit_gcc-20241224` までの依存部が欲しい
> → その簡易パッケージタグ（＝そのコミット）を pin すれば両方入る。

---

## 3. 方式A：submodule（推奨・派生が additive な場合）

pristine を**読み取り専用**で取り込み、CMake・Python cfg・改変は `fmp3_core` 側に置く。pristine ファイルは編集しない。

```bash
# fmp3_core リポジトリで
git submodule add <fmp3_archive-url> fmp3
cd fmp3
git fetch --tags
git checkout polarfire_soc_kit_gcc-20241224   # pin 点（タグ＝コミット）
cd ..
git add .gitmodules fmp3
git commit -m "Pin fmp3 upstream: v3.1.0 / polarfire_soc_kit_gcc-20241224"

# pin を記録
git -C fmp3 rev-parse HEAD > UPSTREAM_PRISTINE.txt
echo "3.1.0" > UPSTREAM_VERSION
```

- CMake からは `fmp3/kernel/…`, `fmp3/arch/<core>/<chip>/…`, `fmp3/target/<board>/…` を参照。
- pristine の `cfg/` は**使わない**。Python cfg は `fmp3_core` 側の別ディレクトリ（例 `configurator/`）に置き、CMake から呼ぶ。`DIVERGENCE_MAP.md` に「cfg を Python 実装へ置換」と記録。
- pristine への小さな改変（フック・修正）が要る場合は、`fmp3/` を直接編集せず **`patches/*.patch`** か**オーバレイ**（同名ファイルを `fmp3_core` 側に置き、CMake が pristine より優先）で表現。
- **更新**：`cd fmp3 && git fetch --tags && git checkout <新タグ>` → `patches` 再適用 → `UPSTREAM_*` / `DIVERGENCE_MAP.md` 更新 → CI。

長所：pristine と派生の分離が明確・pin が SHA で厳密・pristine を履歴で二重管理しない。
短所：pristine を in-place 編集できない（＝改変は patch/overlay に寄せる規律が要る）。

---

## 4. 方式B：vendor import（pristine を直接編集したい場合・`asp3_core` 実績方式）

pristine を `fmp3_core` 内に**コピーして取り込み**、in-place 編集して `DIVERGENCE_MAP.md` で乖離管理する。

`fmp3_core` 側に置くスクリプト例（`tools/import_upstream.sh`）：

```bash
#!/usr/bin/env bash
# fmp3_core/tools/import_upstream.sh
# fmp3_archive の指定 ref から pristine を取り込み、SHA を記録する。
set -euo pipefail
ARCHIVE_URL="${ARCHIVE_URL:-<fmp3_archive-url>}"
REF="${1:?usage: import_upstream.sh <tag-or-commit> [target ...]}"; shift
DEST="fmp3"                       # pristine の置き場（fmp3_core 直下）

tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
git clone --no-checkout "$ARCHIVE_URL" "$tmp/a"
git -C "$tmp/a" fetch --tags
sha="$(git -C "$tmp/a" rev-parse "$REF")"

# ref のツリーを取り出し（必要なら対象ターゲットだけに絞る）
git -C "$tmp/a" archive "$REF" | tar -x -C "$tmp/tree" 2>/dev/null || { mkdir -p "$tmp/tree"; git -C "$tmp/a" archive "$REF" | tar -x -C "$tmp/tree"; }

# 非依存部＋対応ターゲットのみ反映する運用にするなら、ここで rsync/tar の include を絞る
mkdir -p "$DEST"
tar -C "$tmp/tree" -cf - . | tar -C "$DEST" -xf -

echo "$sha" > UPSTREAM_PRISTINE.txt
echo "取り込み完了: $REF ($sha) -> $DEST/"
echo "→ 3-way マージで自分の改変を再適用し、DIVERGENCE_MAP.md を更新すること。"
```

- 取り込み後、自分の改変を **3-way マージ**（前回 pristine → 今回 pristine → 現行）で再適用。
- 乖離は必ず `DIVERGENCE_MAP.md` に1行1件で記録（ファイル・理由・上流報告有無）。

長所：pristine を自由に編集でき、`asp3_core` と同じ運用がそのまま乗る。
短所：pin が SHA ファイル管理（submodule ほど機構的に強制されない）・更新時に手マージが要る。

---

## 5. どちらを選ぶか

| 状況 | 方式 |
|------|------|
| `kernel/` 等を素のまま使い、CMake・Python cfg を**上に足すだけ** | **A（submodule）** |
| `kernel/`・`arch/` に**手を入れる**（トレースフック・上流バグ修正など）必要がある | **B（vendor import）** |

`asp3_core` は乖離を持つため B 相当。`fmp3_core` も同程度に pristine を触るなら B、CMake/cfg 追加が中心で pristine をほぼ触らないなら A が綺麗。

---

## 6. 共通の約束（`fmp3_archive` 側の pin 契約）

- タグは**不変**（リネーム・削除・付け替え禁止）。`fmp3_core` の pin が壊れる。
- 依存部の共有ファイルは**日付順コミット**で単調（`fmp3_core` のマージ差分が意味を持つ）。
- pin は「非依存部 `v3.M.N`（`UPSTREAM_VERSION`）＋ 全ツリー SHA（`UPSTREAM_PRISTINE.txt`）」で表現する。
