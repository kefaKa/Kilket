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
