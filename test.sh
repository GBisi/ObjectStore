#!/bin/bash

echo "START TEST"

>testout.log

echo "START PHASE 1"
for i in $(seq 1 50); do 
	./client client$i 1 >> testout.log & 
	pids[${i}]=$!
done

for pid in ${pids[*]}; do
    wait $pid
done
	
echo "START PHASE 2"
for i in $(seq 1 30); do 
	./client client$i 2 >> testout.log & 
	pids[${i}]=$!
done

echo "START PHASE 3"

for i in $(seq 31 50); do 
	./client client$i 3 >> testout.log & 
	pids[${i}]=$!
done

for pid in ${pids[*]}; do
    wait $pid
done

echo "END TEST"
