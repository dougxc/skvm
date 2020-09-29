TOP=../..
include $(TOP)/build/Makefile.inc

JAVA          = java
JAVAC         = javac
JAR           = jar

JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS)
 
CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target


$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists: 
	@rm -f .filelist 

compilefiles:
	@if [ '!' -d classes ]; then rm -rf classes; mkdir classes; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) `cat .filelist`; \
		$(JAVAC) -d classes -classpath classes:src \
	              `cat .filelist`; \
	fi

ifeq ($(PLATFORM), solaris)
all:    unix
endif

ifeq ($(PLATFORM), linux)
all:    unix
endif

ifeq ($(PLATFORM), win32)
all:    windows
endif

ifeq ($(PLATFORM), palm)
ifeq ($(GCC), true)
   CC = gcc
   CFLAGS = -DWIN32 -DGCC -mno-cygwin -mcpu=i386
   OPTIMIZE_FLAG = -O2
else
   CC= cl
   CFLAGS = -DWIN32
   OPTIMIZE_FLAG= -Os -O1
endif
all:    palm
endif


ifeq ($(PLATFORM),dcrypt)
all:  dcrypt
endif

unix:	 nativeFunctionTableUnix.c ROMjavaUnix.c

windows: nativeFunctionTableWin.c ROMjavaWin.c

palm:    nativeFunctionTablePalm.c nativeRelocationPalm.c  binfiles

dcrypt:  nativeFunctionTableDCryptor.c ROMjavaDCryptor.c

tools: eraselists $(CLASSFILES) compilefiles 

ifneq ($(JCC_PASS_TWO), true)

ROMjava%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

nativeFunctionTable%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

nativeRelocation%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

#Classes that aren't currently used for a particular platform
NON_Unix_CLASSES =  'com/sun/cldc/io/palm/http/' \
                    'com/sun/cldc/io/palm/https/' \
                    'com/sun/cldc/io/palm/comm/'  \
                    'com/dcrypt/'

NON_Win_CLASSES =   'com/sun/cldc/io/palm/http/' \
                    'com/sun/cldc/io/palm/https/' \
                    'com/sun/cldc/io/palm/comm/'  \
                    'com/dcrypt/'

NON_Palm_CLASSES =  'com/sun/cldc/io/j2me/comm/'  \
                    'com/dcrypt/'

# The DCryptor does not (yet) support any of the network connection
# protocols
NON_DCryptor_CLASSES =  'com/sun/cldc/io/palm/http/' \
                    'com/sun/cldc/io/palm/https/' \
                    'com/sun/cldc/io/palm/comm/' \
										'com/sun/kjava/' \
										'com/sun/cldc/io/j2me/http' \
										'com/sun/cldc/io/j2me/serversocket' \
										'com/sun/cldc/io/j2me/datagram' \
										'com/sun/cldc/io/j2me/events' \
										'com/sun/cldc/io/j2me/socket' \
										'com/sun/cldc/io/NetworkConnectionBase.class'

SVM_ONLY_CLASSES = \
    'javax/microedition/cbs'

classes%.zip: classes.zip
	@echo ... $@
	@rm -rf tmpjar
	@mkdir tmpjar
	@cp classes.zip $@
	@echo $($(patsubst classes%.zip,NON_%_CLASSES,$@))		
	@(cd tmpjar; $(JAR) xf ../$@; \
	 rm -rf $($(patsubst classes%.zip,NON_%_CLASSES,$@)); \
	 if [ "$(SVM)" != "true" ]; then \
		echo $(SVM_ONLY_CLASSES); \
   	rm -rf $(SVM_ONLY_CLASSES); \
	 fi; \
	 chmod -R a+wrx .)
	@rm $@
	@$(JAR) cfM0 $@ -C tmpjar .
	@rm -rf tmpjar

classes.zip: $(TOP)/api/classes.zip
	@echo ... $@
	@cmp -s $< $@ || (echo cp $< $@ ; cp $< $@)


$(TOP)/api/classes.zip: .FORCE
	@(cd $(TOP)/api; $(MAKE) classes.zip)

else

# One of the possible architectures when creating ROMjava*.c
UnixArch = KVM
WinArch =  KVM
PalmArch = Palm
DCryptorArch = KVM

UnixFlags = 
WinFlags =  
PalmFlags = -nq



ROMjava%.c: classes%.zip 
	@echo ... $@
	$(JAVA) -classpath classes JavaCodeCompact \
 	          $($(patsubst classes%.zip,%Flags,$<)) \
	         -arch $($(patsubst classes%.zip,%Arch,$<)) -o $@ $<

nativeFunctionTable%.c: classes%.zip
	@echo ... $@
	@cp -f src/*.properties classes
	$(JAVA) -classpath classes JavaCodeCompact \
 	          $($(patsubst classes%.zip,%Flags,$<)) \
		 -arch KVM_Native -o $@ $<

nativeRelocation%.c: classes%.zip
	@echo ... $@
	$(JAVA) -classpath classes JavaCodeCompact \
	         -imageAttribute relocating \
 	          $($(patsubst classes%.zip,%Flags,$<)) \
	         -arch $($(patsubst classes%.zip,%Arch,$<)) -o $@ $<

endif



ROMjavaPalm: ROMjavaPalm.c src/runtime/PalmMain.c
	@echo ... $@
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) \
            -I. -I$(TOP)/kvm/VmPilot/h -I$(TOP)/kvm/VmCommon/h \
            -I$(TOP)/kvm/VmExtra/h -DROMIZING ROMjavaPalm.c -o ROMjavaPalm

binfiles: ROMjavaPalm
	@echo ...Creating Palm binary files
	@rm -rf $(TOP)/kvm/VmPilot/build/bin
	@mkdir -p $(TOP)/kvm/VmPilot/build/bin
	@./ROMjavaPalm $(TOP)/kvm/VmPilot/build/bin/PalmROM


clean:
	@rm -rf .filelist
	@rm -rf classes tmpjar
	@rm -rf *.zip
	@rm -rf ROMjava*
	@rm -rf nativeFunctionTable*
	@rm -rf nativeRelocation*
	@rm -rf *~ */*~ */*/*~
	@rm -rf *# */*# */*/*#


# I have no idea why this is necessary, but otherwise gnumake deletes
# these as intermediate files
.SECONDARY:	classesUnix.zip classesPalm.zip classesWin.zip classesDCryptor.zip

.FORCE:
