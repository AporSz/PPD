#! /bin/bash

SCRIPT_PATH=$1
RETRIES=$2
THREADS=$3
FILE_OUTPUT=$4
LANGUAGE=$5
INDEX=1

N=1000000

RESULTS=()

if [ -f "$FILE_OUTPUT" ]; then
    echo "$FILE_OUTPUT exists. Output will be appended"
    INDEX=$(tail -n1 $FILE_OUTPUT | cut -d"," -f1)
    INDEX=$((INDEX+1))
else
    echo "$FILE_OUTPUT does not exist. A new file will be created"
    echo "trial_no,average_time,threads,language" > $FILE_OUTPUT
fi

for i in $(seq 1 $RETRIES); do
	echo "Trial no:" $i "for" $SCRIPT_PATH "with" $THREADS "threads."

	result=$(./$SCRIPT_PATH $N $THREADS)
        RESULTS+=($result)
done

average=$(echo "${RESULTS[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')

echo $INDEX","$average","$THREADS","$LANGUAGE >> $FILE_OUTPUT