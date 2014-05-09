#!/usr/bin/env sh
if ! test -x check-progress-on-github-of-today; then
	echo "executable ./check-progress-on-github-of-today not found" >&2
	exit 1
fi

if ! ./check-progress-on-github-of-today >/dev/null; then
	ttytter -keyf=coma_ararat -status="@n_IMRC You have not yet made progress today. How about your progress?"
fi
