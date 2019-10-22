# miniStudio GUIBuilder

## building miniStudio GUIBuilder 

* build dependencies

```
# build minigui

$ ./autogen.sh
$ build/buildlib-linux-pc-mstudio && make -j4  && make install


# build mgutils

$ ./autogen.sh
$ build/buildlib-linux-pc-mstudio && make -j4  && make -j4 && make install

# build mgplus

$ ./autogen.sh
$ build/buildlib-linux-pc-mstudio && make -j4  && make install

# build mgncs

$ ./autogen.sh
$ build/buildlib-linux-pc-mstudio && make -j4 && make install

# build mgeff

$ ./autogen.sh
$ ./configure $prefix_config --with-targetname=mstudio  --with-libsuffix=msd && make -j4 && make install


# build chipmunk() 
$ cd 3rd-party/chipmunk-5.3.1
$ cmake -DCMAKE_INSTALL_PREFIX=$DEST_P
$ make -j4  && make install

```

* build GUIBuilder

```

# build GUIBuilder

$ ./autogen.sh
$ ./configure $prefix_config --enable-werror && make -j4 && make install

# resourse
$ cd etc
$ make -j4 && make install

# tools/mgcfg-trans
$ cd tools/mgcfg-trans
$ rm -f mgcfg-trans
$ gcc lex.yy.c -o mgcfg-trans
$ cp -v mgcfg-trans  DEST/bin/

# tools/res2c
$ cd tools/res2c
$ make clean
$ make
$ cp -v cfg2c.py  $DEST_P/bin/
$ cp -v res2c $DEST_P/bin/

```

