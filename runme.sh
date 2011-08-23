#!/bin/bash


DIRECTORY='/home/castongj/'
#
# setarch allows for aslr disabling for each program. Can be run as user.
#
##detect if aslr is on, bail if it is
#ASLR=`cat /proc/sys/kernel/randomize_va_space`
#if [ "$ASLR" -eq 1 ] 
#then
#    echo "ASLR needs to be turned off. As root 'echo 0> /proc/sys/kernel/randomize_va_space'"
#fi

cd "$DIRECTORY/input"
for file in *
do
    bash "$DIRECTORY"/runme2.sh "$DIRECTORY" "$file"
done

#find "$DIRECTORY/input/" -type f |parallel ./runme2.sh "$DIRECTORY" {}

cd "$DIRECTORY

python minimal-coverage.py
