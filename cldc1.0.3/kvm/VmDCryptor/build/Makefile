TOP=../../..
include $(TOP)/build/Makefile.inc

# PLATFORM must be set correctly!
ifneq ($(PLATFORM), dcrypt)
  $(error PLATFORM must be set to 'dcrypt')
endif

# DCRYPT_HOME must be set correctly!
ifndef DCRYPT_HOME
  $(error DCRYPT_HOME must be set)
endif

export GCC=true

ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

ifeq ($(SIMULATOR), true)
   a =
   s = _s
else
   a = .a
   s =
endif
j =


ifeq ($(DEBUG_COLLECTOR), true)
COLLECTOR = collectorDebug.c
else
COLLECTOR = collector.c
endif

# Platform independent files first (VmCommon/src)
SRCFILES = \
  cache.c               \
  class.c               \
  fields.c              \
  frame.c               \
  garbage.c             \
  $(COLLECTOR)          \
  global.c              \
  interpret.c           \
  execute.c             \
  loader.c              \
  native.c              \
  property.c            \
  thread.c              \
  nativeCore.c          \
  hashtable.c           \
  verifier.c            \
  log.c                 \
  inflate.c             \
  stackmap.c            \
  profiling.c           \
  pool.c                \
  StartJVM.c            \
  events.c

# Platform independent extra files (kvm/VmExtra/src)
SRCFILES += \
  jar.c \
  resource.c \
  commProtocol.c

# Platform dependent files (kvm/VmDCryptor/src, tools/jcc)
SRCFILES += \
  main.c                \
  runtime_md.c					\
  loader_md.c						\
	nativeFunctionTableDCryptor.c \
	commProtocol_md.c     \
  nativeDCryptor.c
#  datagramProtocol_md.c \
#  socketProtocol_md.c   \

OTHER_FLAGS += -DENABLE_JAVA_DEBUGGER=0
OTHER_FLAGS += -DJAR_FILES_USE_STDIO=0

# Not sure if we want this but keep it for now. Will probably change when we
# try to make the interpreter loop back into one big switch statement with
# the cases ordered by bytecode priority
OTHER_FLAGS +=-DPADTABLE=1


ifeq ($(ROMIZING), false) 
   ROMFLAGS = -DROMIZING=0
else
   SRCFILES += ROMjavaDCryptor.c
endif

ifeq ($(SIMULATOR),true)
all: kvm$(g)
else
all: kvm$(g).bin
endif


OBJFILES = $(patsubst %.c,obj$(g)$(s)/%.o,$(SRCFILES))

vpath %.c  $(TOP)/kvm/VmCommon/src/ \
           $(TOP)/kvm/VmDCryptor/src/ \
           $(TOP)/kvm/VmExtra/src/ \
           $(TOP)/tools/jcc/

LIBS =
CPPFLAGS =  \
	   -I$(TOP)/kvm/VmCommon/h \
		 -I$(TOP)/kvm/VmDCryptor/h \
		 -I$(TOP)/kvm/VmExtra/h

ifeq ($(SIMULATOR),true)
		CPPFLAGS += -I$(TOP)/kvm/VmDCryptor/simulator/ \
					-DUNIX -DLINUX -D$(ARCH) -DSIMULATOR
else
		CPPFLAGS += -I$(DCRYPT_HOME)/include
endif


ifeq ($(SIMULATOR),true)
CC = gcc
CFLAGS =  -Wall $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS) -DSIMULATOR
DEBUG_FLAG = -g
OPTIMIZE_FLAG = -O2
else
CC = $(DCRYPT_HOME)/bin/d_gcc -O -fomit-frame-pointer -mcpu=strongarm1100
LD = $(DCRYPT_HOME)/bin/d_ld
CFLAGS =  -Wall $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
DEBUG_FLAG = #-g   this flag conflicts with the -fomit-frame-pointer flag
OPTIMIZE_FLAG = -O2
endif

ifeq ($(SVM), true)
   OTHER_FLAGS += -DSVM=1
   ifeq ($(EXCESSIVE_GARBAGE_COLLECTION), true)
      OTHER_FLAGS += -DEXCESSIVE_GARBAGE_COLLECTION=1
   endif
   SRCFILES += crypto.c crypto_provider_MD5RSABasic.c cbs.c
endif

ifeq ($(DEBUG),true)
DEBUG_FLAG += -DINCLUDEDEBUGCODE=1 
endif

$(TOP)/tools/jcc/ROMjavaDCryptor.c $(TOP)/tools/jcc/nativeFunctionTableDCryptor.c: jccDCryptor

.PHONY:  jccDCryptor
jccDCryptor: 
	@(cd $(TOP)/tools/jcc; $(MAKE) dcrypt)


# The spec file expects kvm.a
kvm$(g).bin: build.spec kvm$(g).a
	@echo "Building $@ ..."
	@if [ "kvm$(g).a" != "kvm.a" ]; then \
		cp -f kvm$(g).a kvm.a; \
	fi
	$(DCRYPT_HOME)/bin/build -v build.spec -o $@ -l build.log
	
ifeq ($(SIMULATOR),true)
kvm$(g)$(a): obj$(g)$(s)/ $(OBJFILES)
		@echo "Linking ... $@"
		$(CC) $(OBJFILES) -o $@ $(LIBS) -lncurses
else
kvm$(g)$(a): obj$(g)/ $(OBJFILES)
		@echo "Linking ... $@"
		$(LD) -r $(OBJFILES) -o $@
endif

clean: 
	rm -rf core kvm* ./obj* ./SunWS_cache .sb
	rm -rf $(TOP)/tools/jcc/ROMjavaDCryptor.c $(TOP)/tools/jcc/nativeFunctionTableDCryptor.c

obj$(g)/execute.o : execute.c bytecodes.c 

obj$(s)/%.o: %.c
		@echo "... $@"
		@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(OPTIMIZE_FLAG) -c -o $@ $<

obj_g$(s)/%.o: %.c
		@echo "... $@"
		@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(DEBUG_FLAG) -c -o $@ $<

obj$(g)$(s)/:
		mkdir -p $@

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E -o $@ $<


%.s: %.c .FORCE
	@echo "... $@"
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -S -o $@ $<


regression:
	CLASSPATH=../../../api/classes:../../../samples/classes kvm tests.RegressionTest

.FORCE:
