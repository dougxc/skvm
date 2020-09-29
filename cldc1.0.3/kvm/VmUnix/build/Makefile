TOP=../../..
include $(TOP)/build/Makefile.inc

ifeq ($(PLATFORM), linux)
  export XPM=true
  export GCC=true
endif

ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

j =

# cancel automatic SCCS get
%: SCCS/s.%

ifeq ($(XPM), true)
  CLEANUPXPM =  .includexpm$(j)$(g)
else
  CLEANUPXPM =  .noincludexpm$(j)$(g)
endif

ifeq ($(DEBUG_COLLECTOR), true)
COLLECTOR = collectorDebug.c
else
COLLECTOR = collector.c
endif

SRCFILES  = cache.c class.c fields.c frame.c garbage.c $(COLLECTOR)   \
            global.c interpret.c execute.c loader.c main.c native.c   \
            property.c thread.c nativeCore.c loaderFile.c hashtable.c \
            verifier.c log.c jar.c inflate.c  stackmap.c profiling.c  \
	    pool.c runtime_md.c StartJVM.c fakeStaticMemory.c         \
	    nativeFunctionTableUnix.c events.c resource.c             \
	    nativeSpotlet.c nativeGraphics.c                          \
	    datagramProtocol.c datagramProtocol_md.c                  \
	    socketProtocol.c socketProtocol_md.c                      \
	    commProtocol.c commProtocol_md.c

ifeq ($(DEBUG), true)
   SRCFILES += debugger.c debuggerSocketIO.c debuggerOutputStream.c debuggerInputStream.c

else
   OTHER_FLAGS += -DENABLE_JAVA_DEBUGGER=0
endif

ifeq ($(EXCESSIVE_GARBAGE_COLLECTION), true)
   OTHER_FLAGS += -DEXCESSIVE_GARBAGE_COLLECTION=1
endif

ifeq ($(VERY_EXCESSIVE_GARBAGE_COLLECTION), true)
   OTHER_FLAGS += -DEXCESSIVE_GARBAGE_COLLECTION=1
   OTHER_FLAGS += -DVERY_EXCESSIVE_GARBAGE_COLLECTION=1
endif

ifeq ($(SVM), true)
   OTHER_FLAGS += -DSVM=1
   SRCFILES += crypto.c crypto_provider_MD5RSABasic.c cbs.c
endif

ifeq ($(USE_JAM), true)
   OTHER_FLAGS += -DUSE_JAM=1
   SRCFILES += jam.c jamParse.c jamHttp.c jamStorage.c
   j = _j
else

endif

# The src for java.lang.Class still has to be manually edited to uncomment
# the class initialization code at the bottom if this option is being used.
ifeq ($(CLASS_INITIALIZATION_IN_JAVA), true)
    OTHER_FLAGS += -DCLASS_INITIALIZATION_IN_JAVA=1
endif

ifeq ($(ROMIZING), false)
   ROMFLAGS = -DROMIZING=0
else
   SRCFILES += ROMjavaUnix.c
endif

all: kvm$(j)$(g)

OBJFILES = $(patsubst %.c,obj$(j)$(g)/%.o,$(SRCFILES))

vpath %.c  $(TOP)/kvm/VmCommon/src/ $(TOP)/kvm/VmUnix/src/ \
           $(TOP)/kvm/VmExtra/src/ $(TOP)/tools/jcc/ $(TOP)/jam/src

ifeq ($(PLATFORM), solaris)
LIBS =    -lXt -lX11 -lm -lsocket -lnsl
CPPFLAGS = -DUNIX -DSOLARIS -D$(ARCH) \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/kvm/VmCommon/h \
	   -I$(TOP)/kvm/VmUnix/h -I$(TOP)/jam/h -I$(TOP)/kvm/VmCommon/src

XWINFLAGS =  -I/usr/openwin/include
endif

ifeq ($(PLATFORM), linux)
LIBS =    -L/usr/X11R6/lib -lXt -lX11 -lm -lnsl -lICE -lSM
CPPFLAGS = -DUNIX -DLINUX -D$(ARCH) \
	   -I$(TOP)/kvm/VmCommon/h -I$(TOP)/kvm/VmUnix/h \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/jam/h
XWINFLAGS =  -I/usr/X11R6/include
endif

ifeq ($(XPM), true)
    LIBS += -lXpm
    XWINFLAGS += -I/usr/local/include -DUSE_XPM=1
    XPM_BITMAPS = blank.xpm
endif


ifeq ($(GCC), true)
   CC = gcc
   CFLAGS =  -Wall $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
   DEBUG_FLAG = -g
   OPTIMIZE_FLAG = -O2
else
   CC = cc
   CFLAGS = -Xa $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
   DEBUG_FLAG = -g -xsb
   OPTIMIZE_FLAG = -xO2
endif

DEBUG_FLAG += -DINCLUDEDEBUGCODE=1
#DEBUG_FLAG += -DUSESTATIC=1

$(TOP)/tools/jcc/ROMjavaUnix.c $(TOP)/tools/jcc/nativeFunctionTableUnix.c: jccUnix

.PHONY:  jccUnix
jccUnix:
	@(cd $(TOP)/tools/jcc; $(MAKE) unix)

kvm$(j)$(g): obj$j$g/ $(CLEANUPXPM) $(OBJFILES)
	@echo "Linking ... $@"
	@$(CC) $(OBJFILES) $(EXTRALDFLAGS) -o $@ $(LIBS)

clean:
	rm -rf core kvm* .noincludexpm* ./obj* ./SunWS_cache .sb
	rm -rf $(TOP)/tools/jcc/ROMjavaUnix.c $(TOP)/tools/jcc/nativeFunctionTableUnix.c

obj$(j)$g/execute.o : execute.c bytecodes.c

obj$(j)/%.o: %.c
	@echo "... $@"
	@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(XWINFLAGS) $(OPTIMIZE_FLAG) -c -o $@ $<

obj$(j)_g/%.o: %.c
	@echo "... $@"
	@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(DEBUG_FLAG) -c -o $@ $<

obj$j$g/:
		mkdir -p $@

.includexpm$(j)$(g):
		@if [ ! -f .includexpm$(j)$(g) ]; then \
		    touch .includexpm$(j)$(g); \
		    rm -f .noincludexpm$(j)$(g); \
		    rm -rf obj$(j)$(g)/nativeGraphics.o; \
		fi

.noincludexpm$(j)$(g):
		@if [ ! -f .noincludexpm$(j)$(g) ]; then \
		    touch .noincludexpm$(j)$(g); \
		    rm -f .includexpm$(j)$(g); \
		    rm -rf obj$(j)$(g)/nativeGraphics.o; \
		fi
ifeq ($(GCC), true)

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) $(DEBUG_FLAG) -E -o $@ $<

%_g.i: %.c .FORCE
	$(CC) $(EXTRAFLAGS) $(CFLAGS) $(DEBUG_FLAG) -E -o $@ $<

else

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) $(DEBUG_FLAG) -E $< > $@

endif

%.s: %.c .FORCE
	@echo "... $@"
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -S -o $@ $<


regression:
	CLASSPATH=../../../api/classes:../../../samples/classes kvm tests.RegressionTest

.FORCE:
