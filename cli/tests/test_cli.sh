#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")" || exit 1

PASS=0
FAIL=0

source ./test_add.sh
source ./test_init.sh
source ./test_list.sh
source ./test_run.sh
source ./test_remove.sh
source ./test_set.sh


# -- TEST INIT --
test_init_creates_task
test_init_twice_fails

# -- TEST ADD --
test_add_path_appears_in_list
test_add_path_nonexistent_fails
test_add_duplicate_path_no_duplicate
test_add_command_appears_in_list
test_add_duplicate_command_fails
test_add_ignored_path_appears_in_list
test_add_duplicate_ignored_path_fails
test_add_ignored_pattern_appears_in_list
test_add_duplicate_ignored_pattern_fails
test_ignored_path_prevents_add_from_listing
test_add_with_no_flags
test_add_empty_command
test_add_on_success_appears_in_list
test_add_duplicate_on_success_no_duplicate
test_add_on_failure_appears_in_list
test_add_duplicate_on_failure_no_duplicate
test_add_multiple_flags_at_once

# -- TEST LIST --

test_list_no_flags_is_noop
test_list_tasks_works_without_init_in_cwd
test_list_paths_fails_without_init_in_cwd
test_list_ignored_shows_defaults
test_list_combined_flags_when_initialized
