#!/usr/bin/env bash
set -uo pipefail

PASS=0
FAIL=0

source ./test_helpers.sh
# test_run.sh

test_run_default_starts_and_exits_on_sigint() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --command "echo hi" >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)

  kilket run > "$outfile" 2>&1 &
  local pid=$!

  sleep 1
  kill -INT "$pid" 2>/dev/null

  # Give it up to 3 seconds to exit gracefully; force-kill if it doesn't.
  local waited=0
  while kill -0 "$pid" 2>/dev/null && [[ $waited -lt 3 ]]; do
    sleep 1
    waited=$((waited+1))
  done
  if kill -0 "$pid" 2>/dev/null; then
    kill -9 "$pid" 2>/dev/null
    FAIL=$((FAIL+1))
    echo "FAIL: run did not exit on SIGINT within 3s, force-killed"
  else
    PASS=$((PASS+1))
  fi

  local out
  out=$(cat "$outfile")
  assert_contains "$out" "Exiting safely" "run prints clean exit message on SIGINT"
  rm -f "$outfile"
}

test_run_all_starts_and_exits_on_sigint() {
  fresh_dir
  kilket init >/dev/null 2>&1
  kilket add --command "echo hi" >/dev/null 2>&1

  local outfile
  outfile=$(mktemp)

  kilket run --all > "$outfile" 2>&1 &
  local pid=$!

  sleep 1
  kill -INT "$pid"
  wait "$pid" 2>/dev/null
