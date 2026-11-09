#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: run_all.sh [--start-id ID] [--dry-run]

Runs all required HW4 test configurations. Each configuration is measured
10 times by script.sh, which also checks its result against Variant 0.
Results are written to hw4/all_results.csv.

Use --start-id to resume at a 1-based configuration ID.
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW4_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
GENERATOR="$HW4_DIR/generate_numbers.py"
RUNNER="$SCRIPT_DIR/script.sh"

START_ID=1
DRY_RUN=false
while (($# > 0)); do
    case "$1" in
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --start-id)
            (($# >= 2)) || usage
            [[ "$2" =~ ^[1-9][0-9]*$ ]] || usage
            START_ID=$2
            shift 2
            ;;
        *) usage ;;
    esac
done

CONFIGURATIONS=()
add_configuration() {
    local variant=$1
    local test_case=$2
    local processes=$3
    CONFIGURATIONS+=("$variant $test_case $processes")
}

add_random_configurations() {
    local variant=$1
    local test_case=$2
    shift 2
    local processes
    for processes in "$@"; do
        add_configuration "$variant" "$test_case" "$processes"
    done
}

# Variant 0 is sequential, so it is run with one process for every input shape.
for test_case in fixed16 random10000 random1000 randomSkewed; do
    add_configuration 0 "$test_case" 1
done

# Variant 1.1 uses the same input sizes and process counts as Variant 1.
for variant in 1 1.1 3; do
    add_configuration "$variant" fixed16 5
    add_random_configurations "$variant" random10000 5 9 17
    add_random_configurations "$variant" randomSkewed 5 9 17
done

# Variant 2 uses the separate sizes and process counts specified in its tests.
add_configuration 2 fixed16 4
add_random_configurations 2 random1000 4 8 16
add_random_configurations 2 randomSkewed 4 8 16

CONFIGURATION_COUNT=${#CONFIGURATIONS[@]}
if (( START_ID > CONFIGURATION_COUNT )); then
    echo "Error: start ID must be between 1 and $CONFIGURATION_COUNT." >&2
    exit 2
fi

if [[ "$DRY_RUN" == true ]]; then
    echo "Dry run: configurations $START_ID-$CONFIGURATION_COUNT, 10 executions each."
    for ((index = START_ID - 1; index < CONFIGURATION_COUNT; index++)); do
        read -r variant test_case processes <<< "${CONFIGURATIONS[index]}"
        echo "[$((index + 1))/$CONFIGURATION_COUNT] variant=$variant case=$test_case processes=$processes"
    done
    exit 0
fi

RESULTS_FILE="$HW4_DIR/all_results.csv"
CSV_HEADER="timestamp,variant,processes,test_case,n1,n2,average_time_seconds"
if (( START_ID == 1 )); then
    echo "$CSV_HEADER" > "$RESULTS_FILE"
elif [[ -s "$RESULTS_FILE" ]]; then
    IFS= read -r existing_header < "$RESULTS_FILE"
    if [[ "$existing_header" != "$CSV_HEADER" ]]; then
        echo "Error: cannot resume because $RESULTS_FILE has an unexpected CSV header." >&2
        exit 1
    fi
else
    echo "Error: cannot resume because $RESULTS_FILE does not exist or is empty. Run from configuration 1 first." >&2
    exit 1
fi

TEMP_DIR=$(mktemp -d)
BACKUP_DIR="$TEMP_DIR/original"
CASE_DIR="$TEMP_DIR/cases"
mkdir -p "$BACKUP_DIR" "$CASE_DIR"

for file in number1.txt number2.txt number3.txt; do
    if [[ -e "$HW4_DIR/$file" ]]; then
        cp -p -- "$HW4_DIR/$file" "$BACKUP_DIR/$file"
    fi
done

cleanup() {
    for file in number1.txt number2.txt number3.txt; do
        if [[ -e "$BACKUP_DIR/$file" ]]; then
            cp -p -- "$BACKUP_DIR/$file" "$HW4_DIR/$file"
        else
            rm -f -- "$HW4_DIR/$file"
        fi
    done
    rm -rf -- "$TEMP_DIR"
}
trap cleanup EXIT

write_fixed_number() {
    local path=$1
    local digits=${2// /}
    printf '%s\n' "${#digits}" > "$path"
    for ((index = 0; index < ${#digits}; index++)); do
        printf '%s ' "${digits:index:1}" >> "$path"
    done
    printf '\n' >> "$path"
}

mkdir -p "$CASE_DIR/fixed16"
write_fixed_number "$CASE_DIR/fixed16/number1.txt" "9999 4444 4444 9999"
write_fixed_number "$CASE_DIR/fixed16/number2.txt" "9999 5555 5555 9999"

for test_case in random10000 random1000 randomSkewed; do
    case "$test_case" in
        random10000) n1=10000; n2=10000 ;;
        random1000) n1=1000; n2=1000 ;;
        randomSkewed) n1=100; n2=100000 ;;
    esac
    mkdir -p "$CASE_DIR/$test_case"
    python3 "$GENERATOR" "$n1" "$n2" >/dev/null
    cp -- "$HW4_DIR/number1.txt" "$CASE_DIR/$test_case/number1.txt"
    cp -- "$HW4_DIR/number2.txt" "$CASE_DIR/$test_case/number2.txt"
done

configuration=0
for entry in "${CONFIGURATIONS[@]}"; do
    ((configuration += 1))
    if (( configuration < START_ID )); then
        continue
    fi

    read -r variant test_case processes <<< "$entry"
    case "$test_case" in
        fixed16) n1=16; n2=16 ;;
        random10000) n1=10000; n2=10000 ;;
        random1000) n1=1000; n2=1000 ;;
        randomSkewed) n1=100; n2=100000 ;;
    esac

    cp -- "$CASE_DIR/$test_case/number1.txt" "$HW4_DIR/number1.txt"
    cp -- "$CASE_DIR/$test_case/number2.txt" "$HW4_DIR/number2.txt"

    case_csv="$TEMP_DIR/result-$configuration.csv"
    log_file="$TEMP_DIR/run-$configuration.log"
    echo "[$configuration/$CONFIGURATION_COUNT] variant=$variant case=$test_case (${n1}+${n2} digits) processes=$processes"
    if ! "$RUNNER" "$variant" "$processes" "$case_csv" >"$log_file" 2>&1; then
        cat "$log_file" >&2
        echo "Error: benchmark failed for variant=$variant, case=$test_case, processes=$processes." >&2
        exit 1
    fi

    average=$(awk -F, '$4 == "average" { print $5; count++ } END { if (count != 1) exit 1 }' "$case_csv") || {
        cat "$log_file" >&2
        echo "Error: expected exactly one average row in $case_csv." >&2
        exit 1
    }
    printf '%s,%s,%s,%s,%s,%s,%s\n' \
        "$(date --iso-8601=seconds)" "$variant" "$processes" "$test_case" "$n1" "$n2" "$average" \
        >> "$RESULTS_FILE"
    tail -n 3 "$log_file"
done

echo "Completed configurations $START_ID-$CONFIGURATION_COUNT ($((CONFIGURATION_COUNT - START_ID + 1)) total, 10 executions each)."
echo "Average results written to $RESULTS_FILE"
