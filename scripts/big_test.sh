#!/bin/bash

# ============================================================================
# Massive test script for lem-in
# Generates 10x each map style and tests all generated maps
# ============================================================================

# Colors for display
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RESET='\033[0m'
BOLD='\033[1m'
BOLDCYAN='\033[1;36m'

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Map styles to generate
MAP_STYLES=(
    "flow-one"
    "flow-ten"
    "flow-thousand"
    "big"
    "big-superposition"
)

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SLOW_TESTS=0
TIMEOUT_TESTS=0

# Log folder for failures
LOG_DIR="$PROJECT_DIR/resources/big_test_logs"
LOG_FILE=""

# Prepare log file on first use
prepare_log_file() {
    if [ -z "$LOG_FILE" ]; then
        mkdir -p "$LOG_DIR"
        local timestamp
        timestamp=$(date +%Y%m%d_%H%M%S)
        LOG_FILE="$LOG_DIR/big_test_${timestamp}.log"
        touch "$LOG_FILE"
    fi
}

# Add entry to log and save failed map
log_failure() {
    local message="$1"
    local map_file="$2"
    local map_name="$3"

    prepare_log_file
    echo "$message" >> "$LOG_FILE"

    if [ -f "$map_file" ]; then
        local dest_file="$LOG_DIR/${map_name}.txt"
        if [ -e "$dest_file" ]; then
            local counter=1
            while [ -e "${LOG_DIR}/${map_name}_${counter}.txt" ]; do
                counter=$((counter + 1))
            done
            dest_file="${LOG_DIR}/${map_name}_${counter}.txt"
        fi
        cp "$map_file" "$dest_file"
    fi
}

# Function to display error message
error() {
    echo -e "${RED}${BOLD}[ERROR]${RESET} $1" >&2
}

# Function to display success message
success() {
    echo -e "${GREEN}${BOLD}[SUCCESS]${RESET} $1"
}

# Function to display info message
info() {
    echo -e "${BLUE}${BOLD}[INFO]${RESET} $1"
}

# Function to display warning message
warning() {
    echo -e "${YELLOW}${BOLD}[WARNING]${RESET} $1"
}

# Detect generator according to OS
detect_generator() {
    local os_name
    os_name=$(uname -s 2>/dev/null)

    case "$os_name" in
        Darwin*)
            echo "$PROJECT_DIR/resources/map_generator/generator_osx"
            ;;
        Linux*)
            echo "$PROJECT_DIR/resources/map_generator/generator_linux"
            ;;
        *)
            error "Unsupported OS: $os_name"
            exit 1
            ;;
    esac
}

GENERATOR=$(detect_generator)

if [ ! -f "$GENERATOR" ]; then
    error "Generator not found for $(uname -s): $GENERATOR"
    exit 1
fi

LEMIN="$PROJECT_DIR/lem-in"
TEST_DIR="$PROJECT_DIR/resources/big_test_maps"

# Check that necessary files exist
if [ ! -f "$GENERATOR" ]; then
    error "Generator not found: $GENERATOR"
    exit 1
fi

if [ ! -f "$LEMIN" ]; then
    error "lem-in not found: $LEMIN"
    error "Compile first with: make"
    exit 1
fi

# Create test folder
mkdir -p "$TEST_DIR"
info "Test folder: $TEST_DIR"

# Function to generate a map
generate_map() {
    local style=$1
    local index=$2
    local output_file="$TEST_DIR/${style}_${index}"
    
    if [ "$style" = "flow-one" ]; then
        "$GENERATOR" --flow-one > "$output_file" 2>/dev/null
    elif [ "$style" = "flow-ten" ]; then
        "$GENERATOR" --flow-ten > "$output_file" 2>/dev/null
    elif [ "$style" = "flow-thousand" ]; then
        "$GENERATOR" --flow-thousand > "$output_file" 2>/dev/null
    elif [ "$style" = "big" ]; then
        "$GENERATOR" --big > "$output_file" 2>/dev/null
    elif [ "$style" = "big-superposition" ]; then
        "$GENERATOR" --big-superposition > "$output_file" 2>/dev/null
    else
        error "Unknown style: $style"
        return 1
    fi
    
    if [ $? -ne 0 ] || [ ! -f "$output_file" ] || [ ! -s "$output_file" ]; then
        error "Generation failed: ${style}_${index}"
        return 1
    fi
    
    return 0
}

