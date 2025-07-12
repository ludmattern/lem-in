# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_suite.sh                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lmattern <lmattern@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/11 15:36:16 by lmattern          #+#    #+#              #
#    Updated: 2025/07/12 10:58:13 by lmattern         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/bin/bash

# ============================================================================
# LEM-IN PROFESSIONAL TEST SUITE
# Comprehensive testing framework for lem-in parser validation
# ============================================================================

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
RESET='\033[0m'

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Configuration
BINARY="./lem-in"
TIMEOUT=10 # seconds
VERBOSE=false

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

print_header() {
	printf "${BLUE}===========================================${RESET}\n"
	printf "${WHITE}LEM-IN PROFESSIONAL TEST SUITE${RESET}\n"
	printf "${BLUE}===========================================${RESET}\n"
	printf "${CYAN}Testing binary: ${BINARY}${RESET}\n"
	printf "${CYAN}Timeout: ${TIMEOUT}s per test${RESET}\n"
	printf "\n"
}

print_section() {
	printf "\n${PURPLE}[%s]${RESET}\n" "$1"
	printf "${PURPLE}$(printf '=%.0s' {1..50})${RESET}\n"
}

run_test() {
	local test_name="$1"
	local input="$2"
	local should_pass="$3"    # true/false
	local expected_error="$4" # optional expected error message

	TOTAL_TESTS=$((TOTAL_TESTS + 1))

	printf "%-40s " "$test_name:"

	# Run the test with timeout
	local output
	local exit_code
	output=$(printf "%b\n" "$input" | timeout $TIMEOUT $BINARY 2>&1)
	exit_code=$?

	# Check timeout
	if [ $exit_code -eq 124 ]; then
		printf "${RED}[TIMEOUT]${RESET}\n"
		FAILED_TESTS=$((FAILED_TESTS + 1))
		return 1
	fi

	# Check result
	if [ "$should_pass" = "true" ]; then
		if [ $exit_code -eq 0 ]; then
			printf "${GREEN}[PASS]${RESET}\n"
			PASSED_TESTS=$((PASSED_TESTS + 1))
		else
			printf "${RED}[FAIL] (expected success)${RESET}\n"
			if [ "$VERBOSE" = "true" ]; then
				printf "${YELLOW}Output: %s${RESET}\n" "$output"
			fi
			FAILED_TESTS=$((FAILED_TESTS + 1))
		fi
	else
		if [ $exit_code -ne 0 ]; then
			# Check for specific error if provided
			if [ -n "$expected_error" ]; then
				if echo "$output" | grep -q "$expected_error"; then
					printf "${GREEN}[PASS] (correct error)${RESET}\n"
					PASSED_TESTS=$((PASSED_TESTS + 1))
				else
					printf "${YELLOW}[PASS] (wrong error message)${RESET}\n"
					if [ "$VERBOSE" = "true" ]; then
						printf "${YELLOW}Expected: %s${RESET}\n" "$expected_error"
						printf "${YELLOW}Got: %s${RESET}\n" "$output"
					fi
					PASSED_TESTS=$((PASSED_TESTS + 1))
				fi
			else
				printf "${GREEN}[PASS]${RESET}\n"
				PASSED_TESTS=$((PASSED_TESTS + 1))
			fi
		else
			printf "${RED}[FAIL] (expected failure)${RESET}\n"
			if [ "$VERBOSE" = "true" ]; then
				printf "${YELLOW}Output: %s${RESET}\n" "$output"
			fi
			FAILED_TESTS=$((FAILED_TESTS + 1))
		fi
	fi
}

run_file_test() {
	local file_path="$1"
	local should_pass="$2"
	local test_name="$(basename "$file_path")"

	if [ ! -f "$file_path" ]; then
		printf "${YELLOW}[SKIP] (file not found): %s${RESET}\n" "$test_name"
		return
	fi

	TOTAL_TESTS=$((TOTAL_TESTS + 1))
	printf "%-40s " "$test_name:"

	local output
	local exit_code
	output=$(timeout $TIMEOUT $BINARY <"$file_path" 2>&1)
	exit_code=$?

	if [ $exit_code -eq 124 ]; then
		printf "${RED}[TIMEOUT]${RESET}\n"
		FAILED_TESTS=$((FAILED_TESTS + 1))
		return 1
	fi

	if [ "$should_pass" = "true" ]; then
		if [ $exit_code -eq 0 ]; then
			printf "${GREEN}[PASS]${RESET}\n"
			PASSED_TESTS=$((PASSED_TESTS + 1))
		else
			printf "${RED}[FAIL]${RESET}\n"
			FAILED_TESTS=$((FAILED_TESTS + 1))
		fi
	else
		if [ $exit_code -ne 0 ]; then
			printf "${GREEN}[PASS]${RESET}\n"
			PASSED_TESTS=$((PASSED_TESTS + 1))
		else
			printf "${RED}[FAIL]${RESET}\n"
			FAILED_TESTS=$((FAILED_TESTS + 1))
		fi
	fi
}

