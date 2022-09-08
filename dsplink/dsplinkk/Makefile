
DVSDKPATH=/opt/dvsdk
#DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
#DSPLINKPATH=/home/jay/git/ti/TI-OMAP3-DSPLink/dsplink-1_65_02
DSPLINKPATH=$(DVSDKPATH)/dsplink-1_65_02
#LINUXKERNELPATH=/media/sdb1/jay/linux-3.2.102
LINUXKERNELPATH=/home/jay/git/RobertCNelson/armv7-multiplatform/KERNEL

obj-m := dsplinkk.o

dsplinkk-y := CFG_map.o dsp.o omap3530_phy_shmem.o omap3530_dspclk.o \
omap3530.o omap3530_hal.o omap3530_hal_intgen.o omap3530_hal_mmu.o \
omap3530_hal_pwr.o omap3530_hal_boot.o omap3530_map.o ldrv.o \
ldrv_proc.o ldrv_drv.o shm_drv.o ldrv_ips.o ips.o ldrv_smm.o \
ldrv_pool.o sma_pool.o ldrv_msgq.o ldrv_mqt.o zcpy_mqt.o ldrv_chnl.o \
ldrv_chirps.o ldrv_data.o zcpy_data.o ldrv_ringio.o ldrv_mpcs.o \
ldrv_mplist.o ldrv_os.o ldrv_mpcs_os.o gen_utils.o list.o idm.o \
trc.o linklog.o coff.o no_loader.o coff_64x.o coff_55x.o coff_int.o \
coff_mem.o coff_shm.o coff_file.o osal.o kfile.o kfile_pseudo.o \
prcs.o user.o print.o mem.o isr.o dpc.o sync.o kfiledef.o \
notify_knl.o pmgr_chnl.o pmgr_proc.o pmgr_msgq.o drv_pmgr.o

ccflags-y += -DTRACE_KERNEL -DOS_LINUX -DMAX_DSPS=1 -DMAX_PROCESSORS=2 \
-DID_GPP=1 -DOMAP3530 -DPROC_COMPONENT -DPOOL_COMPONENT \
-DNOTIFY_COMPONENT -DMPCS_COMPONENT -DRINGIO_COMPONENT \
-DMPLIST_COMPONENT -DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK -DCHNL_COMPONENT \
-DCHNL_ZCPY_LINK -DZCPY_LINK -DKFILE_DEFAULT -DOMAP3530 \
-DOMAP3530_PHYINTERFACE=SHMEM_INTERFACE -DGPP_SWI_MODE \
-I$(DSPLINKPATH)/dsplink/gpp/inc \
-I$(DSPLINKPATH)/dsplink/gpp/inc/usr \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/arch \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/arch/OMAP3530 \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/arch/OMAP3530/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/gen \
-I$(DSPLINKPATH)/dsplink/gpp/src/gen/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/gen/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/osal \
-I$(DSPLINKPATH)/dsplink/gpp/src/osal/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/pmgr \
-I$(DSPLINKPATH)/dsplink/gpp/src/pmgr/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/pmgr/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/api \
-I$(DSPLINKPATH)/dsplink/gpp/src/api/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/api/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/DRV \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/DATA \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/MPCS \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/MPLIST \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/MQT \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/IPS \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/SMM \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/RINGIO \
-I$(DSPLINKPATH)/dsplink/gpp/src/ldrv/POOLS

