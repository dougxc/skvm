BUILDDIR=$(shell pwd)
TOP=../..
PLATFORM=palm
export PLATFORM

SUBDIRS = \
  $(TOP)/tools/preverifier/build/$(PLATFORM) \
  $(TOP)/api \
  $(TOP)/tools/palm \
  $(TOP)/samples \
  $(TOP)/samples/jam

ifeq ($(DEBUG), true) 
   SUBDIRS += $(TOP)/tools/kdp
endif

ifneq ($(ROMIZING), false) 
   SUBDIRS += $(TOP)/tools/jcc
endif

SUBDIRS += $(TOP)/kvm/VmPilot/build


all clean : FORCE
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done

FORCE: ;
