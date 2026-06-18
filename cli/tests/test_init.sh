#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source test_helpers.sh

# --- init ---

test_init_creates_task() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local code=$?
  assert_exit 0 "$code" "init creates a task"

  local files
  files=$(ls "$KILKET_CONFIG_DIR_TEST" 2>/dev/null)
  if [[ -z "$files" ]]; then
    FAIL=$((FAIL+1))
    echo "FAIL: init creates a task (no config file written)"
  else
    PASS=$((PASS+1))
  fi
}

test_init_twice_fails() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out
  out=$(kilket init 2>&1)
  assert_contains "$out" "Error:" "init twice should print an Error"
}
