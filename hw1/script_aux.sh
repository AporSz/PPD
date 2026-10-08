#!/bin/bash
if [ ! -f "hw1-ppd-cpp/build/hw1_ppd_cpp" ]; then
    mkdir -p hw1-ppd-cpp/build
    (cd hw1-ppd-cpp/build && cmake .. && make)
fi

./script.sh hw1-ppd-cpp/build/hw1_ppd_cpp 10 4 results.csv cpp
./script.sh aux_java.sh 10 4 results.csv java