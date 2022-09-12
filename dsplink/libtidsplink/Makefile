
OBJS=CFG_system.o CFG_Linux.o CFG_ARM.o \
CFG_OMAP3530_SHMEM.o proc.o msgq.o _idm_usr.o \
pool.o _pool.o pool_arch.o \
_sync_usr.o _mem_usr.o drv_api.o

DVSDKPATH=/opt/dvsdk
#DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
DSPLINKPATH=$(DVSDKPATH)/dsplink-1_65_02
#DSPLINKPATH=/home/jay/git/ti/TI-OMAP3-DSPLink/dsplink-1_65_02

CFLAGS=-O2 -DOS_LINUX -DMAX_DSPS=1 -DMAX_PROCESSORS=2 -DID_GPP=1 \
-DPROC_COMPONENT -DPOOL_COMPONENT -DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK \
-DZCPY_LINK -DOMAP3530 -DOMAP3530_PHYINTERFACE=SHMEM_INTERFACE \
-DGPP_SWI_MODE \
-I$(DSPLINKPATH)/dsplink/gpp/inc/usr \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux \
-I$(DSPLINKPATH)/dsplink/gpp/src/api \
-I$(DSPLINKPATH)/dsplink/gpp/src/api/Linux \
-Wall -fPIC -D_REENTRANT -DTRACE_USER
LDFLAGS=
LIBS=-lpthread -lc

CC=/usr/bin/arm-linux-gnueabihf-gcc
AR=/usr/bin/arm-linux-gnueabihf-ar
STRIP=/usr/bin/arm-linux-gnueabihf-strip

all: libtidsplink.so libtidsplink.a

libtidsplink.so: $(OBJS)
	$(CC) -shared -o libtidsplink.so $(LDFLAGS) $(OBJS) $(LIBS)
	$(STRIP) libtidsplink.so

libtidsplink.a: $(OBJS)
	$(AR) -rv libtidsplink.a $(OBJS)

clean:
	rm -f $(OBJS) libtidsplink.*

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/config/all/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/api/%.c
	$(CC) $(CFLAGS) -c $<

%.o: $(DSPLINKPATH)/dsplink/gpp/src/api/Linux/%.c
	$(CC) $(CFLAGS) -c $<
