#! /bin/bash

SCRIPT_PATH=$1
RETRIES=$2
THREADS=$3
FILE_OUTPUT=$4
LANGUAGE=$5
INDEX=1

N=1000000

RESULTS=()

if [ ! -f "input.txt" ] || [ "$(head -n1 input.txt 2>/dev/null)" != "$N" ]; then
    echo "Generating input.txt for N=$N..."
    python3 -c "
import random
n = $N
with open('input.txt', 'w') as f:
    f.write(f'{n}\n')
    f.write(' '.join(f'{random.uniform(0, 1000):.6f}' for _ in range(n)) + '\n')
    f.write(' '.join(f'{random.uniform(0, 1000):.6f}' for _ in range(n)) + '\n')
"
fi

if [ -f "$FILE_OUTPUT" ]; then
    echo "$FILE_OUTPUT exists. Output will be appended"
    INDEX=$(tail -n1 $FILE_OUTPUT | cut -d"," -f1)
    if [[ "$INDEX" =~ ^[0-9]+$ ]]; then
        INDEX=$((INDEX+1))
    else
        INDEX=1
    fi
else
    echo "$FILE_OUTPUT does not exist. A new file will be created"
    echo "trial_no,average_time,threads,language" > $FILE_OUTPUT
fi

CMD="$SCRIPT_PATH"
if [[ "$CMD" != ./* && "$CMD" != /* ]]; then
    CMD="./$CMD"
fi

for i in $(seq 1 $RETRIES); do
	echo "Trial no:" $i "for" $SCRIPT_PATH "with" $THREADS "threads."

	result=$($CMD $N $THREADS)
        RESULTS+=($result)
done

average=$(echo "${RESULTS[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')

echo $INDEX","$average","$THREADS","$LANGUAGE >> $FILE_OUTPUT