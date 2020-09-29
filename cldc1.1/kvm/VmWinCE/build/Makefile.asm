#
# Makefile for GNU make, adapted from the solaris version
# 23:18:39 05/16/02 @(#)Makefile.asm	1.2
#
TOP=../../..
include $(TOP)/build/Makefile.inc

default: asm_build

include $(TOP)/kvm/VmWinCE/build/Makefile

ASM = armasm

SRCFILES  +=  FastInterpretAux.c 

OBJFILES += obj$(j)$(g)/FastInterpret9.o obj$(j)$(g)/FastInterpretAux.o

vpath % $(TOP)/kvm/VmExtra/src/sarm
 
CFLAGS+=-DALTERNATIVE_FAST_INTERPRETER=1

asm_build: kvm_asm$(j)$(g).exe

kvm_asm$(j)$(g).exe: obj$j$g/  fp_obj$j$g/ $(OBJFILES) $(FP_OBJFILES) $(RCFILES)
	@echo "Linking ... $@"
	@$(LD) $(OBJFILES) $(FP_OBJFILES) $(RCFILES) $(LINKER_OUTPUT)$@ \
	       $(LIBS) $(LDFLAGS)

## Template
#obj$j$g/FastInterpret7.o: FastInterpret.S offsets.h
#	@echo "... $@"
#	$(CC) -DARM7=1 $(CPPFLAGS) -c -I . -o $@ $<

obj$j$g/FastInterpret9.o: FastInterpret9.masm
	@echo "... $@"
	$(ASM) -o $@ $<

#How to generate MASM file 
FastInterpret7.masm: FastInterpret.S offsets.h .FORCE
	@echo $(patsubst FastInterpret%.masm,-DARM%,$@)
	@echo "... $@"
	@$(CC) -DMASM $(patsubst FastInterpret%.masm,-DARM%,$@) -E -I ../../VmCommon/h -I . $< \
        | perl  -e '$$_ = ";". join(";", <STDIN>);'                      \
                -e 's/(\D):\//\1\//g;         #no : after drive letter'  \
                -e 's/:/;/g; s/\n//g;'                                   \
	        -e '# Make the actual changes that are required'         \
                -e 's/\.word/DCD/g;'                                     \
                -e 's/\.ascii/DCB/g;'                                    \
                -e 's/\.asciz([^;]*);/DCB \1;\t.byte 0;/g;'              \
                -e 's/\.byte/DCB/g;'                                     \
	        -e 's/\.align/align/g;'                                  \
	        -e 's/\.extern/IMPORT/g;'                                \
                -e 's/\$$/#/g;                #Constants use hash sign'  \
                -e 's/([1-9])([bf])/%\2\1/g;  #Format of local labels'   \
                -e 's/adrl(\w\w)/adr\1l/g;    #adrlxx -> adrxxl'         \
	        -e 's/;(#[^;]*;)+/;/g;        #preprocessor junk'        \
                -e '# These are purely for beautifying the code'         \
	        -e 's/;(\s*;)+/;/g;           #remove blank lines'       \
	        -e 's/\s*,\s*/, /g;           #spacing around commas'    \
		-e 's/;\s+(\w+)\s+/;\t\1\t/g; #tabs before, after opcode'\
                -e 's/;\s+(\d+);/;\1;/g;      #no space before tmp labels'\
		-e 's/\s+([!\}\]])/\1/g;      #no space before !}]'      \
		-e 's/([!\{\[#])\s+/\1/g;     #no space after !{[#'      \
                -e '# This is required again.  Make ; into newlines'     \
	        -e 's/;/\n/g; print'                                     \
                > $@	

offsets.h:     makeOffsets.exe
	./makeOffsets.exe > offsets.h

makeOffsets.exe:	makeOffsets.c
	$(CC) $(CPPFLAGS) -o $@ $<

