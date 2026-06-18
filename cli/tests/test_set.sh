#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source ./test_helpers.sh
# test_set.sh

test_set_active() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket set --active >/dev/null 2>&1
  local out
  out=$(kilket check --active 2>&1)
  assert_contains "$out" "is active" "set --active makes task active"
}

test_set_deactive() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket set --active >/dev/null 2>&1
  kilket set --deactive >/dev/null 2>&1
  local out
  out=$(kilket check --deactive 2>&1)
  assert_contains "$out" "is deactive" "set --deactive makes task deactive"
}

test_set_active_then_check_not_deactive() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket set --active >/dev/null 2>&1
  local out
  out=$(kilket check --deactive 2>&1)
  assert_contains "$out" "not deactive" "active task is not deactive"
}

test_set_deactive_then_check_not_active() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket set --deactive >/dev/null 2>&1
  local out
  out=$(kilket check --active 2>&1)
  assert_contains "$out" "not active" "deactive task is not active"
}
