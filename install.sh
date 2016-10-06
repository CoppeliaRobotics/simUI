#!/bin/sh

if [ "x$VREP_ROOT" = "x" ]; then
    echo "error: \$VREP_ROOT is not set" 1>&2
    exit 1
fi

if [ "`uname`" = "Darwin" ]; then
    TARGET="$VREP_ROOT/vrep.app/Contents/MacOS/"
    DLEXT=dylib
else
    TARGET="$VREP_ROOT"
    DLEXT=so
fi

make && \
cp -v "$(dirname "$0")/libv_repExtCustomUI.$DLEXT" "$TARGET"

