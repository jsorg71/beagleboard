
DVSDKPATH=/opt/dvsdk
LINUXUTILSPATH=$(DVSDKPATH)/linuxutils_2_26_02_05
LINUXKERNELPATH=/media/sdb1/jay/linux-3.2.102

obj-m := cmemk.o

ccflags-y += -DUSE_UDEV -DMAX_POOLS=128 \
-I$(LINUXUTILSPATH)/packages/ti/sdo/linuxutils/cmem/src/module

all:
	sh copy_files.sh $(LINUXUTILSPATH)
	make PROFILE=RELEASE -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm modules V=1

clean:
	make -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm clean V=1
	rm -f *.c
