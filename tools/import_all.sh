#!/usr/bin/env bash
#
# import_all.sh - RELEASES.csv を日付昇順に処理し、ブランチ作成・取込・タグ付けを自動化。
#
# 使い方:
#   tools/import_all.sh            # dry-run（何をするか表示のみ）
#   tools/import_all.sh --execute  # 実行
#
# CSV: date,type,version,target,tool,package,sha256,branch,tag
#   type=indep のとき version 必須、target/tool は空。
#   type=simple のとき target/tool 必須、version 空。
#
set -euo pipefail

die(){ echo "ERROR: $*" >&2; exit 1; }
log(){ echo "[all] $*" >&2; }

REPO_ROOT="$(git rev-parse --show-toplevel)" || die "git リポジトリ内で実行すること"
CSV="${REPO_ROOT}/RELEASES.csv"
IMPORT="${REPO_ROOT}/tools/import_fmp3.sh"
[ -f "$CSV" ]    || die "見つからない: $CSV"
[ -x "$IMPORT" ] || die "実行不可: $IMPORT （chmod +x したか？）"

EXECUTE=0
[ "${1:-}" = "--execute" ] && EXECUTE=1

# CSV を読み、コメント/空行/ヘッダを除去し、date で安定ソート
mapfile -t ROWS < <(
  grep -vE '^\s*(#|$)' "$CSV" \
    | grep -vE '^date,type,' \
    | sort --stable -t, -k1,1
)
[ "${#ROWS[@]}" -gt 0 ] || die "RELEASES.csv に有効な行が無い"

# release/3.M ブランチを必要に応じて作成（前メジャー tip / main から分岐）
ensure_branch(){
  local br="$1"
  git show-ref --verify --quiet "refs/heads/${br}" && return 0
  [[ "$br" =~ ^release/3\.([0-9]+)$ ]] || die "branch 形式が不正: ${br}"
  local major="${BASH_REMATCH[1]}" base=""
  local parent="release/3.$((major-1))"
  if git show-ref --verify --quiet "refs/heads/${parent}"; then
    base="$parent"
  elif git show-ref --verify --quiet "refs/heads/main"; then
    base="main"
  fi
  if [ "$EXECUTE" -eq 1 ]; then
    if [ -n "$base" ]; then git branch "$br" "$base"; else git branch "$br"; fi
  fi
  log "create branch ${br}${base:+ (from ${base})}"
}

last_date=""
for row in "${ROWS[@]}"; do
  IFS=, read -r date type version target tool package sha256 branch tag <<<"$row"
  [ -n "$date" ] && [ -n "$type" ] && [ -n "$package" ] && [ -n "$branch" ] \
    || die "行の必須項目が欠落: $row"

  # グローバル日付昇順の健全性チェック（ソート済みだが二重確認）
  if [ -n "$last_date" ] && [ "$date" \< "$last_date" ]; then
    die "日付が降順: ${date} < ${last_date}"
  fi
  last_date="$date"

  # パッケージパス解決
  pkg="$package"
  [[ "$pkg" = /* ]] || pkg="${REPO_ROOT}/${pkg}"

  # 既にタグ済みの行はスキップ（再実行時の冪等性。CSV は追記専用の恒久台帳のため）
  if [ -n "${tag}" ] && git rev-parse -q --verify "refs/tags/${tag}" >/dev/null; then
    log "skip (既存タグ ${tag}): ${date} ${type} ${target}${version}"
    continue
  fi

  ensure_branch "$branch"

  args=( "$type" --date "$date" )
  case "$type" in
    indep)  args+=( --version "$version" );;
    simple) args+=( --target "$target" --tool "$tool" );;
    *)      die "不明な type: ${type}";;
  esac
  args+=( "$pkg" )

  if [ "$EXECUTE" -eq 1 ]; then
    log "switch ${branch}; import ${type} ${date} ${target}${version}"
    git switch "$branch" >/dev/null
    [ -f "$pkg" ] || die "パッケージが無い: ${pkg}"
    "$IMPORT" "${args[@]}"
    # 期待タグとの突き合わせ
    if [ -n "${tag}" ] && ! git rev-parse -q --verify "refs/tags/${tag}" >/dev/null; then
      die "期待タグ ${tag} が作られていない（CSV と導出タグの不一致）"
    fi
  else
    echo "DRY-RUN: git switch ${branch} && tools/import_fmp3.sh ${args[*]}"
  fi
done

if [ "$EXECUTE" -eq 1 ]; then
  log "完了。git log --all --date=short --format='%d %ad %s' で確認を。"
else
  log "dry-run 完了。実行は --execute を付ける。"
fi
