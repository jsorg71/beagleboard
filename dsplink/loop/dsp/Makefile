
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
loop_config.obj loop_config_local.obj main.obj swiLoop.obj tskLoop.obj \
loopcfg.obj loopcfg_c.obj

DVSDKPATH=/opt/dvsdk
DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
DSPBIOSPATH=$(DVSDKPATH)/dspbios_5_41_03_17
CGTPATH=$(DVSDKPATH)/cgt6x_6_1_14
XDCPATH=$(DVSDKPATH)/xdctools_3_16_03_36

CFLAGS=-O2 -DMAX_DSPS=1 -DMAX_PROCESSORS=2 -DID_GPP=1 -DOMAP3530 \
-DPROC_COMPONENT -DPOOL_COMPONENT -DNOTIFY_COMPONENT -DMPCS_COMPONENT \
-DRINGIO_COMPONENT -DMPLIST_COMPONENT -DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK \
-DCHNL_COMPONENT -DCHNL_ZCPY_LINK -DZCPY_LINK -DPROCID=0 -DOMAP3530 \
-DOMAP3530_INTERFACE=SHMEM_INTERFACE -DPHYINTERFACE=SHMEM_INTERFACE \
-DDSP_SWI_MODE \
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
-I$(DSPLINKPATH)/dsplink/dsp/src/samples/loop \
-I$(DSPLINKPATH)/dsplink/dsp/src/samples/loop/DspBios \
-q -pdr -pdv -pden -ml3 -mv6400+

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
$(DSPLINKPATH)/dsplink/dsp/src/samples/loop;\
."

all: loop.out

loop.out: $(OBJS)
	$(CC) -z $(LDFLAGS) -c -q -x -oloop.out -mloop.map $(OBJS) loopcfg.cmd
	$(STRIP) loop.out

clean:
	rm -f $(OBJS) loop.out loop.cdb loopcfg* loop.map

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

%.obj: $(DSPLINKPATH)/dsplink/dsp/src/samples/loop/%.c
	$(CC) $(CFLAGS) -DTSK_MODE -c $<

loopcfg.obj:
	$(TCONF) -Dconfig.importPath=$(IMPORT_PATH) $(DSPLINKPATH)/dsplink/dsp/src/samples/loop/DspBios/5.XX/OMAP3530/loop.tcf 0
	$(CC) $(CFLAGS) -c loopcfg.s62
	$(CC) $(CFLAGS) -c loopcfg_c.c
