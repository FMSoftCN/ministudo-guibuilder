#! /bin/sh

binPath=/usr/local/bin/
etcPath=/usr/local/etc/guibuilder/

cp ./Debug/guibuilder $binPath

if ! test -d $etcPath ; then
	mkdir $etcPath
else
	rm -rf $etcPath/*
fi
cp -r etc/* $etcPath
chmod a+rw $etcPath/user-templates

respath=`awk -F= '/^respath/ {print $2} ' < etc/MiniGUI.cfg`

#cp res path
cp -r etc/msd ${respath}/bmp
#cp the MiniGUI.cfg
#mv $etcPath/MiniGUI.cfg ${HOME}/.MiniGUI.cfg

