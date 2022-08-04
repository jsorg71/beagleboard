#!/bin/sh

DVSDKPATH=/opt/dvsdk
DSPLINKPATH=$DVSDKPATH/dsplink_1_65_01_05_eng

echo "copying files from $DSPLINKPATH"
cp $DSPLINKPATH/dsplink/gpp/src/arch/CFG_map.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/dsp.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/shmem/Linux/omap3530_phy_shmem.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/shmem/Linux/omap3530_dspclk.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_hal.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_intgen.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_mmu.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_pwr.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_hal_boot.c .
cp $DSPLINKPATH/dsplink/gpp/src/arch/OMAP3530/omap3530_map.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/ldrv.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/ldrv_proc.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/DRV/ldrv_drv.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/DRV/shm_drv.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/IPS/ldrv_ips.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/IPS/ips.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/SMM/ldrv_smm.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/POOLS/ldrv_pool.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/POOLS/sma_pool.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/ldrv_msgq.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/MQT/ldrv_mqt.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/MQT/zcpy_mqt.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/ldrv_chnl.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/ldrv_chirps.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/DATA/ldrv_data.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/DATA/zcpy_data.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/RINGIO/ldrv_ringio.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/MPCS/ldrv_mpcs.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/MPLIST/ldrv_mplist.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/Linux/ldrv_os.c .
cp $DSPLINKPATH/dsplink/gpp/src/ldrv/Linux/ldrv_mpcs_os.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/gen_utils.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/list.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/idm.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/trc.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/linklog.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/no_loader.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_64x.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_55x.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_int.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_mem.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_shm.c .
cp $DSPLINKPATH/dsplink/gpp/src/gen/coff_file.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/osal.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/kfile.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/kfile_pseudo.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/prcs.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/user.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/print.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/mem.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/isr.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/dpc.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/sync.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/kfiledef.c .
cp $DSPLINKPATH/dsplink/gpp/src/osal/Linux/2.6.18/notify_knl.c .
cp $DSPLINKPATH/dsplink/gpp/src/pmgr/pmgr_chnl.c .
cp $DSPLINKPATH/dsplink/gpp/src/pmgr/pmgr_proc.c .
cp $DSPLINKPATH/dsplink/gpp/src/pmgr/pmgr_msgq.c .
cp $DSPLINKPATH/dsplink/gpp/src/pmgr/Linux/2.6.18/drv_pmgr.c .

echo "building dsplinkk.ko"

make

echo "done"
