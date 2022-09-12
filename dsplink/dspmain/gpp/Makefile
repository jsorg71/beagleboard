
DVSDKPATH=/opt/dvsdk
#DSPLINKPATH=$(DVSDKPATH)/dsplink_1_65_01_05_eng
DSPLINKPATH=$(DVSDKPATH)/dsplink-1_65_02

CC=/usr/bin/arm-linux-gnueabihf-gcc
STRIP=/usr/bin/arm-linux-gnueabihf-strip

OBJS=dspmain.o dspmain_peer.o

CFLAGS=-O2 -DOS_LINUX -DMAX_DSPS=1 -DMAX_PROCESSORS=2 -DID_GPP=1 \
-DPROC_COMPONENT -DPOOL_COMPONENT -DMSGQ_COMPONENT -DMSGQ_ZCPY_LINK \
-DZCPY_LINK -DOMAP3530 -DOMAP3530_PHYINTERFACE=SHMEM_INTERFACE \
-DGPP_SWI_MODE \
-I$(DSPLINKPATH)/dsplink/gpp/inc/usr \
-I$(DSPLINKPATH)/dsplink/gpp/inc/sys/Linux \
-I.. -Wall -fPIC -D_REENTRANT -DTRACE_USER

LDFLAGS=

LIBS=../../libtidsplink/libtidsplink.a -lpthread

all: dspmain

dspmain: $(OBJS)
	$(CC) -o dspmain $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -r $(OBJS) dspmain

dsptest: dsptest.o
	$(CC) -o dsptest $(LDFLAGS) dsptest.o $(LIBS)

dsptestsck: dsptestsck.o libdspmain.o
	$(CC) -o dsptestsck $(LDFLAGS) dsptestsck.o libdspmain.o $(LIBS)
