#!/usr/bin/env bash
which ttytter >/dev/null 2>/dev/null
RET=$?
if test $RET -ne 0; then
	echo "error: ttytter is not found"
	exit $RET
fi

DIR=~/.totweet/totweet
FDIR=$DIR/../failed

mkdir -p $DIR $FDIR
cd $DIR

while true; do
	sleep 3
	for f in $(\ls -1trA); do
		if test -s $f; then
			ST="$(cat $f)"
			(
				ttytter -keyf=coma_ararat -status="$ST" -hold >/dev/null
				RET=$?
				if test $RET -ne 0; then
					echo "ttytter exited with status $RET" >&2
					REST="$(mktemp --tmpdir=$FDIR)"
					chmod 666 $REST
					echo "$ST" >$REST
					echo "the tweet is restored to $REST (original filename is $f)" >&2
				fi
			) &
		fi
		rm -f $f
	done
done
