TOP=../..
KVM_ROOT=../../..

BUILDER = cmdide
IDEARGS = /b /c /q /v y

WRAPPER= $(shell if [ -f Wrapper.mcp ];then; echo true ;fi)

MCPFILES =  $(shell find . -name '*.mcp' | grep -v SCCS | grep -v KVM_ROM )

ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

MCPFILES+=KVM_ROM$(g).mcp

PRCFILES = $(subst mcp,prc,$(MCPFILES))



ifeq ($(WRAPPER),true)
all: wrapper_image $(PRCFILES)
else
all: $(PRCFILES)
endif

# $< is dependency
# $@ is target

%.prc : %.mcp
	@$(BUILDER) $(IDEARGS) $< 2>&1 || exit 1
	@sleep 5

wrapper_image: Wrapper.prc
	@cp $(KVM_ROOT)/kvm/VmPilot/build/Wrapper.prc $(KVM_ROOT)/tools/palm/classes/palm/database
	@rm -f Empty.prc
	@java -classpath $(KVM_ROOT)/tools/palm/classes \
	    palm.database.MakePalmApp -empty -icon \
	    $(KVM_ROOT)/samples/icons/default.bmp  -outfile Empty.prc
	@java -classpath $(KVM_ROOT)/tools/palm/classes \
	    palm.database.PRC2CHeader \
	    Empty.prc > $(KVM_ROOT)/kvm/VmPilot/jam/h/wrapper.h
	@rm -f Empty.prc

clean:
	rm -rf *.[oa] obj/* .filelist .unzippedclasses
	rm -f KVM *.prc *.prc.* *.class *.pdb bin.res *.i *.s classes.zip
	find . -name "*~" | xargs rm -f
	rm -rf Resource.frk/*.tmp  *.tmp *.tmp.*
	rm -rf "KVM_ROM_Data" "KVM_ROM_g_Data" "KVMutil_Data" "Jam_Data" "Wrapper_Data"
	rm -rf classes obj bin
	rm -f ../jam/h/wrapper.h


.PHONY: clean veryclean
