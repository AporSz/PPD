#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: script.sh <cpp|java> <s|h|v|b|l|c> <threads> [runs=10] [dynamic|static] [csv-file]

Examples:
  ./scripts/script.sh cpp h 4
  ./scripts/script.sh cpp c 8 10 static results.csv
  ./scripts/script.sh java b 4

Run generateMatrix.py first to create hw2/files/MatrixF.txt and
hw2/files/ConvolutionM.txt. The same files are used for every run.
EOF
    exit 2
}

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
HW2_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
FILES_DIR="$HW2_DIR/files"
CPP_DIR="$HW2_DIR/hw2-ppd-cpp"
JAVA_DIR="$HW2_DIR/hw2-ppd-java"

[[ $# -ge 3 && $# -le 6 ]] || usage

LANGUAGE=${1,,}
TYPE=${2,,}
THREADS=$3
RUNS=${4:-10}
ALLOCATION=${5:-dynamic}
CSV_FILE=${6:-"$SCRIPT_DIR/results.csv"}

case "$LANGUAGE" in
    cpp|java) ;;
    *) echo "Error: language must be 'cpp' or 'java'." >&2; exit 2 ;;
esac

case "$TYPE" in
    s|h|v|b|l|c) ;;
    *) echo "Error: type must be one of s, h, v, b, l, c." >&2; exit 2 ;;
esac

[[ "$THREADS" =~ ^[1-9][0-9]*$ ]] || { echo "Error: threads must be a positive integer." >&2; exit 2; }
[[ "$RUNS" =~ ^[1-9][0-9]*$ ]] || { echo "Error: runs must be a positive integer." >&2; exit 2; }

if [[ "$LANGUAGE" == cpp ]]; then
    case "${ALLOCATION,,}" in
        dynamic|d) ALLOCATION=dynamic ;;
        static|s) ALLOCATION=static ;;
        *) echo "Error: allocation must be 'dynamic' or 'static'." >&2; exit 2 ;;
    esac
else
    ALLOCATION=not-applicable
fi

if [[ ! -f "$FILES_DIR/MatrixF.txt" || ! -f "$FILES_DIR/ConvolutionM.txt" ]]; then
    echo "Error: input matrices are missing. Run scripts/generateMatrix.py first." >&2
    exit 1
fi

read -r N < "$FILES_DIR/MatrixF.txt"
read -r M < <(sed -n '2p' "$FILES_DIR/MatrixF.txt")
read -r K_ROWS < "$FILES_DIR/ConvolutionM.txt"
read -r K_COLS < <(sed -n '2p' "$FILES_DIR/ConvolutionM.txt")
for dimension in "$N" "$M" "$K_ROWS" "$K_COLS"; do
    [[ "$dimension" =~ ^[1-9][0-9]*$ ]] || {
        echo "Error: invalid matrix dimensions in input files." >&2
        exit 1
    }
done
[[ "$K_ROWS" == "$K_COLS" && $((K_ROWS % 2)) -eq 1 ]] || {
    echo "Error: the convolution matrix must be square with an odd dimension." >&2
    exit 1
}

case "$LANGUAGE" in
    cpp)
        cmake -S "$CPP_DIR" -B "$CPP_DIR/build" -DCMAKE_BUILD_TYPE=Release
        cmake --build "$CPP_DIR/build" --parallel
        PROGRAM=("$CPP_DIR/build/hw2_ppd_cpp" "$THREADS" "$TYPE" "$ALLOCATION")
        ;;
    java)
        mkdir -p "$JAVA_DIR/out"
        mapfile -t JAVA_SOURCES < <(find "$JAVA_DIR/src" -name '*.java' -type f | sort)
        javac -d "$JAVA_DIR/out" "${JAVA_SOURCES[@]}"
        PROGRAM=(java -cp "$JAVA_DIR/out" Main "$THREADS" "$TYPE")
        ;;
esac

if [[ "$CSV_FILE" != /* ]]; then
    CSV_FILE="$PWD/$CSV_FILE"
fi
mkdir -p "$(dirname -- "$CSV_FILE")"
if [[ ! -s "$CSV_FILE" ]]; then
    echo "timestamp,language,type,allocation,n,m,k,threads,run,time_ms" > "$CSV_FILE"
fi

# A fresh baseline is necessary whenever a test starts, so it always matches
# the matrix files used by this run.
rm -f "$FILES_DIR/sequential.txt" "$FILES_DIR/output.txt"

case "$LANGUAGE" in
    cpp) WORK_DIR=$CPP_DIR ;;
    java) WORK_DIR=$JAVA_DIR ;;
esac

TIMES=()
for ((run = 1; run <= RUNS; run++)); do
    echo "Run $run/$RUNS: $LANGUAGE type=$TYPE threads=$THREADS allocation=$ALLOCATION"
    result=$(cd "$WORK_DIR" && "${PROGRAM[@]}")
    if [[ ! "$result" =~ ^([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?$ ]]; then
        echo "Error: program did not return a single execution-time value: $result" >&2
        exit 1
    fi
    if [[ ! -s "$FILES_DIR/sequential.txt" || ! -s "$FILES_DIR/output.txt" ]] ||
       ! cmp -s "$FILES_DIR/sequential.txt" "$FILES_DIR/output.txt"; then
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
