#!/bin/sh
cmd="{gsub(/$3/, \"_RARE_\"); print}"
echo "'"$cmd"'"
cat $1 | awk "$cmd" > $2
