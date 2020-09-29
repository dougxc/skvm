#
# Makefile for GNU make, adapted from the solaris version
# @(#)GNUmakefile	1.5 99/11/22
#

TOP=../../..
include $(TOP)/build/Makefile.inc

ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

j =

ifeq ($(DEBUG_COLLECTOR), true)
COLLECTOR = collectorDebug.c
else
COLLECTOR = collector.c
endif

SRCFILES  = cache.c class.c fields.c frame.c garbage.c $(COLLECTOR)   \
            global.c interpret.c execute.c loader.c main.c native.c   \
            property.c thread.c nativeCore.c loaderFile.c hashtable.c \
            verifier.c log.c jar.c inflate.c  stackmap.c profiling.c  \
	    pool.c events.c resource.c StartJVM.c fakeStaticMemory.c  \
	    nativeFunctionTableWin.c runtime_md.c runtime2_md.c       \
	    nativeSpotlet.c winGraphics.c nativeGraphics.c            \
	    datagramProtocol.c datagramProtocol_md.c                  \
	    socketProtocol.c socketProtocol_md.c                      \
	    commProtocol.c commProtocol_md.c async.c

ifeq ($(DEBUG), true)
   SRCFILES += debugger.c debuggerSocketIO.c debuggerOutputStream.c debuggerInputStream.c
   OTHER_FLAGS += -DENABLE_JAVA_DEBUGGER=1
else
   OTHER_FLAGS += -DENABLE_JAVA_DEBUGGER=0
endif

OTHER_FLAGS +=-DPADTABLE=1

ifeq ($(USE_JAM), true)
   OTHER_FLAGS += -DUSE_JAM=1
   SRCFILES += jam.c jamParse.c jamHttp.c jamStorage.c
   j = _j
else

endif

ifeq ($(ROMIZING), false) 
   ROMFLAGS = -DROMIZING=0
else
   SRCFILES += ROMjavaWin.c
endif

all: kvm$(j)$(g).exe


OBJFILES = $(patsubst %.c,obj$(j)$(g)/%.o,$(SRCFILES))

vpath %.c  $(TOP)/kvm/VmCommon/src/ $(TOP)/kvm/VmWin/src/ \
	   $(TOP)/kvm/VmExtra/src/  $(TOP)/tools/jcc/ \
	   $(TOP)/jam/src/

ifeq ($(GCC), true)
   CC = gcc
   LD = gcc
   LIBS= -luser32 -lgdi32 -lkernel32 -lmsvcrt -lwsock32
   CPPFLAGS = -DWIN32 -DGCC -I$(TOP)/kvm/VmCommon/h -I$(TOP)/kvm/VmWin/h \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/jam/h -I$(TOP)/kvm/VmCommon/src
   CFLAGS = -mno-cygwin -mcpu=i386 -Wall $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
   DEBUG_FLAG = -g
   OPTIMIZE_FLAG = -O2
   OUTPUT = -o 
   LINKER_OUTPUT = -o
   LDFLAGS = -mno-cygwin
else
   CC= cl
   LD= link
   LIBS= user32.lib gdi32.lib kernel32.lib wsock32.lib
   CPPFLAGS = -DWIN32 -I$(TOP)/kvm/VmCommon/h -I$(TOP)/kvm/VmWin/h \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/jam/h /MT
   CFLAGS= -nologo $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
   DEBUG_FLAG= -Zi -Od
   OPTIMIZE_FLAG= -Ox
   OUTPUT = -Fo
   LINKER_OUTPUT = -out:
   LDFLAGS = 
ifeq ($(DEBUG), true)
   LDFLAGS += -debug
endif

endif

DEBUG_FLAG += -DINCLUDEDEBUGCODE=1 

$(TOP)/tools/jcc/ROMjavaWin.c $(TOP)/tools/jcc/nativeFunctionTableWin.c: jccWin

.PHONY:  jccWin
jccWin: 
	@(cd $(TOP)/tools/jcc; $(MAKE) windows)


kvm$(j)$(g).exe: obj$j$g/  $(OBJFILES)
	@echo "Linking ... $@"
	@$(LD) $(LDFLAGS) $(OBJFILES) $(LINKER_OUTPUT)$@ $(LIBS)

clean: 
	rm -rf kvm* obj*

$(OBJFILES): $(TOP)/kvm/VmWin/h/machine_md.h

obj$(j)$g/execute.o : execute.c bytecodes.c 

obj$(j)/%.o: %.c
		@echo "... $@"
		@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -c $(OUTPUT)$@ $<

obj$(j)_g/%.o: %.c
		@echo "... $@"
		@$(CC) $(CFLAGS) $(DEBUG_FLAG) -c $(OUTPUT)$@ $<

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
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E $(OUTPUT)$@ $<

else 

%.i: %.c .FORCE
	echo $(CC)
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E $< > $@

endif

%.s: %.c .FORCE
	@echo "... $@"
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -S $(OUTPUT)$@ $<

.FORCE:
