#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: script.sh <cpp|java> <s|h|v> <threads> [csv-file]

Examples:
  ./scripts/script.sh cpp s 1
  ./scripts/script.sh cpp h 4 results.csv
  ./scripts/script.sh java v 8

Run generateMatrix.py first to create files/date.txt and
files/ConvolutionM.txt with a 3x3 convolution matrix. The same inputs are
used for every repetition. C++ always uses dynamic matrix allocation.
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW3_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
FILES_DIR="$HW3_DIR/files"
CPP_DIR="$HW3_DIR/hw3-ppd-cpp"
JAVA_DIR="$HW3_DIR/hw3-ppd-java"

[[ $# -ge 3 && $# -le 4 ]] || usage

LANGUAGE=${1,,}
TYPE=${2,,}
THREADS=$3
RUNS=10
CSV_FILE=${4:-"$SCRIPT_DIR/results.csv"}

case "$LANGUAGE" in
    cpp|java) ;;
    *) echo "Error: language must be 'cpp' or 'java'." >&2; exit 2 ;;
esac

case "$TYPE" in
    s|h|v) ;;
    *) echo "Error: type must be 's' (sequential), 'h' (horizontal), or 'v' (vertical)." >&2; exit 2 ;;
esac

[[ "$THREADS" =~ ^[1-9][0-9]*$ ]] || { echo "Error: threads must be a positive integer." >&2; exit 2; }
[[ "$RUNS" =~ ^[1-9][0-9]*$ ]] || { echo "Error: runs must be a positive integer." >&2; exit 2; }

if [[ "$LANGUAGE" == cpp ]]; then
    ALLOCATION=dynamic
else
    ALLOCATION=not-applicable
fi

if [[ ! -f "$FILES_DIR/date.txt" || ! -f "$FILES_DIR/ConvolutionM.txt" ]]; then
    echo "Error: input matrices are missing. Run scripts/generateMatrix.py first." >&2
    exit 1
fi

read -r N < "$FILES_DIR/date.txt"
read -r M < <(sed -n '2p' "$FILES_DIR/date.txt")
read -r K_ROWS < "$FILES_DIR/ConvolutionM.txt"
read -r K_COLS < <(sed -n '2p' "$FILES_DIR/ConvolutionM.txt")
for dimension in "$N" "$M" "$K_ROWS" "$K_COLS"; do
    [[ "$dimension" =~ ^[1-9][0-9]*$ ]] || {
        echo "Error: invalid matrix dimensions in input files." >&2
        exit 1
    }
done
[[ "$K_ROWS" == 3 && "$K_COLS" == 3 ]] || {
    echo "Error: HW3 requires a 3x3 convolution matrix." >&2
    exit 1
}

case "$LANGUAGE" in
    cpp)
        cmake -S "$CPP_DIR" -B "$CPP_DIR/build" -DCMAKE_BUILD_TYPE=Release
        cmake --build "$CPP_DIR/build" --parallel
        PROGRAM_PATH="$CPP_DIR/build/hw3_ppd_cpp"
        ;;
    java)
        mkdir -p "$JAVA_DIR/out"
        mapfile -t JAVA_SOURCES < <(find "$JAVA_DIR/src" -name '*.java' -type f | sort)
        javac -d "$JAVA_DIR/out" "${JAVA_SOURCES[@]}"
        PROGRAM_PATH=(java -cp "$JAVA_DIR/out" Main)
        ;;
esac

if [[ "$CSV_FILE" != /* ]]; then
    CSV_FILE="$PWD/$CSV_FILE"
fi
mkdir -p "$(dirname -- "$CSV_FILE")"
if [[ ! -s "$CSV_FILE" ]]; then
    echo "timestamp,language,type,allocation,n,m,k,threads,run,time_ms" > "$CSV_FILE"
fi

if [[ "$TYPE" == s ]]; then
    rm -f "$FILES_DIR/sequential.txt" "$FILES_DIR/output.txt"
else
    rm -f "$FILES_DIR/output.txt"
fi

case "$LANGUAGE" in
    cpp) WORK_DIR=$CPP_DIR ;;
    java) WORK_DIR=$JAVA_DIR ;;
esac

if [[ "$LANGUAGE" == cpp ]]; then
    PROGRAM=("$PROGRAM_PATH" "$THREADS" "$TYPE")
    BASELINE_PROGRAM=("$PROGRAM_PATH" 1 s)
else
    PROGRAM=("${PROGRAM_PATH[@]}" "$THREADS" "$TYPE")
    BASELINE_PROGRAM=("${PROGRAM_PATH[@]}" 1 s)
fi

if [[ "$TYPE" != s && ! -s "$FILES_DIR/sequential.txt" ]]; then
    echo "Generating sequential reference before timing line-based runs."
    baseline_result=$(cd "$WORK_DIR" && "${BASELINE_PROGRAM[@]}")
    if [[ ! "$baseline_result" =~ ^([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?$ ]] ||
       [[ ! -s "$FILES_DIR/sequential.txt" || ! -s "$FILES_DIR/output.txt" ]] ||
       ! cmp -s "$FILES_DIR/sequential.txt" "$FILES_DIR/output.txt"; then
        echo "Error: could not generate a valid sequential reference." >&2
        exit 1
    fi
fi

TIMES=()
for ((run = 1; run <= RUNS; run++)); do
    echo "Run $run/$RUNS: $LANGUAGE type=$TYPE threads=$THREADS allocation=$ALLOCATION"
    result=$(cd "$WORK_DIR" && "${PROGRAM[@]}")
    if [[ ! "$result" =~ ^([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?$ ]]; then
        echo "Error: program did not return a single execution-time value: $result" >&2
        exit 1
    fi
    if [[ "$TYPE" == s ]]; then
        if [[ ! -s "$FILES_DIR/sequential.txt" ]]; then
            echo "Error: sequential execution did not produce a reference on run $run." >&2
            exit 1
        fi
    else
        [[ -s "$FILES_DIR/sequential.txt" && -s "$FILES_DIR/output.txt" ]] || {
            echo "Error: missing output or sequential reference on run $run." >&2
            exit 1
        }
    fi
    if ! cmp -s "$FILES_DIR/sequential.txt" "$FILES_DIR/output.txt"; then
        echo "Error: output differs from the sequential reference on run $run." >&2
        exit 1
    fi

    TIMES+=("$result")
    printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
        "$(date --iso-8601=seconds)" "$LANGUAGE" "$TYPE" "$ALLOCATION" \
        "$N" "$M" "$K_ROWS" "$THREADS" "$run" "$result" >> "$CSV_FILE"
done

AVERAGE=$(printf '%s\n' "${TIMES[@]}" | awk '{ total += $1 } END { printf "%.6f", total / NR }')
printf '%s,%s,%s,%s,%s,%s,%s,%s,average,%s\n' \
    "$(date --iso-8601=seconds)" "$LANGUAGE" "$TYPE" "$ALLOCATION" \
    "$N" "$M" "$K_ROWS" "$THREADS" "$AVERAGE" >> "$CSV_FILE"
echo "Average execution time: $AVERAGE ms"
echo "Results appended to $CSV_FILE"
