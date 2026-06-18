#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source ./test_helpers.sh

# test_list.sh

test_list_no_flags_is_noop() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out code
  out=$(kilket list 2>&1)
  code=$?
  assert_exit 0 "$code" "list with no flags exits 0"
  if [[ -z "$out" ]]; then
    PASS=$((PASS+1))
  else
    FAIL=$((FAIL+1))
    echo "FAIL: list with no flags should print nothing"
    echo "  actual output: $out"
  fi
}

test_list_tasks_works_without_init_in_cwd() {
  fresh_dir
  # init in one dir, then list --tasks from a different, uninitialized dir
  kilket init >/dev/null 2>&1
  local task_dir
  task_dir=$(pwd)
  mkdir -p ../other_dir
  cd ../other_dir || exit 1
  local out
  out=$(kilket list --tasks 2>&1)
  assert_contains "$out" "$task_dir" "list --tasks shows tasks regardless of cwd"
}
