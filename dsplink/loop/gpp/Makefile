
OBJS=CFG_system.o CFG_Linux.o CFG_ARM.o \
CFG_OMAP3530_SHMEM.o proc.o chnl.o msgq.o mpcs.o _mpcs.o _idm_usr.o \
ringio.o _ringio.o mplist.o _mplist.o pool.o _pool.o pool_arch.o \
_sync_usr.o _mem_usr.o drv_api.o _mpcs_os.o notify.o _notify.o \
loop.o \
loop_os.o main.o

DVSDKPATH=/opt/dvsdk
#DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
DSPLINKPATH=$(DVSDKPATH)/dsplink-1_65_02
#DSPLINKPATH=/home/jay/git/ti/TI-OMAP3-DSPLink/dsplink-1_65_02

CFLAGS=-O2 -DOS_LINUX -DMAX_DSPS=1 -DMAX_PROCESSORS=2 -DID_GPP=1 \
-DPROC_COMPONENT -DPOOL_COMPONENT -DNOTIFY_COMPONENT \
-DMPCS_COMPONENT -DRINGIO_COMPONENT -DMPLIST_COMPONENT \
-DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK -DCHNL_COMPONENT \
-DCHNL_ZCPY_LINK -DZCPY_LINK -DKFILE_DEFAULT \
-DOMAP3530 -DOMAP3530_PHYINTERFACE=SHMEM_INTERFACE \
-DGPP_SWI_MODE \
-I$(DSPLINKPATH)/dsplink/gpp/inc \
-I$(DSPLINKPATH)/dsplink/gpp/inc/usr \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/api \
-I$(DSPLINKPATH)/dsplink/gpp/src/api/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/api/Linux/2.6.18 \
-I$(DSPLINKPATH)/dsplink/gpp/src/samples/loop \
-I$(DSPLINKPATH)/dsplink/gpp/src/samples/loop/Linux \
-Wall -D_REENTRANT -DTRACE_USER
LDFLAGS=
LIBS=-lpthread -lc
CC=/usr/bin/arm-linux-gnueabihf-gcc
STRIP=/usr/bin/arm-linux-gnueabihf-strip

all: myloopgpp

myloopgpp: $(OBJS)
	$(CC) -o myloopgpp $(LDFLAGS) $(OBJS) $(LIBS)
	$(STRIP) myloopgpp

clean:
	rm -f $(OBJS) myloopgpp
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/config/all/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/api/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/api/Linux/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/samples/loop/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/samples/loop/Linux/%.c
	$(CC) $(CFLAGS) -c $<
