
OBJS=failure.obj linklog.obj \
dsplinkips.obj ips.obj \
hal.obj hal_interrupt.obj \
dsplink.obj shm_drv.obj \
dsplinkpool.obj buf_pool.obj sma_pool.obj \
mpcs.obj _mpcs.obj \
mplist.obj _mplist.obj \
dsplinkmqt.obj dsplinkmsgq.obj zcpy_mqt.obj \
dsplinkdata.obj zcpy_data.obj \
notify.obj _notify.obj \
ringio.obj _ringio.obj \
dspmain.obj dspmain_edma.obj dspmain_mqt_sma.obj \
dspmaincfg.obj dspmaincfg_c.obj \
bios_edma3_drv_sample_cs.obj \
bios_edma3_drv_sample_omap35xx_cfg.obj \
bios_edma3_drv_sample_init.obj

DVSDKPATH=/opt/dvsdk
#DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
DSPLINKPATH=$(DVSDKPATH)/dsplink-1_65_02
DSPBIOSPATH=$(DVSDKPATH)/dspbios_5_41_03_17
CGTPATH=$(DVSDKPATH)/cgt6x_6_1_14
XDCPATH=$(DVSDKPATH)/xdctools_3_16_03_36
EDMA3PATH=$(DVSDKPATH)/edma3lld_01_11_02_05

CFLAGS=-O2 -DMAX_DSPS=1 -DMAX_PROCESSORS=2 -DID_GPP=1 -DOMAP3530 \
-DPROC_COMPONENT -DPOOL_COMPONENT -DNOTIFY_COMPONENT -DMPCS_COMPONENT \
-DRINGIO_COMPONENT -DMPLIST_COMPONENT -DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK \
-DCHNL_COMPONENT -DCHNL_ZCPY_LINK -DZCPY_LINK -DPROCID=0 -DOMAP3530 \
-DOMAP3530_INTERFACE=SHMEM_INTERFACE -DPHYINTERFACE=SHMEM_INTERFACE \
-DDSP_TSK_MODE \
-I$(DSPLINKPATH)/dsplink/dsp/inc \
-I$(DSPLINKPATH)/dsplink/dsp/inc/DspBios \
-I$(DSPLINKPATH)/dsplink/dsp/inc/DspBios/5.XX \
-I$(DSPLINKPATH)/dsplink/dsp/inc/DspBios/5.XX/OMAP3530 \
-I$(DSPLINKPATH)/dsplink/dsp/inc/OMAP3530 \
-I$(DSPLINKPATH)/dsplink/dsp/inc/C64XX \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/gen \
-I$(DSPBIOSPATH)/packages/ti/bios/include \
-I$(CGTPATH)/include \
-I$(DSPBIOSPATH)/packages/ti/rtdx/include/c6000 \
-I$(DSPBIOSPATH)/packages/ti/psl/include \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/hal \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/hal/DspBios/OMAP3530 \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/ips \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/drv \
-I$(DSPLINKPATH)/dsplink/dsp/src/base/drv/DspBios \
-I$(DSPLINKPATH)/dsplink/dsp/src/pools \
-I$(DSPLINKPATH)/dsplink/dsp/src/pools/DspBios \
-I$(DSPLINKPATH)/dsplink/dsp/src/mpcs \
-I$(DSPLINKPATH)/dsplink/dsp/src/mplist \
-I$(DSPLINKPATH)/dsplink/dsp/src/msg \
-I$(DSPLINKPATH)/dsplink/dsp/src/msg/DspBios \
-I$(DSPLINKPATH)/dsplink/dsp/src/data \
-I$(DSPLINKPATH)/dsplink/dsp/src/data/DspBios \
-I$(DSPLINKPATH)/dsplink/dsp/src/notify \
-I$(DSPLINKPATH)/dsplink/dsp/src/ringio \
-I. -I.. -I$(EDMA3PATH)/packages \
-q -pdr -pdv -pden -ml3 -mv6400+

# -DDSP_SWI_MODE \
# -DDSP_TSK_MODE \
# --disable:sploop

LDFLAGS= \
-i$(DSPBIOSPATH)/packages/ti/bios/lib \
-i$(CGTPATH)/lib \
-i$(DSPBIOSPATH)/packages/ti/rtdx/lib/c6000 \
-i$(DSPBIOSPATH)/packages/ti/psl/lib
LIBS=

CC=$(CGTPATH)/bin/cl6x
TCONF=$(XDCPATH)/tconf
STRIP=$(CGTPATH)/bin/strip6x

IMPORT_PATH="$(DSPBIOSPATH)/packages;\
$(DSPBIOSPATH)/packages/ti/bios;\
$(DSPLINKPATH)/dsplink/dsp/inc/DspBios;\
$(DSPLINKPATH)/dsplink/dsp/inc/DspBios/5.XX;\
."

EDMA3LIBS=$(EDMA3PATH)/packages/ti/sdo/edma3/drv/lib/Release/edma3_drv_bios.lib \
$(EDMA3PATH)/packages/ti/sdo/edma3/rm/lib/omap35xx/Release/edma3_rm_bios.lib

all: dspmain.out

dspmain.out: $(OBJS)
	$(CC) -z $(LDFLAGS) -c -q -x -odspmain.out -mdspmain.map $(OBJS) $(EDMA3LIBS) dspmaincfg.cmd
	$(STRIP) dspmain.out

clean:
	rm -f $(OBJS) dspmain.out dspmain.cdb dspmaincfg* dspmain.map

%.obj: %.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/gen/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/ips/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/ips/DspBios/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/hal/DspBios/OMAP3530/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/drv/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/base/drv/DspBios/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/pools/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/pools/DspBios/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/mpcs/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/mplist/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/msg/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/msg/DspBios/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/data/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/data/DspBios/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/notify/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/ringio/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(EDMA3PATH)/packages/ti/sdo/edma3/drv/sample/src/%.c
	$(CC) $(CFLAGS) -c $<

%.obj: $(EDMA3PATH)/packages/ti/sdo/edma3/drv/sample/src/configs/%.c
	$(CC) $(CFLAGS) -c $<

dspmaincfg.obj:
	$(TCONF) -Dconfig.importPath=$(IMPORT_PATH) dspmain.tcf 0
	$(CC) $(CFLAGS) -c dspmaincfg.s62
	$(CC) $(CFLAGS) -c dspmaincfg_c.c
