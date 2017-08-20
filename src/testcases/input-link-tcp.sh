#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage:   $0 <json-link-file>"
    exit
fi

nc 127.0.0.1 9601 < $1
