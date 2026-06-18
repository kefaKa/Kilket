#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source test_helpers.sh

# --- add --path ---

test_add_path_appears_in_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  mkdir -p subdir
  kilket add --path ./subdir >/dev/null 2>&1
  local out
  out=$(kilket list --paths 2>&1)
  assert_contains "$out" "subdir" "add --path shows up in list --paths"
}

test_add_path_nonexistent_fails() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out
  out=$(kilket add --path ./does_not_exist 2>&1)
  assert_contains "$out" "Error:" "add --path on nonexistent path errors"
}

test_add_duplicate_path_no_duplicate() {
  fresh_dir
  kilket init >/dev/null 2>&1
  mkdir -p dup
  kilket add --path ./dup >/dev/null 2>&1
  kilket add --path ./dup >/dev/null 2>&1
  local out count
  out=$(kilket list --paths 2>&1)
  count=$(grep -o "dup" <<< "$out" | wc -l)
  assert_exit 2 "$count" "adding the same path twice does not duplicate"
}
# --- add --command ---

test_add_command_appears_in_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --command "echo hi" >/dev/null 2>&1
  local out
  out=$(kilket list --commands 2>&1)
  assert_contains "$out" "echo hi" "add --command shows up in list --commands"
}

test_add_duplicate_command_fails() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --command "echo hi" >/dev/null 2>&1
  local out
  out=$(kilket add --command "echo hi" 2>&1)
  assert_contains "$out" "Error:" "adding the same command twice errors"
}

# --- add --ignored-path ---

test_add_ignored_path_appears_in_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --ignored-path .git >/dev/null 2>&1
  local out
  out=$(kilket list --ignored 2>&1)
  assert_contains "$out" ".git" "add --ignored-path shows up in list --ignored"
}

test_add_duplicate_ignored_path_fails() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --ignored-path .git >/dev/null 2>&1
  local out
  out=$(kilket add --ignored-path .git 2>&1)
  assert_contains "$out" "Error:" "adding the same ignored-path twice errors"
}

# --- add --ignored-pattern ---

test_add_ignored_pattern_appears_in_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --ignored-pattern "*.o" >/dev/null 2>&1
  local out
  out=$(kilket list --ignored 2>&1)
  assert_contains "$out" "*.o" "add --ignored-pattern shows up in list --ignored"
}

test_add_duplicate_ignored_pattern_fails() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --ignored-pattern "*.o" >/dev/null 2>&1
  local out
  out=$(kilket add --ignored-pattern "*.o" 2>&1)
  assert_contains "$out" "Error:" "adding the same ignored-pattern twice errors"
}
