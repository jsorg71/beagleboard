#!/bin/sh

DVSDKPATH=/opt/dvsdk
LINUXUTILSPATH=$DVSDKPATH/linuxutils_2_26_02_05

echo "copying files from $LINUXUTILSPATH"
cp $LINUXUTILSPATH/packages/ti/sdo/linuxutils/cmem/src/module/cmemk.c .
