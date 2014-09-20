#!/usr/bin/env sh
which ttytter >/dev/null 2>/dev/null
RET=$?
if test $RET -ne 0; then
	echo "error: ttytter is not found"
	exit $RET
fi

DIR=~/.totweet/totweet

mkdir -p $DIR
cd $DIR

while true; do
	sleep 3
	for f in $(\ls -1trA); do
		if test -s $f; then
			ttytter -keyf=coma_ararat -status="$(cat $f)" -hold
		fi
		rm -f $f
	done
done