# Function to test a map
test_map() {
    local map_file=$1
    local map_name=$(basename "$map_file")
    
    # Extract required number
    local required=$(grep -m1 -E '^#Here is the number of lines required: *[0-9]+' "$map_file" 2>/dev/null | sed -E 's/.*: *([0-9]+).*/\1/')
    
    if [ -z "$required" ]; then
        local plain_msg="FAIL    ${map_name:0:50} -> missing required-lines header"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}missing required-lines header${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Measure execution time with time command
    # TIMEFORMAT='%R' gives real time in seconds (decimal format)
    # time writes to stderr, so we capture stderr separately
    local TIMEFORMAT='%R'
    local temp_output=$(mktemp)
    local time_stderr=$( { time "$LEMIN" < "$map_file" 2>/dev/null > "$temp_output"; } 2>&1 )
    local output=$(cat "$temp_output" 2>/dev/null)
    rm -f "$temp_output"
    
    # Extract real time (last line of stderr containing time output)
    local elapsed=$(echo "$time_stderr" | tail -1)
    
    # Check that elapsed is a valid number
    if ! echo "$elapsed" | grep -qE '^[0-9]+\.?[0-9]*$'; then
        local plain_msg="FAIL    ${map_name:0:50} -> unable to measure execution time"
        error "Unable to measure time for ${map_name}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Convert decimal time to integer (round up to be strict)
    # Use awk to round up
    local elapsed_int=$(echo "$elapsed" | awk '{printf "%.0f", ($1 + 0.5)}')
    
    # Check timeout (15 seconds) - invalidate map if exceeded
    # According to subject: "15 seconds is too much" → map is invalid
    if [ "$elapsed_int" -ge 15 ]; then
        TIMEOUT_TESTS=$((TIMEOUT_TESTS + 1))
        local plain_msg="FAIL    ${map_name:0:50} -> TIMEOUT (${elapsed}s, max=15s) - MAP INVALIDE"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}TIMEOUT (${elapsed}s, max=15s) - MAP INVALIDE${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    local got=$(echo "$output" | awk -F': ' '/^# Number of lines: /{v=$2} END{if (v) print v}')
    
    if [ -z "$got" ]; then
        local plain_msg="FAIL    ${map_name:0:50} -> no result found (got N/A / wanted $required, time=${elapsed}s)"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}no result found (got N/A / wanted $required, time=${elapsed}s)${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Determine time status (use elapsed_int for comparisons)
    local time_status=""
    local time_color=""
    if [ "$elapsed_int" -le 3 ]; then
        time_status="excellent"
        time_color="${GREEN}"
    elif [ "$elapsed_int" -le 9 ]; then
        time_status="mediocre"
        time_color="${YELLOW}"
        SLOW_TESTS=$((SLOW_TESTS + 1))
    else
        time_status="slow"
        time_color="${RED}"
        SLOW_TESTS=$((SLOW_TESTS + 1))
    fi
    
    # Determine status according to number of turns
    local status_label=""
    local status_color=""
    if [ "$got" -eq "$required" ]; then
        status_label="PERFECT"
        status_color="${CYAN}"
    elif [ "$got" -lt "$required" ]; then
        status_label="BETTER"
        status_color="${BOLDCYAN}"
    elif [ "$got" -gt $((required + 10)) ]; then
        status_label="WARNING"
        status_color="${YELLOW}"
    else
        status_label="SUCCESS"
        status_color="${GREEN}"
    fi

    # Display result
    local result_text="got/wanted: ${got}/${required}"
    if [ "$elapsed_int" -le 9 ]; then
        echo -e "${status_color}${status_label}${RESET} ${map_name:0:50} -> ${result_text}, time=${time_color}${elapsed}s${RESET} (${time_status})"
    else
        echo -e "${status_color}${status_label}${RESET} ${map_name:0:50} -> ${result_text}, time=${time_color}${elapsed}s${RESET} (${time_status}) ${YELLOW}⚠${RESET}"
    fi

    # WARNING remains a logical success (not optimal) -> no blocking failure
    if [ "$status_label" = "WARNING" ]; then
        warning "${map_name:0:50} -> more than 10 lines above required (got=$got / wanted=$required)"
    fi

    return 0
}

