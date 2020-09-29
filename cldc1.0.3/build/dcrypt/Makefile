BUILDDIR=$(shell pwd)
TOP=../..
PLATFORM=dcrypt
export PLATFORM=dcrypt

ifndef DCRYPT_HOME
  DCRYPT_HOME=/usr/local/dcrypt
  export DCRYPT_HOME
endif

export ROMIZING=true

SUBDIRS = \
  $(TOP)/tools/preverifier/build/dcrypt \
  $(TOP)/api
#  $(TOP)/samples \
#  $(TOP)/samples/jam

ifeq ($(DEBUG), true) 
   SUBDIRS += $(TOP)/tools/kdp
endif

ifneq ($(ROMIZING), false) 
   SUBDIRS += $(TOP)/tools/jcc
endif

SUBDIRS += $(TOP)/kvm/VmDCryptor/build

all clean : FORCE
	@echo "********** KVM_FLAGS = $(KVM_FLAGS) **********"
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done

FORCE: 
