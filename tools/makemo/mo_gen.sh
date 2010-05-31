#!/bin/sh

if [ $# != 1 ]; then
	echo "Usage: $0 <locale>"
	exit 0
fi

if [ ! -f "$1.po" ]; then
	echo "please generate the <$1.po> first"
	exit 0
fi

msgfmt -o ../../etc/lang/$1.mo $1.po
