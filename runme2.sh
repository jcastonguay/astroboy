#!/bin/bash

DIRECTORY=$1
file=$2
echo "$DIRECTORY $file"

PROGRAM='/bin/ls'

setarch i386 ~/pin/pin -t ~/obj-intel64/corpusdist -o "$DIRECTORY/$file.out" -- "$PROGRAM" "$DIRECTORY/input/$file"
#if file does not exist report crash
#check file for crash canary

canary=`tail -n1 "$DIRECTORY/$file.out"|grep '^END'`
if [ -z "$canary"  ] #file will not exist if pin did not finish
then
    cp "$DIRECTORY/input/$file" "$DIRECTORY/crashes/"
fi
sort -n "$DIRECTORY/$file.out" |uniq >"$DIRECTORY/coverage/$file.coverage"
rm "$DIRECTORY/$file.out"