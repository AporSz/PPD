#!/bin/bash
if [ ! -f "hw1-ppd-java/out/Main.class" ]; then
    mkdir -p hw1-ppd-java/out
    javac -d hw1-ppd-java/out hw1-ppd-java/src/Main.java
fi
java -cp hw1-ppd-java/out Main "$@"