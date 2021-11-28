#!bin/bash

make clean
make build
rm output.txt
make run > output.txt