# Generate all maps
info "Generating 10 maps for each style..."
for style in "${MAP_STYLES[@]}"; do
    info "Generating maps: $style"
    for i in {1..10}; do
        if generate_map "$style" "$i"; then
            echo -e "  ${CYAN}✓${RESET} ${style}_${i} generated"
            sleep 1
        else
            echo -e "  ${RED}✗${RESET} ${style}_${i} failed"
            sleep 1
        fi
    done
done

echo ""
info "Starting tests..."

# Test all generated maps
for map_file in "$TEST_DIR"/*; do
    if [ -f "$map_file" ]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        if test_map "$map_file"; then
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
done

# Final summary
echo ""
echo "=========================================="
if [ $FAILED_TESTS -eq 0 ] && [ $TIMEOUT_TESTS -eq 0 ]; then
    success "All tests passed!"
    echo -e "${GREEN}${BOLD}Result:${RESET} $PASSED_TESTS/$TOTAL_TESTS tests succeeded"
    if [ $SLOW_TESTS -gt 0 ]; then
        warning "$SLOW_TESTS test(s) with mediocre (>3s) or slow (>9s) performance"
    fi
    if [ -n "$LOG_FILE" ]; then
        warning "See logs in $LOG_FILE (maps copied to $LOG_DIR)"
    fi
else
    error "Some tests failed"
    echo -e "${RED}${BOLD}Failures:${RESET} $FAILED_TESTS/$TOTAL_TESTS"
    echo -e "${GREEN}${BOLD}Succeeded:${RESET} $PASSED_TESTS/$TOTAL_TESTS"
    if [ $TIMEOUT_TESTS -gt 0 ]; then
        echo -e "${RED}${BOLD}Timeouts:${RESET} $TIMEOUT_TESTS test(s) > 15s"
    fi
    if [ $SLOW_TESTS -gt 0 ]; then
        warning "$SLOW_TESTS test(s) with mediocre (>3s) or slow (>9s) performance"
    fi
    if [ -n "$LOG_FILE" ]; then
        warning "See logs in $LOG_FILE (maps copied to $LOG_DIR)"
    fi
fi
echo "=========================================="
echo ""
echo -e "${CYAN}${BOLD}Performance criteria:${RESET}"
echo -e "  ${GREEN}≤ 3s${RESET}  : Excellent (2-3s is ideal)"
echo -e "  ${YELLOW}4-9s${RESET}  : Mediocre (9s is acceptable but not optimal)"
echo -e "  ${RED}10-14s${RESET} : Slow (acceptable but should be optimized)"
echo -e "  ${RED}≥ 15s${RESET}  : ${RED}${BOLD}Too slow - INVALID MAP${RESET} (according to subject: '15 seconds is too much')"
echo ""
echo -e "${CYAN}${BOLD}Result status legend:${RESET}"
echo -e "  ${BOLDCYAN}BETTER${RESET}  : Solution better than required (got < wanted)"
echo -e "  ${CYAN}PERFECT${RESET} : Exact optimal solution (got = wanted)"
echo -e "  ${GREEN}SUCCESS${RESET} : Good solution (got ≤ wanted + 10 lines)"
echo -e "  ${YELLOW}WARNING${RESET} : Suboptimal solution (got > wanted + 10 lines)"
echo -e "  ${RED}FAIL${RESET}    : Test failed (error, timeout, or no result)"

# Clean up generated maps
echo ""
info "Cleaning up generated maps..."
if [ -d "$TEST_DIR" ]; then
    rm -rf "$TEST_DIR"
    if [ $? -eq 0 ]; then
        echo -e "  ${CYAN}✓${RESET} Folder $TEST_DIR deleted"
    else
        warning "Unable to delete $TEST_DIR"
    fi
else
    echo -e "  ${CYAN}✓${RESET} No folder to clean up"
fi

# Return appropriate error code
if [ $FAILED_TESTS -eq 0 ] && [ $TIMEOUT_TESTS -eq 0 ]; then
    exit 0
else
    exit 1
fi