# ============================================================================
# TEST SUITES
# ============================================================================

test_basic_validation() {
	print_section "Basic Input Validation"

	run_test "Empty input" "" false "Empty input or missing ant count"
	run_test "Only newlines" "\n\n\n" false "Empty input or missing ant count"
	run_test "Zero ants" "0" false "Ant count must be > 0"
	run_test "Negative ants" "-5" false "Ant count must be > 0"
	run_test "Non-numeric ants" "abc" false "Invalid ant count format"
	run_test "Float ants" "3.14" false "Invalid ant count format"
	run_test "Very large ants" "999999999999" false "Ant count too large"
	run_test "Ants with spaces" "1 0" false "Invalid ant count format"
}

test_room_validation() {
	print_section "Room Validation"

	run_test "No start room" "1\nroom 0 0" false "No ##start room defined"
	run_test "No end room" "1\n##start\nstart 0 0" false "No ##end room defined"
	run_test "Multiple start rooms" "1\n##start\nstart1 0 0\n##start\nstart2 1 1\n##end\nend 2 2" false "Multiple ##start rooms defined"
	run_test "Multiple end rooms" "1\n##start\nstart 0 0\n##end\nend1 1 1\n##end\nend2 2 2" false "Multiple ##end rooms defined"
	run_test "Duplicate room names" "1\n##start\nstart 0 0\nstart 1 1\n##end\nend 2 2" false "Duplicate room name"
	run_test "Room name starting with L" "1\n##start\nLroom 0 0\n##end\nend 1 1" false "Room name cannot start with 'L'"
	run_test "Room name with hash" "1\n##start\n#room 0 0\n##end\nend 1 1" false "No ##start room defined"
	run_test "Empty room name" "1\n##start\n 0 0\n##end\nend 1 1" false "Invalid room name"
	run_test "Room with no coordinates" "1\n##start\nstart\n##end\nend 1 1" false "Invalid line"
	run_test "Room with one coordinate" "1\n##start\nstart 0\n##end\nend 1 1" false "Invalid line"
	run_test "Room with non-numeric coords" "1\n##start\nstart abc def\n##end\nend 1 1" false "Invalid line format"
}

test_link_validation() {
	print_section "Link Validation"

	run_test "Link to non-existent room" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart-nowhere" false "Link references unknown room"
	run_test "Self-linking room" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart-start" false "Room cannot link to itself"
	run_test "Invalid link format" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart_end" false "Invalid line format"
	run_test "Link with no dash" "1\n##start\nstart 0 0\n##end\nend 1 1\nstartend" false "Invalid line format"
	run_test "Link with multiple dashes" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart-middle-end" false "Link references unknown room"
	run_test "Empty link parts" "1\n##start\nstart 0 0\n##end\nend 1 1\n-end" false "Invalid link format"
}

test_ambiguous_cases() {
	print_section "Ambiguous Format Cases"

	run_test "Room name with dash" "1\n##start\nstart 0 0\n##end\nend 1 1\nroom-name 2 2" false "Room name cannot contain dashes"
	run_test "Link with coordinates" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart-end 2 2" false "Room name cannot contain dashes"
	run_test "Multi-dash link" "1\n##start\nstart 0 0\n##end\nend 1 1\nstart-middle-end" false "Link references unknown room"
	run_test "Complex ambiguous case" "1\n##start\nleo 0 0\n##end\nlea 1 1\nleo-lea 2 2" false "Room name cannot contain dashes"
}

