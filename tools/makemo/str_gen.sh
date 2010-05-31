#!/bin/sh

if [ $# != 1 ]; then
	echo "Usage: $0 <locale>"
	exit 0
fi

if [ ! -f "xml2an" ]; then
	echo "Please run makefile in current directory"
	exit 0
fi

./xml2an ../../etc/mainframe.cfg //@caption > cfg.c
./xml2an ../../etc/imgeditor.layout //@caption >> cfg.c
./xml2an ../../etc/uieditor.layout //@caption >> cfg.c
./xml2an ../../etc/uieditor/control.cfg //@caption >> cfg.c
./xml2an ../../etc/uieditor/mobile.cfg //@caption >> cfg.c
./xml2an ../../etc/uieditor/control.cfg //name >> cfg.c
./xml2an ../../etc/uieditor/mobile.cfg //name >> cfg.c

find ../../ \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) | xgettext -k_ -f -

sed -e 's/charset=CHARSET/charset=utf-8/g' messages.po > messages_tmp.po 

if [ -e "$1.po" ]
then
	sed -e 's/charset=CHARSET/charset=utf-8/g' $1.po > tmp.po 
	msgcat --use-first -o $1.po tmp.po messages_tmp.po
	rm messages.po messages_tmp.po tmp.po
else
	mv messages.po $1.po
	rm messages_tmp.po
fi