all: CFG_map.c dsp.c omap3530_phy_shmem.c omap3530_dspclk.c \
omap3530.c omap3530_hal.c omap3530_hal_intgen.c omap3530_hal_mmu.c \
omap3530_hal_pwr.c omap3530_hal_boot.c omap3530_map.c ldrv.c \
ldrv_proc.c ldrv_drv.c shm_drv.c ldrv_ips.c ips.c ldrv_smm.c \
ldrv_pool.c sma_pool.c ldrv_msgq.c ldrv_mqt.c zcpy_mqt.c ldrv_chnl.c \
ldrv_chirps.c ldrv_data.c zcpy_data.c ldrv_ringio.c ldrv_mpcs.c \
ldrv_mplist.c ldrv_os.c ldrv_mpcs_os.c gen_utils.c list.c idm.c \
trc.c linklog.c coff.c no_loader.c coff_64x.c coff_55x.c coff_int.c \
coff_mem.c coff_shm.c coff_file.c osal.c kfile.c kfile_pseudo.c \
prcs.c user.c print.c mem.c isr.c dpc.c sync.c kfiledef.c \
notify_knl.c pmgr_chnl.c pmgr_proc.c pmgr_msgq.c drv_pmgr.c
	make PROFILE=RELEASE -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

clean:
	make -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean
	rm -f *.c

CFG_map.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/CFG_map.c .

dsp.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/dsp.c .

omap3530_phy_shmem.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/shmem/Linux/omap3530_phy_shmem.c .

omap3530_dspclk.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/shmem/Linux/omap3530_dspclk.c .

omap3530.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530.c .

omap3530_hal.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_hal.c .

omap3530_hal_intgen.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_intgen.c .

omap3530_hal_mmu.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_mmu.c .

omap3530_hal_pwr.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_pwr.c .

omap3530_hal_boot.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_boot.c .

omap3530_map.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/arch/OMAP3530/omap3530_map.c .

ldrv.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/ldrv.c .

ldrv_proc.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/ldrv_proc.c .

ldrv_drv.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/DRV/ldrv_drv.c .

shm_drv.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/DRV/shm_drv.c .

ldrv_ips.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/IPS/ldrv_ips.c .

ips.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/IPS/ips.c .

ldrv_smm.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/SMM/ldrv_smm.c .

ldrv_pool.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/POOLS/ldrv_pool.c .

sma_pool.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/POOLS/sma_pool.c .

ldrv_msgq.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/ldrv_msgq.c .

ldrv_mqt.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/MQT/ldrv_mqt.c .

zcpy_mqt.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/MQT/zcpy_mqt.c .

ldrv_chnl.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/ldrv_chnl.c .

ldrv_chirps.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/ldrv_chirps.c .

ldrv_data.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/DATA/ldrv_data.c .

zcpy_data.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/DATA/zcpy_data.c .

ldrv_ringio.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/RINGIO/ldrv_ringio.c .

ldrv_mpcs.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/MPCS/ldrv_mpcs.c .

ldrv_mplist.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/MPLIST/ldrv_mplist.c .

ldrv_os.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/Linux/ldrv_os.c .

ldrv_mpcs_os.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/ldrv/Linux/ldrv_mpcs_os.c .

gen_utils.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/gen_utils.c .

list.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/list.c .

idm.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/idm.c .

trc.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/trc.c .

linklog.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/linklog.c .

coff.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff.c .

no_loader.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/no_loader.c .

coff_64x.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_64x.c .

coff_55x.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_55x.c .

coff_int.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_int.c .

coff_mem.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_mem.c .

coff_shm.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_shm.c .

coff_file.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/gen/coff_file.c .

osal.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/osal.c .

kfile.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/kfile.c .

kfile_pseudo.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/kfile_pseudo.c .

prcs.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/prcs.c .

user.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/user.c .

print.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/print.c .

mem.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/mem.c .

isr.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/isr.c .

dpc.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/dpc.c .

sync.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/sync.c .

kfiledef.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/kfiledef.c .

notify_knl.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/osal/Linux/2.6.18/notify_knl.c .

pmgr_chnl.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/pmgr/pmgr_chnl.c .

pmgr_proc.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/pmgr/pmgr_proc.c .

pmgr_msgq.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/pmgr/pmgr_msgq.c .

drv_pmgr.c:
	cp $(DSPLINKPATH)/dsplink/gpp/src/pmgr/Linux/2.6.18/drv_pmgr.c .
