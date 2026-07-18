#!/usr/bin/env bash
#
# verify_release.sh - 指定タグの Git 復元物と、元パッケージの差分を検証する。
#
# 使い方:
#   tools/verify_release.sh <tag> <package.tar.*|dir>
#
# 動作:
#   元パッケージに含まれる各ファイルについて、同一パスを `git show <tag>:<path>` と
#   バイト比較する。差分（不一致・欠落）を列挙。差分ゼロが期待。
#   ※ アーカイブは複数ターゲットの累積ツリーなので、比較は「元パッケージ側のファイル集合」に限定する。
#
set -euo pipefail
die(){ echo "ERROR: $*" >&2; exit 1; }

REPO_ROOT="$(git rev-parse --show-toplevel)" || die "git リポジトリ内で実行すること"
TAG="${1:-}"; PKG="${2:-}"
[ -n "$TAG" ] && [ -n "$PKG" ] || die "使い方: verify_release.sh <tag> <package>"
git -C "$REPO_ROOT" rev-parse -q --verify "refs/tags/${TAG}" >/dev/null \
  || die "タグが無い: ${TAG}"

INDEP_LIST="${REPO_ROOT}/tools/indep_paths.txt"
mapfile -t INDEP < <(grep -vE '^\s*(#|$)' "$INDEP_LIST" 2>/dev/null | sed 's:/*$::')
is_indep(){ local p="$1" i; for i in "${INDEP[@]}"; do [[ "$p" == "$i" || "$p" == "$i"/* ]] && return 0; done; return 1; }

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
if [ -d "$PKG" ]; then cp -a "$PKG/." "$TMP/"; else tar xf "$PKG" -C "$TMP"; fi
SRC="$TMP"
mapfile -t _top < <(find "$TMP" -mindepth 1 -maxdepth 1)
if [ "${#_top[@]}" -eq 1 ] && [ -d "${_top[0]}" ]; then SRC="${_top[0]}"; fi

dep_diff=0; indep_diff=0; missing=0; checked=0
while IFS= read -r -d '' f; do
  rel="${f#"$SRC"/}"
  checked=$((checked+1))
  if ! git -C "$REPO_ROOT" cat-file -e "${TAG}:${rel}" 2>/dev/null; then
    if is_indep "$rel"; then
      echo "INDEP-MISSING: ${rel}"   # 非依存部は個別P由来。この tag 時点に該当版が無い可能性
    else
      echo "MISSING in git: ${rel}"; missing=$((missing+1))
    fi
    continue
  fi
  if ! git -C "$REPO_ROOT" show "${TAG}:${rel}" | cmp -s - "$f"; then
    if is_indep "$rel"; then
      echo "INDEP-DIFF (§9.2 signal): ${rel}"; indep_diff=$((indep_diff+1))
    else
      echo "DIFF: ${rel}"; dep_diff=$((dep_diff+1))
    fi
  fi
done < <(find "$SRC" -type f -print0)

echo "----"
echo "checked=${checked} dep_diff=${dep_diff} indep_diff=${indep_diff} missing=${missing}"
if [ "$dep_diff" -eq 0 ] && [ "$missing" -eq 0 ] && [ "$indep_diff" -eq 0 ]; then
  echo "OK: 完全一致"; exit 0
elif [ "$dep_diff" -eq 0 ] && [ "$missing" -eq 0 ]; then
  # 依存部は一致。非依存部のみ差分 → 個別P と簡易P同梱の非依存部がズレている（§9.2）
  echo "WARN: 依存部は一致。非依存部のみ差分（間にマイナー版があった等のシグナル。取込順・indep_paths.txt を確認）"
  exit 2
else
  echo "NG: 依存部に差分/欠落あり"; exit 1
fi
