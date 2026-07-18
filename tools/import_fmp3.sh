#!/usr/bin/env bash
#
# import_fmp3.sh - リリース済み FMP3 パッケージを Git に取り込む
#
# 使い方:
#   個別パッケージ（ターゲット非依存部）:
#     tools/import_fmp3.sh indep  --date YYYYMMDD --version 3.M.N <package.tar.*|dir>
#
#   簡易パッケージ（ターゲット依存部のみ。非依存部は自動除外）:
#     tools/import_fmp3.sh simple --date YYYYMMDD --target <name> --tool <tool> \
#                    [--target-paths "arch/core/chip target/board"] <package.tar.*|dir>
#
# 前提:
#   - カレントが対象 Git リポジトリ、かつ取込対象の release/3.M を switch 済み。
#   - tools/indep_paths.txt（非依存部許可リスト）と tools/infra_paths.txt（保護対象）がある。
#
set -euo pipefail

die(){ echo "ERROR: $*" >&2; exit 1; }
log(){ echo "[import] $*" >&2; }

REPO_ROOT="$(git rev-parse --show-toplevel)" || die "git リポジトリ内で実行すること"
INDEP_LIST="${REPO_ROOT}/tools/indep_paths.txt"
INFRA_LIST="${REPO_ROOT}/tools/infra_paths.txt"
[ -f "$INDEP_LIST" ] || die "見つからない: $INDEP_LIST"
[ -f "$INFRA_LIST" ] || die "見つからない: $INFRA_LIST"

MODE="${1:-}"; shift || die "mode 未指定 (indep|simple)"

DATE=""; VERSION=""; TARGET=""; TOOL=""; TARGET_PATHS=""; PKG=""
while [ $# -gt 0 ]; do
  case "$1" in
    --date)          DATE="$2"; shift 2;;
    --version)       VERSION="$2"; shift 2;;
    --target)        TARGET="$2"; shift 2;;
    --tool)          TOOL="$2"; shift 2;;
    --target-paths)  TARGET_PATHS="$2"; shift 2;;
    -*)              die "不明なオプション: $1";;
    *)               PKG="$1"; shift;;
  esac
done

[ -n "$DATE" ] || die "--date 必須 (YYYYMMDD)"
[ -n "$PKG" ]  || die "パッケージ(tar または展開済みディレクトリ)を指定すること"
[[ "$DATE" =~ ^[0-9]{8}$ ]] || die "--date は YYYYMMDD 形式"
command -v tar >/dev/null || die "tar が必要"

# --- 日付順チェック（共有ファイルの上書き/履歴退行を防止） ---
# 直近の「取込コミット」のみを基準にする（scaffold 等の運用コミットは無視）。
new_epoch="$(date -d "${DATE}" +%s)"
last_epoch="$(git -C "$REPO_ROOT" log -1 --grep='^Import ' --format=%at 2>/dev/null || echo 0)"
last_epoch="${last_epoch:-0}"
if [ "$new_epoch" -lt "$last_epoch" ]; then
  die "日付 ${DATE} は現在ブランチの直近取込($(date -d @${last_epoch} +%Y%m%d))より過去。日付昇順で取り込むこと。"
fi

# --- 展開 ---
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
if [ -d "$PKG" ]; then
  cp -a "$PKG/." "$TMP/"
else
  tar xf "$PKG" -C "$TMP"
fi
# 単一トップディレクトリなら潜る
SRC="$TMP"
mapfile -t _top < <(find "$TMP" -mindepth 1 -maxdepth 1)
if [ "${#_top[@]}" -eq 1 ] && [ -d "${_top[0]}" ]; then SRC="${_top[0]}"; fi
log "source root: $SRC"

# リストを配列へ（コメント・空行除去、末尾スラッシュ除去）
readlist(){ grep -vE '^\s*(#|$)' "$1" | sed 's:/*$::'; }
mapfile -t INDEP < <(readlist "$INDEP_LIST")
mapfile -t INFRA < <(readlist "$INFRA_LIST")

case "$MODE" in
  #############################################################
  indep)  # ターゲット非依存部（カーネル本体 + cfg）を rm+copy（削除反映）
    [ -n "$VERSION" ] || die "--version 必須 (3.M.N)"
    [[ "$VERSION" =~ ^3\.[0-9]+\.[0-9]+$ ]] || die "--version は 3.M.N 形式"
    for p in "${INDEP[@]}"; do
      rm -rf "${REPO_ROOT:?}/${p}"
      if [ -e "${SRC}/${p}" ]; then
        mkdir -p "$(dirname "${REPO_ROOT}/${p}")"
        cp -a "${SRC}/${p}" "${REPO_ROOT}/${p}"
      else
        log "警告: パッケージに非依存部パスが無い: ${p}"
      fi
    done
    MSG="Import individual package (target-independent) v${VERSION} (${DATE})"
    TAG="v${VERSION}"
    ;;

  #############################################################
  simple)  # ターゲット依存部のみを overlay（非依存部・運用ファイルは除外）
    [ -n "$TARGET" ] || die "--target 必須"
    [ -n "$TOOL" ]   || die "--tool 必須"

    # 同一ターゲット再リリースの削除反映（指定パスのみ rm。共有/他ターゲットは消さない）
    if [ -n "$TARGET_PATHS" ]; then
      for p in $TARGET_PATHS; do rm -rf "${REPO_ROOT:?}/${p}"; done
    fi

    # 非依存部・運用ファイルを除外して overlay（tar パイプ。rsync 非依存で移植性が高い）
    EXCLUDES=()
    for p in "${INDEP[@]}" "${INFRA[@]}"; do
      EXCLUDES+=("--exclude=./${p}")
    done
    ( cd "$SRC" && tar cf - "${EXCLUDES[@]}" . ) | ( cd "$REPO_ROOT" && tar xf - )

    MSG="Import simple package ${TARGET}_${TOOL} (${DATE})"
    TAG="${TARGET}_${TOOL}-${DATE}"
    ;;

  *) die "不明な mode: $MODE (indep|simple)";;
esac

# --- 念のため：非依存部保護（simple で indep が変化していないか検査） ---
if [ "$MODE" = "simple" ]; then
  git -C "$REPO_ROOT" add -A
  for p in "${INDEP[@]}"; do
    if ! git -C "$REPO_ROOT" diff --cached --quiet -- "$p" 2>/dev/null; then
      die "simple 取込で非依存部が変化した: ${p}（indep_paths.txt / パッケージ内容を確認）"
    fi
  done
else
  git -C "$REPO_ROOT" add -A
fi

# --- コミット & タグ（日付を公開日に固定） ---
export GIT_AUTHOR_DATE="${DATE:0:4}-${DATE:4:2}-${DATE:6:2}T00:00:00 +0900"
export GIT_COMMITTER_DATE="$GIT_AUTHOR_DATE"

if git -C "$REPO_ROOT" diff --cached --quiet; then
  log "変更なし。コミットをスキップ: $MSG"
else
  git -C "$REPO_ROOT" commit -q -m "$MSG"
  log "commit: $MSG"
fi

if git -C "$REPO_ROOT" rev-parse -q --verify "refs/tags/${TAG}" >/dev/null; then
  die "タグが既存: ${TAG}（タグは不変。付け替え禁止）"
fi
git -C "$REPO_ROOT" tag -a "$TAG" -m "$MSG"
log "tag: ${TAG}"
