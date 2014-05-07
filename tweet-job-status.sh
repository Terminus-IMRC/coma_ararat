#!/usr/bin/env sh

TOTWEET_DIR=~/.totweet/totweet
mkdir -p $TOTWEET_DIR

usage(){
	echo "usage: $0 [start|progress|end]" >&2
}

if test $# -ne 1; then
	echo "$0: invalid the number of arguments" >&2
	usage
	exit 1
fi

case $1 in
	start|progress|end);;
	*) echo "$0: invalid operator" >&2
	   usage
	   exit 1;;
esac

p="@n_IMRC $USER: job"
if test -n "$JOB_NAME"; then
	p="$p $JOB_NAME"
fi
if test -n "$JOB_ID"; then
	p="$p #$JOB_ID"
fi
if test -n "$JOB_NUM_NODES"; then
	p="$p on $JOB_NUM_NODES node(s)"
fi
p="$p:"

case $1 in
	start) echo "$p started ($(date))" >$TOTWEET_DIR/$USER.$JOB_ID.01;;
	progress) echo "$p progressing ($(date))" >$TOTWEET_DIR/$USER.$JOB_ID.02;;
	end) echo "$p finished ($(date))" >$TOTWEET_DIR/$USER.$JOB_ID.03;;
esac
