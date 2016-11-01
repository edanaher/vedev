#!/bin/sh

outfile=${1:-events.log}

result/bin/vedve > vedve.log 2>&1 &
pid=$!
sleep 1
evtest /dev/input/event15 > raw_events.log 2>&1 &
while ps | grep $pid; do
  sleep 1
done

cat events.log  | grep EV_KEY | sed 's/,//' | awk '{print $3" "$3-last; last=$3}' | sed 1d > $outfile
