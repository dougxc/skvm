#
# Build specification for D'Cryptor KVM
#
# Copyright 2000 D'Crypt Pte Ltd
# All rights reserved.
#
# $Id: build.spec,v 1.1.1.1 2002/05/21 20:38:47 dougxc Exp $
#
# $Log: build.spec,v $
# Revision 1.1.1.1  2002/05/21 20:38:47  dougxc
# All the Secure KVM project stuff under one top level CVS module
#
# Revision 1.1.1.1  2002/02/26 00:18:30  dsimon
# initial import of SVM to CVS
#
#
# This spec will need to be modified to factoring functionality in separate
# modules for security reasons by someone who understands how to do this
# factoring appropriately.
#

build KVM
  banner
  "D'Cryptor KVM\r\n\n" ;

  entry module = main label = start ;

  #
  # This is the PHYSICAL address of the Flash ROM on the DCryptor and
  # differs from the VIRTUAL address mentioned in flash.h.
  #
  const FLASH_REAL_BASE        = 0x00000000 ;
  #
  # These value *must* be kept in sync with those of the same name defined
  # in VmDCryptor/h/machine_md.h
  #
  const FLASH_BLOCK_COUNT = 32 ;
  const FLASH_BLOCK_SIZE  = 128K;
  const JAR_FLASH_BLOCK   = 6 ;
  const MAX_JAR_SIZE      = (FLASH_BLOCK_COUNT - JAR_FLASH_BLOCK) *
                             FLASH_BLOCK_SIZE;

  module system
    library = fx.o ;
    library = flash.o ;
    entry label = fx_load access = main ;
    entry label = flash_read access = main ;
  end module

  module gpio
    library = gpio.o ;
    stack   = 0x4000 ;
    entry label=gpio_set_output access=rtc,noisesrc,main ;
    entry label=gpio_set_input access=main ;
    entry label=gpio_set_high access=rtc,noisesrc,main ;
    entry label=gpio_set_low access=main ;
    entry label=gpio_get_lvl access=main ;
    map gpio at 0x90040000 size 4K ;
  end module
    
  module noisesrc
    library = noisesrc.a ;
    library = libc.a ;
    library = libgcc.a ;
    library = uart.o ;
    stack  = 0x4000 ;
    entry label = ns_init access = main ;
    entry label = ns_get_byte access = main ;
    map mcp at 0x80060000 size 4K, at 0x90060000 size 4K ;
  end module

  module rtc
    library = rtc.a ;
    library = libgcc.a ;
    stack  = 0x4000 ;
    entry label=rtc_get_time access=main ;
    entry label=rtc_init access=main ;
    entry label=rtc_start_oscr access=main ;
    entry label=rtc_stop_oscr access=main ;
    map mcp at 0x80060000 size 4K, at 0x90060000 size 4K ;
  end module

  module timer
    library = timer.o ;
    library = libgcc.a ;
    stack   = 0x4000 ;
    entry label=timer_delay_ms access=noisesrc,main ;
    entry label=timer_init access=noisesrc,main ;
    entry label=timer_timeout access=noisesrc,main ;
    entry label=ostimer access=main ;
    map ostimer at 0x90000000 size 4K ;
  end module

  module main
    object  = kvm.a ;
    library = uart.o ;
    library = uart1.o ;
    library = uart5.o ;
    library = audio.a ;
    library = melp.a ;
    library = swi.o ;
    library = libc.a ;
    library = libgcc.a ;
    stack  = 0x8000 ;
    #
    # Map an array to the flash block(s) containing the JAR. Note that this
    # covers the complete range of blocks which may contain the JAR, even
    # if the JAR does not actually fill this range.
    #
    map JarFlashBlock at FLASH_REAL_BASE + (FLASH_BLOCK_SIZE * JAR_FLASH_BLOCK)
                      size MAX_JAR_SIZE ;

    # Needed by runtime_md.c 
    map ostimer at 0x90000000 size 4K ;

    # Needed by uart1.o
    interrupt 15 label=uart1_isr user=0 ;
    map uart1 at 0x80010000 size 4K ;
    map sdlc  at 0x80020000 size 4K ;
    map gpio  at 0x90040000 size 4K ;

    # Needed by audio.a
    interrupt 22 label=audio_tx_isr user=0 ;
    interrupt 23 label=audio_rx_isr user=1 ;
    map mcp at 0x80060000 size 4K, at 0x90060000 size 4K ;
    map dma at 0xb0000000 size 4K ;

    # Needed by uart5.o
    map uart5 at 0x18000000 size 64K ;

  end module

  shared rtc
    struct  = rtc_time_shared ;
    file    = rtc.h ;
    access  = main ;
  end shared

  shared audio_shared
    struct = audio_shared ;
    file   = audio.h ;
    access = main ;
  end shared

  shared audio_shared_samples
    struct = audio_shared_samples ;
    file   = audio.h ;
    access = main ;
  end shared

end build
