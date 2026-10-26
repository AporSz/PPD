#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: run_all.sh [runs=10] [--start-id ID] [--dry-run]

Runs every required hw2 language, allocation, executor, thread-count, and
matrix-size combination. Results are written to hw2/all_results.csv.
Use --start-id to resume at a 1-based configuration ID, preserving existing
results. Use --dry-run to list the number of configurations without running them.
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW2_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
GENERATOR="$SCRIPT_DIR/generateMatrix.py"
RUNNER="$SCRIPT_DIR/script.sh"

RUNS=10
START_ID=1
DRY_RUN=false
runs_set=false
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
        *)
            if [[ "$runs_set" == false && "$1" =~ ^[1-9][0-9]*$ ]]; then
                RUNS=$1
                runs_set=true
                shift
            else
                usage
            fi
            ;;
    esac
done

MATRIX_CASES=(
    "10 10 3 4"
    "1000 1000 3 2 4 8 16"
    "10 10000 5 2 4 8 16"
    "10000 10 5 2 4 8 16"
    "10000 10000 5 2 4 8 16"
)
EXECUTOR_TYPES=(s h v l c b)
LANGUAGES=(cpp java)

configuration_count=0
for matrix_case in "${MATRIX_CASES[@]}"; do
    read -r n m k thread_values <<< "$matrix_case"
    read -r -a threads <<< "$thread_values"
    for language in "${LANGUAGES[@]}"; do
        if [[ "$language" == cpp ]]; then
            allocations=(dynamic static)
        else
            allocations=(non)
        fi
        for allocation in "${allocations[@]}"; do
            for thread_count in "${threads[@]}"; do
                for type in "${EXECUTOR_TYPES[@]}"; do
                    ((configuration_count += 1))
                done
            done
        done
    done
done

if (( START_ID > configuration_count )); then
    echo "Error: start ID must be between 1 and $configuration_count." >&2
    exit 2
fi

remaining_count=$((configuration_count - START_ID + 1))
if [[ "$DRY_RUN" == true ]]; then
    echo "Dry run: configurations $START_ID-$configuration_count ($remaining_count total), $RUNS executions each."
    exit 0
fi

RESULTS_FILE="$HW2_DIR/all_results.csv"
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT
CSV_HEADER="timestamp,language,type,allocation,n,m,k,threads,run,time_ms"
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

configuration=0
for matrix_case in "${MATRIX_CASES[@]}"; do
    read -r n m k thread_values <<< "$matrix_case"
    read -r -a threads <<< "$thread_values"

    inputs_generated=false

    for language in "${LANGUAGES[@]}"; do
        if [[ "$language" == cpp ]]; then
            allocations=(dynamic static)
        else
            allocations=(non)
        fi

        for allocation in "${allocations[@]}"; do
            for thread_count in "${threads[@]}"; do
                for type in "${EXECUTOR_TYPES[@]}"; do
                    ((configuration += 1))
                    if (( configuration < START_ID )); then
                        continue
                    fi

                    if [[ "$inputs_generated" == false ]]; then
                        echo "Generating inputs for ${n}x${m}, kernel ${k}x${k}"
                        "$GENERATOR" "$n" "$m" "$k"
                        inputs_generated=true
                    fi

                    csv_file="$TEMP_DIR/result-${configuration}.csv"
                    log_file="$TEMP_DIR/run.log"

                    echo "[$configuration/$configuration_count] $language $allocation type=$type threads=$thread_count matrix=${n}x${m} k=$k"
                    if ! "$RUNNER" "$language" "$type" "$thread_count" "$RUNS" "$allocation" "$csv_file" >"$log_file" 2>&1; then
                        cat "$log_file" >&2
                        echo "Error: benchmark failed for $language $allocation $type p=$thread_count, matrix=${n}x${m} k=$k." >&2
                        exit 1
                    fi

                    average_row=$(awk -F, '$9 == "average" { print; count++ } END { if (count != 1) exit 1 }' "$csv_file") || {
                        cat "$log_file" >&2
                        echo "Error: expected exactly one average row in $csv_file." >&2
                        exit 1
                    }
                    echo "$average_row" >> "$RESULTS_FILE"
                    tail -n 2 "$log_file"
                done
            done
        done
    done
done

echo "Completed configurations $START_ID-$configuration_count ($remaining_count total, $RUNS runs each)."
echo "Average results written to $RESULTS_FILE"
