#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

# test_remove.sh

# --- remove --path ---

test_remove_path_removes_from_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local outside_dir
  outside_dir=$(mktemp -d)
  kilket add --path "$outside_dir" >/dev/null 2>&1
  kilket remove --path "$outside_dir" >/dev/null 2>&1
  local out
  out=$(kilket list --paths 2>&1)
  assert_not_contains "$out" "$outside_dir" "remove --path removes externally-added dir from list --paths"
  rm -rf "$outside_dir"
}
test_remove_nonexistent_path_errors() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out
  out=$(kilket remove --path "nonexistent path" 2>&1)
  assert_contains "$out" "Error:" "removing a path never added errors"
}

# --- remove --command ---

test_remove_command_removes_from_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --command "echo bye" >/dev/null 2>&1
  kilket remove --command "echo bye" >/dev/null 2>&1
  local out
  out=$(kilket list --commands 2>&1)
  assert_not_contains "$out" "echo bye" "remove --command removes it from list --commands"
}

test_remove_nonexistent_command_errors() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out
  out=$(kilket remove --command "nonexistent command" 2>&1)
  assert_contains "$out" "Error:" "removing a command never added errors"
}

# --- remove --on-success ---

test_remove_on_success_removes_from_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --on-success "echo success" >/dev/null 2>&1
  kilket remove --on-success "echo success" >/dev/null 2>&1
  local out
  out=$(kilket list --on-success 2>&1)
  assert_not_contains "$out" "echo success" "remove --on-success removes it from list --on-success"
}

test_remove_nonexistent_on_success_errors() {
  fresh_dir
  kilket init >/dev/null 2>&1
  local out
  out=$(kilket remove --on-success "nonexistent" 2>&1)
  assert_contains "$out" "Error:" "removing on-success never added errors"
}

# --- remove --on-failure ---

test_remove_on_failure_removes_from_list() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --on-failure "echo failure" >/dev/null 2>&1
  kilket remove --on-failure "echo failure" >/dev/null 2>&1
  local out
  out=$(kilket list --on-failure 2>&1)
  assert_not_contains "$out" "echo failure" "remove --on-failure removes it from list --on-failure"
}
