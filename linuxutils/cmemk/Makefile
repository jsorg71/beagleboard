
#DVSDKPATH=/opt/dvsdk
#LINUXUTILSPATH=$(DVSDKPATH)/linuxutils_2_26_02_05
LINUXUTILSPATH=/home/jay/git/ti/ipc-ludev
#LINUXKERNELPATH=/media/sdb1/jay/linux-3.2.102
LINUXKERNELPATH=/home/jay/git/RobertCNelson/armv7-multiplatform/KERNEL

obj-m := cmemk.o

ccflags-y += \
-I$(LINUXUTILSPATH)/include
#-I$(LINUXUTILSPATH)/packages/ti/sdo/linuxutils/cmem/src/module

all: cmemk.ko

cmemk.ko: cmemk.c
	make PROFILE=RELEASE -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

clean:
	make -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
	rm -f *.c

cmemk.c:
	#cp $(LINUXUTILSPATH)/packages/ti/sdo/linuxutils/cmem/src/module/cmemk.c .
	cp $(LINUXUTILSPATH)/src/cmem/module/cmemk.c .
