#!/bin/sh

LINUXUTILSPATH=$1

echo "copying files from $LINUXUTILSPATH"
cp $LINUXUTILSPATH/packages/ti/sdo/linuxutils/cmem/src/module/cmemk.c .
