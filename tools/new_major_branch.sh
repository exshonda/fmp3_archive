#!/usr/bin/env bash
#
# new_major_branch.sh - release/3.(M) を前メジャー tip（または main）から作成。
#
# 使い方:
#   tools/new_major_branch.sh 3.2      # release/3.1 の tip から release/3.2 を作成
#
set -euo pipefail
die(){ echo "ERROR: $*" >&2; exit 1; }

REPO_ROOT="$(git rev-parse --show-toplevel)" || die "git リポジトリ内で実行すること"
VER="${1:-}"
[[ "$VER" =~ ^3\.([0-9]+)$ ]] || die "引数は 3.M 形式（例: 3.2）"
major="${BASH_REMATCH[1]}"
br="release/${VER}"
parent="release/3.$((major-1))"

git -C "$REPO_ROOT" show-ref --verify --quiet "refs/heads/${br}" \
  && die "既に存在: ${br}"

if git -C "$REPO_ROOT" show-ref --verify --quiet "refs/heads/${parent}"; then
  base="$parent"
elif git -C "$REPO_ROOT" show-ref --verify --quiet "refs/heads/main"; then
  base="main"
else
  die "分岐元が無い（${parent} も main も無い）"
fi

git -C "$REPO_ROOT" branch "$br" "$base"
echo "created ${br} from ${base}"
