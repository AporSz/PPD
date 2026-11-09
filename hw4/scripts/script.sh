#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: script.sh <variant> <processes> [csv-file]

Variants: 0, 1, 1.1, 2, 3
Runs the selected variant 10 times, checks every result against Variant 0,
and reports the average execution time.

Example:
  ./scripts/script.sh 2 8
  ./scripts/script.sh 1.1 5 results.csv
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW4_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
BUILD_DIR="$HW4_DIR/build-mpi"
PROGRAM="$BUILD_DIR/hw4"
RUNS=10

[[ $# -ge 2 && $# -le 3 ]] || usage

VARIANT=$1
PROCESS_COUNT=$2
CSV_FILE=${3:-"$SCRIPT_DIR/results.csv"}

case "$VARIANT" in
    0|1|1.1|2|3) ;;
    *) echo "Error: variant must be 0, 1, 1.1, 2, or 3." >&2; exit 2 ;;
esac

[[ "$PROCESS_COUNT" =~ ^[1-9][0-9]*$ ]] || {
    echo "Error: process count must be a positive integer." >&2
    exit 2
}
if [[ "$VARIANT" != 0 && "$PROCESS_COUNT" -lt 2 ]]; then
    echo "Error: variants 1, 1.1, 2, and 3 require at least 2 processes." >&2
    exit 2
fi

for input_file in "$HW4_DIR/number1.txt" "$HW4_DIR/number2.txt"; do
    if [[ ! -s "$input_file" ]]; then
        echo "Error: missing or empty input file: $input_file" >&2
        exit 1
    fi
done

MPIEXEC=${MPIEXEC:-mpirun}
MPIEXEC_ARGS=()
if "$MPIEXEC" --version 2>&1 | grep -qi 'Open MPI'; then
    MPIEXEC_ARGS+=(--oversubscribe)
    if [[ $(id -u) -eq 0 ]]; then
        MPIEXEC_ARGS+=(--allow-run-as-root)
    fi
fi

cmake -S "$HW4_DIR" -B "$BUILD_DIR" \
    -DCMAKE_CXX_COMPILER="${CXX:-mpic++}" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release --parallel

if [[ "$CSV_FILE" != /* ]]; then
    CSV_FILE="$PWD/$CSV_FILE"
fi
mkdir -p "$(dirname -- "$CSV_FILE")"
if [[ ! -s "$CSV_FILE" ]]; then
    echo "timestamp,variant,processes,run,time_seconds" > "$CSV_FILE"
fi

TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT
RESULT_FILE="$HW4_DIR/number3.txt"
REFERENCE_FILE="$TEMP_DIR/number3.reference"
LOG_FILE="$TEMP_DIR/run.log"

run_program() {
    local variant=$1
    if ! (cd "$BUILD_DIR" && "$MPIEXEC" "${MPIEXEC_ARGS[@]}" \
        -np "$PROCESS_COUNT" "$PROGRAM" "$variant") >"$LOG_FILE" 2>&1; then
        cat "$LOG_FILE" >&2
        echo "Error: variant $variant failed." >&2
        exit 1
    fi
}

read_time() {
    awk '
        $NF == "s" && $(NF - 1) ~ /^[0-9]+([.][0-9]+)?$/ {
            print $(NF - 1)
            count++
        }
        END {
            if (count != 1) {
                exit 1
            }
        }
    ' "$LOG_FILE"
}

echo "Generating sequential reference for correctness checks."
run_program 0
if [[ ! -s "$RESULT_FILE" ]]; then
    cat "$LOG_FILE" >&2
    echo "Error: Variant 0 did not create number3.txt." >&2
    exit 1
fi
cp -- "$RESULT_FILE" "$REFERENCE_FILE"

TIMES=()
for ((run = 1; run <= RUNS; run++)); do
    echo "Run $run/$RUNS: variant=$VARIANT processes=$PROCESS_COUNT"
    run_program "$VARIANT"

    time_value=$(read_time) || {
        cat "$LOG_FILE" >&2
        echo "Error: unable to parse exactly one execution time." >&2
        exit 1
    }

    if [[ ! -s "$RESULT_FILE" ]] || ! cmp -s "$REFERENCE_FILE" "$RESULT_FILE"; then
        echo "Error: result differs from the sequential reference on run $run." >&2
        exit 1
    fi

    TIMES+=("$time_value")
    printf '%s,%s,%s,%s,%s\n' \
        "$(date --iso-8601=seconds)" "$VARIANT" "$PROCESS_COUNT" "$run" "$time_value" \
        >> "$CSV_FILE"
    awk -v run="$run" -v total="$RUNS" -v time="$time_value" \
        'BEGIN { printf "  execution time: %s seconds (%d/%d)\n", time, run, total }'
done

AVERAGE=$(printf '%s\n' "${TIMES[@]}" |
    awk '{ total += $1 } END { printf "%.9f", total / NR }')
printf '%s,%s,%s,average,%s\n' \
    "$(date --iso-8601=seconds)" "$VARIANT" "$PROCESS_COUNT" "$AVERAGE" \
    >> "$CSV_FILE"

echo "Average execution time: $AVERAGE seconds"
echo "All $RUNS results match the sequential reference."
echo "Results appended to $CSV_FILE"
