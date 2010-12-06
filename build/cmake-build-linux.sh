#source env.sh
#hear set the source info
# set the build path
BUILD_PATH=
# set the source path
SRC_PATH=
PREFIX=${BUILD_PATH}/usr
PREFIX_PATH=$PREFIX

export BUILD_PATH SRC_PATH PREFIX PREFIX_PATH

if [ ! -d "$SRC_PATH" ]; then
    echo "the source path is invalidate [$SRC_PATH]"
    exit -1
fi


if test "x$PREFIX" = "x"; then
    PREFIX=/usr/local
fi

if test "x$PREFIX_PATH" != "x"; then
    CMAKE_PREFIX_PATH=$PREFIX_PATH
fi

COMMON_CONFIGS="-Dwith_libsuffix=msd \
    -Dwith_targetname:STRING=mstudio \
    -DCMAKE_INSTALL_PREFIX:PATH="${PREFIX}" \
    -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH}"\
    -DCMAKE_LIBRARY_PATH:PATH="${PREFIX}/lib"" 


function build()
{
    echo "========= build $1================"
    mkdir -p $1
    cd $1
    source ${SRC_PATH}/$1/build/cmake-pc-linux-mstudio.cfg
    eval "private_cfg=\${$1_cmake_cfgs}"
    cmake ${COMMON_CONFIGS} $private_cfg "${SRC_PATH}/$1"
    make
    make install
    cd ..
    echo "========= end build $1=========="
}

function build_all()
{
    build minigui
    build mgutils
    build mgplus
    build mgncs
    build guibuilder
}

function clean()
{
    rm -fr $1
}

if [ $# -le 0 ]; then
    build_all
    exit
fi

if test "x$1" = "xclean"; then
    shift
    clean $*
elif test "x$1" = "xcleanall"; then
    clean minigui mgutils mgplus mgncs guibuilder
else
    while test "x$1" != "x"; do
        build $1
        shift
    done
fi