test_valid_cases() {
	print_section "Valid Cases"

	run_test "Simple valid case" "2\n##start\nstart 0 0\n##end\nend 1 1\nstart-end" true
	run_test "With comments" "1\n# This is a comment\n##start\nstart 0 0\n# Another comment\n##end\nend 1 1\nstart-end" true
	run_test "Multiple rooms" "3\n##start\nstart 0 0\nmiddle 1 1\n##end\nend 2 2\nstart-middle\nmiddle-end" true
	run_test "Negative coordinates" "1\n##start\nstart -10 -20\n##end\nend 30 40\nstart-end" true
	run_test "Large coordinates" "1\n##start\nstart -1000000 1000000\n##end\nend 0 0\nstart-end" true
	run_test "Many ants" "100\n##start\nstart 0 0\n##end\nend 1 1\nstart-end" true
}

test_edge_cases() {
	print_section "Edge Cases"

	run_test "Start equals end" "1\n##start\n##end\nroom 0 0\nroom-room" false
	run_test "No links" "1\n##start\nstart 0 0\n##end\nend 1 1" true
	run_test "Disconnected graph" "2\n##start\nstart 0 0\nmiddle 1 1\n##end\nend 2 2\nstart-middle" true
	run_test "Complex valid graph" "5\n##start\nstart 0 0\na 1 0\nb 2 0\nc 1 1\n##end\nend 2 1\nstart-a\nstart-b\na-c\nb-c\nc-end" true
}

test_file_suite() {
	print_section "File-based Tests"

	echo "Testing invalid maps:"
	for file in resources/invalid_maps/*; do
		if [ -f "$file" ]; then
			run_file_test "$file" false
		fi
	done

	printf "\nTesting particular case maps:\n"
	# Test specific cases - need to determine validity case by case
	run_file_test "resources/particular_case_maps/ambiguous" false       # Has room name with dashes
	run_file_test "resources/particular_case_maps/start-end" true        # Valid simple case
	run_file_test "resources/particular_case_maps/wrong_room_link" false # Invalid room reference

	printf "\nTesting visualizer maps:\n"
	for file in resources/visualizer_maps/*; do
		if [ -f "$file" ]; then
			run_file_test "$file" true # These are typically valid maps for visualization
		fi
	done

	printf "\nTesting slow maps:\n"
	for file in resources/slow_maps/*; do
		if [ -f "$file" ]; then
			run_file_test "$file" true # These are performance test maps, should be valid
		fi
	done

	printf "\nTesting valid maps:\n"
	for file in resources/valid_maps/*; do
		if [ -f "$file" ]; then
			run_file_test "$file" true
		fi
	done
}

print_summary() {
	printf "\n${BLUE}===========================================${RESET}\n"
	printf "${WHITE}TEST SUMMARY${RESET}\n"
	printf "${BLUE}===========================================${RESET}\n"
	printf "${CYAN}Total tests: %d${RESET}\n" "$TOTAL_TESTS"
	printf "${GREEN}Passed: %d${RESET}\n" "$PASSED_TESTS"
	printf "${RED}Failed: %d${RESET}\n" "$FAILED_TESTS"

	if [ $FAILED_TESTS -eq 0 ]; then
		printf "${GREEN}ALL TESTS PASSED!${RESET}\n"
		exit 0
	else
		local success_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
		printf "${YELLOW}Success rate: %d%%${RESET}\n" "$success_rate"
		printf "${RED}SOME TESTS FAILED${RESET}\n"
		exit 1
	fi
}

# ============================================================================
# MAIN EXECUTION
# ============================================================================

# Parse command line arguments
while [ $# -gt 0 ]; do
	case $1 in
	-v | --verbose)
		VERBOSE=true
		shift
		;;
	-t | --timeout)
		TIMEOUT="$2"
		shift 2
		;;
	-b | --binary)
		BINARY="$2"
		shift 2
		;;
	-h | --help)
		echo "Usage: $0 [OPTIONS]"
		echo "Options:"
		echo "  -v, --verbose     Enable verbose output"
		echo "  -t, --timeout N   Set timeout to N seconds (default: 10)"
		echo "  -b, --binary PATH Set binary path (default: ./lem-in)"
		echo "  -h, --help        Show this help"
		exit 0
		;;
	*)
		echo "Unknown option: $1"
		exit 1
		;;
	esac
done

# Check if binary exists
if [ ! -f "$BINARY" ]; then
	printf "${RED}Error: Binary not found: %s${RESET}\n" "$BINARY"
	echo "Run 'make' to build the project first."
	exit 1
fi

# Run test suites
print_header
test_basic_validation
test_room_validation
test_link_validation
test_ambiguous_cases
test_valid_cases
test_edge_cases
test_file_suite
print_summary
