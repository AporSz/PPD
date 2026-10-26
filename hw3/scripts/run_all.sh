#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: run_all.sh [--start-id ID] [--dry-run]

Runs sequential and line-based convolution for the HW3 matrix sizes. Each
configuration is repeated exactly 10 times. Results are written to
hw3/all_results.csv. Use --start-id to resume at a 1-based configuration ID.
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW3_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
GENERATOR="$SCRIPT_DIR/generateMatrix.py"
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

MATRIX_CASES=(
    "10 10 2"
    "1000 1000 2 4 8 16"
    "10000 10000 2 4 8 16"
)
LANGUAGES=(cpp java)
EXECUTOR_TYPES=(s h v)

configuration_count=0
for matrix_case in "${MATRIX_CASES[@]}"; do
    read -r n m thread_values <<< "$matrix_case"
    read -r -a line_threads <<< "$thread_values"
    for language in "${LANGUAGES[@]}"; do
        for type in "${EXECUTOR_TYPES[@]}"; do
            if [[ "$type" == s ]]; then
                ((configuration_count += 1))
            else
                configuration_count=$((configuration_count + ${#line_threads[@]}))
            fi
        done
    done
done

if (( START_ID > configuration_count )); then
    echo "Error: start ID must be between 1 and $configuration_count." >&2
    exit 2
fi

remaining_count=$((configuration_count - START_ID + 1))
if [[ "$DRY_RUN" == true ]]; then
    echo "Dry run: configurations $START_ID-$configuration_count ($remaining_count total), 10 executions each."
    exit 0
fi

RESULTS_FILE="$HW3_DIR/all_results.csv"
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
    read -r n m thread_values <<< "$matrix_case"
    read -r -a line_threads <<< "$thread_values"
    inputs_generated=false

    for language in "${LANGUAGES[@]}"; do
        for type in "${EXECUTOR_TYPES[@]}"; do
            if [[ "$type" == s ]]; then
                threads=(1)
            else
                threads=("${line_threads[@]}")
            fi

            for thread_count in "${threads[@]}"; do
                ((configuration += 1))
                if (( configuration < START_ID )); then
                    continue
                fi

                if [[ "$inputs_generated" == false ]]; then
                    reuse_inputs=false
                    if (( START_ID > 1 )) &&
                       [[ -f "$HW3_DIR/files/date.txt" && -f "$HW3_DIR/files/ConvolutionM.txt" ]]; then
                        read -r input_n < "$HW3_DIR/files/date.txt"
                        read -r input_m < <(sed -n '2p' "$HW3_DIR/files/date.txt")
                        read -r input_k_rows < "$HW3_DIR/files/ConvolutionM.txt"
                        read -r input_k_cols < <(sed -n '2p' "$HW3_DIR/files/ConvolutionM.txt")
                        if [[ "$input_n" == "$n" && "$input_m" == "$m" &&
                              "$input_k_rows" == 3 && "$input_k_cols" == 3 ]]; then
                            reuse_inputs=true
                        fi
                    fi

                    if [[ "$reuse_inputs" == true ]]; then
                        echo "Reusing inputs for ${n}x${m}, kernel 3x3"
                    else
                        echo "Generating inputs for ${n}x${m}, kernel 3x3"
                        "$GENERATOR" "$n" "$m" 3
                    fi
                    inputs_generated=true
                fi

                csv_file="$TEMP_DIR/result-${configuration}.csv"
                log_file="$TEMP_DIR/run.log"

                echo "[$configuration/$configuration_count] $language type=$type threads=$thread_count matrix=${n}x${m} k=3"
                if ! "$RUNNER" "$language" "$type" "$thread_count" "$csv_file" >"$log_file" 2>&1; then
                    cat "$log_file" >&2
                    echo "Error: benchmark failed for $language type=$type p=$thread_count, matrix=${n}x${m} k=3." >&2
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

echo "Completed configurations $START_ID-$configuration_count ($remaining_count total, 10 runs each)."
echo "Average results written to $RESULTS_FILE"
