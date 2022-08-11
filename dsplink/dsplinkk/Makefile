
DVSDKPATH=/opt/dvsdk
DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
LINUXKERNELPATH=/media/sdb1/jay/linux-3.2.102

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

all:
	sh copy_files.sh $(DSPLINKPATH)
	make PROFILE=RELEASE -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm modules V=1

clean:
	make -C $(LINUXKERNELPATH) M=$(PWD) ARCH=arm clean V=1
	rm -f *.c
