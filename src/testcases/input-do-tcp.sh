#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage:   $0 <json-do-file>"
    exit
fi

nc 127.0.0.1 9600 < $1
# Creating new line
echo
