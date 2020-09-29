/*
 * Copyright (c) 1998-2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Bytecode interpreter
 * FILE:      interpret.c
 * OVERVIEW:  This file defines the general routines used by the
 *            Java bytecode interpreter.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Major reorganization by Nik Shaylor 9/5/2000
 * NOTE:      In KVM 1.0.2, the interpreter has been restructured for
 *            better performance, but without sacrificing portability.
 *            The high-level interpreter loop is now defined in file
 *            execute.c.  Actual bytecode definitions are in file
 *            bytecodes.c.  Various high-level compilation flags
 *            for the interpreter have been documented in main.h 
 *            and in the KVM Porting Guide.
 *=======================================================================*/

/*=========================================================================
 * Local include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Virtual machine global registers (see description in Interpreter.h)
 *=======================================================================*/

BYTE*         ip_global; /*  Instruction pointer (program counter) */
FRAME         fp_global; /*  Current frame pointer */
cell*         sp_global; /*  Execution stack pointer */
cell*         lp_global; /*  Local variable pointer */
CONSTANTPOOL  cp_global; /*  Constant pool pointer */

#define ip ip_global
#define fp fp_global
#define sp sp_global
#define lp lp_global
#define cp cp_global

/*=========================================================================
 * Bytecode table for debugging purposes (included
 * only if certain tracing modes are enabled).
 *=======================================================================*/

#if INCLUDEDEBUGCODE
const char* const byteCodeNames[] = {
    "NOP",              /*  0x00 */
    "ACONST_NULL",      /*  0x01 */
    "ICONST_M1",        /*  0x02 */
    "ICONST_0",         /*  0x03 */
    "ICONST_1",         /*  0x04 */
    "ICONST_2",         /*  0x05 */
    "ICONST_3",         /*  0x06 */
    "ICONST_4",         /*  0x07 */
    "ICONST_5",         /*  0x08 */
    "LCONST_0",         /*  0x09 */
    "LCONST_1",         /*  0x0A */
    "FCONST_0",         /*  0x0B */
    "FCONST_1",         /*  0x0C */
    "FCONST_2",         /*  0x0D */
    "DCONST_0",         /*  0x0E */
    "DCONST_1",         /*  0x0F */
    "BIPUSH",           /*  0x10 */
    "SIPUSH",           /*  0x11 */
    "LDC",              /*  0x12 */
    "LDC_W",            /*  0x13 */
    "LDC2_W",           /*  0x14 */
    "ILOAD",            /*  0x15 */
    "LLOAD",            /*  0x16 */
    "FLOAD",            /*  0x17 */
    "DLOAD",            /*  0x18 */
    "ALOAD",            /*  0x19 */
    "ILOAD_0",          /*  0x1A */
    "ILOAD_1",          /*  0x1B */
    "ILOAD_2",          /*  0x1C */
    "ILOAD_3",          /*  0x1D */
    "LLOAD_0",          /*  0x1E */
    "LLOAD_1",          /*  0x1F */
    "LLOAD_2",          /*  0x20 */
    "LLOAD_3",          /*  0x21 */
    "FLOAD_0",          /*  0x22 */
    "FLOAD_1",          /*  0x23 */
    "FLOAD_2",          /*  0x24 */
    "FLOAD_3",          /*  0x25 */
    "DLOAD_0",          /*  0x26 */
    "DLOAD_1",          /*  0x27 */
    "DLOAD_2",          /*  0x28 */
    "DLOAD_3",          /*  0x29 */
    "ALOAD_0",          /*  0x2A */
    "ALOAD_1",          /*  0x2B */
    "ALOAD_2",          /*  0x2C */
    "ALOAD_3",          /*  0x2D */
    "IALOAD",           /*  0x2E */
    "LALOAD",           /*  0x2F */
    "FALOAD",           /*  0x30 */
    "DALOAD",           /*  0x31 */
    "AALOAD",           /*  0x32 */
    "BALOAD",           /*  0x33 */
    "CALOAD",           /*  0x34 */
    "SALOAD",           /*  0x35 */
    "ISTORE",           /*  0x36 */
    "LSTORE",           /*  0x37 */
    "FSTORE",           /*  0x38 */
    "DSTORE",           /*  0x39 */
    "ASTORE",           /*  0x3A */
    "ISTORE_0",         /*  0x3B */
    "ISTORE_1",         /*  0x3C */
    "ISTORE_2",         /*  0x3D */
    "ISTORE_3",         /*  0x3E */
    "LSTORE_0",         /*  0x3F */
    "LSTORE_1",         /*  0x40 */
    "LSTORE_2",         /*  0x41 */
    "LSTORE_3",         /*  0x42 */
    "FSTORE_0",         /*  0x43 */
    "FSTORE_1",         /*  0x44 */
    "FSTORE_2",         /*  0x45 */
    "FSTORE_3",         /*  0x46 */
    "DSTORE_0",         /*  0x47 */
    "DSTORE_1",         /*  0x48 */
    "DSTORE_2",         /*  0x49 */
    "DSTORE_3",         /*  0x4A */
    "ASTORE_0",         /*  0x4B */
    "ASTORE_1",         /*  0x4C */
    "ASTORE_2",         /*  0x4D */
    "ASTORE_3",         /*  0x4E */
    "IASTORE",          /*  0x4F */
    "LASTORE",          /*  0x50 */
    "FASTORE",          /*  0x51 */
    "DASTORE",          /*  0x52 */
    "AASTORE",          /*  0x53 */
    "BASTORE",          /*  0x54 */
    "CASTORE",          /*  0x55 */
    "SASTORE",          /*  0x56 */
    "POP",              /*  0x57 */
    "POP2",             /*  0x58 */
    "DUP",              /*  0x59 */
    "DUP_X1",           /*  0x5A */
    "DUP_X2",           /*  0x5B */
    "DUP2",             /*  0x5C */
    "DUP2_X1",          /*  0x5D */
    "DUP2_X2",          /*  0x5E */
    "SWAP",             /*  0x5F */
    "IADD",             /*  0x60 */
    "LADD",             /*  0x61 */
    "FADD",             /*  0x62 */
    "DADD",             /*  0x63 */
    "ISUB",             /*  0x64 */
    "LSUB",             /*  0x65 */
    "FSUB",             /*  0x66 */
    "DSUB",             /*  0x67 */
    "IMUL",             /*  0x68 */
    "LMUL",             /*  0x69 */
    "FMUL",             /*  0x6A */
    "DMUL",             /*  0x6B */
    "IDIV",             /*  0x6C */
    "LDIV",             /*  0x6D */
    "FDIV",             /*  0x6E */
    "DDIV",             /*  0x6F */
    "IREM",             /*  0x70 */
    "LREM",             /*  0x71 */
    "FREM",             /*  0x72 */
    "DREM",             /*  0x73 */
    "INEG",             /*  0x74 */
    "LNEG",             /*  0x75 */
    "FNEG",             /*  0x76 */
    "DNEG",             /*  0x77 */
    "ISHL",             /*  0x78 */
    "LSHL",             /*  0x79 */
    "ISHR",             /*  0x7A */
    "LSHR",             /*  0x7B */
    "IUSHR",            /*  0x7C */
    "LUSHR",            /*  0x7D */
    "IAND",             /*  0x7E */
    "LAND",             /*  0x7F */
    "IOR",              /*  0x80 */
    "LOR",              /*  0x81 */
    "IXOR",             /*  0x82 */
    "LXOR",             /*  0x83 */
    "IINC",             /*  0x84 */
    "I2L",              /*  0x85 */
    "I2F",              /*  0x86 */
    "I2D",              /*  0x87 */
    "L2I",              /*  0x88 */
    "L2F",              /*  0x89 */
    "L2D",              /*  0x8A */
    "F2I",              /*  0x8B */
    "F2L",              /*  0x8C */
    "F2D",              /*  0x8D */
    "D2I",              /*  0x8E */
    "D2L",              /*  0x8F */
    "D2F",              /*  0x90 */
    "I2B",              /*  0x91 */
    "I2C",              /*  0x92 */
    "I2S",              /*  0x93 */
    "LCMP",             /*  0x94 */
    "FCMPL",            /*  0x95 */
    "FCMPG",            /*  0x96 */
    "DCMPL",            /*  0x97 */
    "DCMPG",            /*  0x98 */
    "IFEQ",             /*  0x99 */
    "IFNE",             /*  0x9A */
    "IFLT",             /*  0x9B */
    "IFGE",             /*  0x9C */
    "IFGT",             /*  0x9D */
    "IFLE",             /*  0x9E */
    "IF_ICMPEQ",        /*  0x9F */
    "IF_ICMPNE",        /*  0xA0 */
    "IF_ICMPLT",        /*  0xA1 */
    "IF_ICMPGE",        /*  0xA2 */
    "IF_ICMPGT",        /*  0xA3 */
    "IF_ICMPLE",        /*  0xA4 */
    "IF_ACMPEQ",        /*  0xA5 */
    "IF_ACMPNE",        /*  0xA6 */
    "GOTO",             /*  0xA7 */
    "JSR",              /*  0xA8 */
    "RET",              /*  0xA9 */
    "TABLESWITCH",      /*  0xAA */
    "LOOKUPSWITCH",     /*  0xAB */
    "IRETURN",          /*  0xAC */
    "LRETURN",          /*  0xAD */
    "FRETURN",          /*  0xAE */
    "DRETURN",          /*  0xAF */
    "ARETURN",          /*  0xB0 */
    "RETURN",           /*  0xB1 */
    "GETSTATIC",        /*  0xB2 */
    "PUTSTATIC",        /*  0xB3 */
    "GETFIELD",         /*  0xB4 */
    "PUTFIELD",         /*  0xB5 */
    "INVOKEVIRTUAL",    /*  0xB6 */
    "INVOKESPECIAL",    /*  0xB7 */
    "INVOKESTATIC",     /*  0xB8 */
    "INVOKEINTERFACE",  /*  0xB9 */
    "UNUSED",           /*  0xBA */
    "NEW",              /*  0xBB */
    "NEWARRAY",         /*  0xBC */
    "ANEWARRAY",        /*  0xBD */
    "ARRAYLENGTH",      /*  0xBE */
    "ATHROW",           /*  0xBF */
    "CHECKCAST",        /*  0xC0 */
    "INSTANCEOF",       /*  0xC1 */
    "MONITORENTER",     /*  0xC2 */
    "MONITOREXIT",      /*  0xC3 */
    "WIDE",             /*  0xC4 */
    "MULTIANEWARRAY",   /*  0xC5 */
    "IFNULL",           /*  0xC6 */
    "IFNONNULL",        /*  0xC7 */
    "GOTO_W",           /*  0xC8 */
    "JSR_W",            /*  0xC9 */
    "BREAKPOINT",       /*  0xCA */

    /*  Fast bytecodes: */
    "GETFIELD_FAST",        /*  0xCB */
    "GETFIELDP_FAST",       /*  0xCC */
    "GETFIELD2_FAST",       /*  0xCD */
    "PUTFIELD_FAST",        /*  0xCE */
    "PUTFIELD2_FAST",       /*  0xCF */
    "GETSTATIC_FAST",       /*  0xD0 */
    "GETSTATICP_FAST",      /*  0xD1 */
    "GETSTATIC2_FAST",      /*  0xD2 */
    "PUTSTATIC_FAST",       /*  0xD3 */
    "PUTSTATIC2_FAST",      /*  0xD4 */
    "UNUSED_D5",            /*  0xD5 */
    "INVOKEVIRTUAL_FAST",   /*  0xD6 */
    "INVOKESPECIAL_FAST",   /*  0xD7 */
    "INVOKESTATIC_FAST",    /*  0xD8 */
    "INVOKEINTERFACE_FAST", /*  0xD9 */
    "NEW_FAST",             /*  0xDA */
    "ANEWARRAY_FAST",       /*  0xDB */
    "MULTIANEWARRAY_FAST",  /*  0xDC */
    "CHECKCAST_FAST",       /*  0xDD */
    "INSTANCEOF_FAST",      /*  0xDE */
    "CUSTOMCODE"            /*  0xDF */
};
#endif /* INCLUDEDEBUGCODE */


/*=========================================================================
 * Interpreter tracing & profiling functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getByteCodeName()
 * TYPE:          public debugging operation
 * OVERVIEW:      Given a bytecode value, get the mnemonic name of
 *                the bytecode.
 * INTERFACE:
 *   parameters:  bytecode as an integer
 *   returns:     constant string containing the name of the bytecode
 *=======================================================================*/

#if INCLUDEDEBUGCODE
static const char* 
getByteCodeName(ByteCode token) {
    if (token >= 0 && token <= LASTBYTECODE)
         return byteCodeNames[token];
    else return "<INVALID>";
}
#else
#  define getByteCodeName(token) ""
#endif

/*=========================================================================
 * FUNCTION:      printRegisterStatus()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print all the VM registers of the currently executing
 *                thread for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printRegisterStatus() {
    cell* i;

    fprintf(stdout,"VM status:\n");
    fprintf(stdout,"Instruction pointer.: %lx ", (long)ip);
    if (fp)
        fprintf(stdout, "(offset within invoking method: %d)",
                ip - fp->thisMethod->u.java.code);
    if (ip) {
        fprintf(stdout, "\nNext instruction....: 0x%x", *ip);
#if INCLUDEDEBUGCODE
        if (tracebytecodes) {
            fprintf(stdout, " (%s)", getByteCodeName(*ip));
        }
#endif        
    }

    fprintf(stdout,"\nFrame pointer.......: %lx\n", (long)fp);
    fprintf(stdout,"Local pointer.......: %lx\n", (long)lp);

    if (CurrentThread != NULL) {
        STACK stack;
        int size = 0;
        for (stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
            size += stack->size;
        }

        fprintf(stdout,"Stack size..........: %d; sp: %lx; ranges: ",
                size, (long)sp);
        for (stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
            fprintf(stdout, "%lx-%lx;", (long)stack->cells,
                    (long)(stack->cells + stack->size));
        }
        fprintf(stdout, "\n");
    }

    if (fp) {
        fprintf(stdout,"Contents of the current stack frame:\n");
        for (i = lp; i <= sp; i++) {
        fprintf(stdout,"    %lx: %lx", (long)i, (long)*i);
            if (i == lp) fprintf(stdout," (lp)");
            if (i == (cell*)fp) fprintf(stdout," (fp)");
            if (i == (cell*)fp + SIZEOF_FRAME - 1) {
                fprintf(stdout," (end of frame)");
            }
            if (i == sp) fprintf(stdout," (sp)");
            fprintf(stdout,"\n");
        }
    }
}
#endif /*  INCLUDEDEBUGCODE */


/*=========================================================================
 * FUNCTION:      printVMstatus()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print all the execution status of the VM
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printVMstatus() {
    printStackTrace();
    printRegisterStatus();
    printExecutionStack();
    printProfileInfo();
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      checkVMStatus()
 * TYPE:          public debugging operation (consistency checking)
 * OVERVIEW:      Ensure that VM registers are pointing to meaningful
 *                locations and that stacks have not over/underflown.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 * NOTE:          This operation is only called when profiling
 *                is enabled.
 *=======================================================================*/

#if INCLUDEDEBUGCODE && ENABLEPROFILING
static void checkVMstatus() {
    STACK stack;
    int valid;

    if (CurrentThread == NIL) {
        return;
    }

    /*  Catch stack over/underflows */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, sp)) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError(KVM_MSG_STACK_POINTER_CORRUPTED);
    }

    /*  Frame pointer should always be within stack range */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, (cell*)fp) && (cell*)fp <= sp) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError(KVM_MSG_FRAME_POINTER_CORRUPTED);
    }

    /*  Locals pointer should always be within stack range */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, lp) && lp <= sp) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError(KVM_MSG_LOCALS_POINTER_CORRUPTED);
    }

    /*  Check the number of active threads; this should always be > 1 */
    if (!areActiveThreads()) { 
        fatalVMError(KVM_MSG_ACTIVE_THREAD_COUNT_CORRUPTED);
   }
}
#else

#define checkVMstatus()

#endif /* INCLUDEDEBUGCODE && ENABLEPROFILING */

/*=========================================================================
 * FUNCTION:      fatalSlotError()
 * TYPE:          public debugging operation (consistency checking)
 * OVERVIEW:      Report missing field/method and exit
 * INTERFACE:
 *   parameters:  CONSTANTPOOL constantPool, int cpIndex
 *   returns:     <nothing>
 *   throws:      NoSuchMethodError or NoSuchFieldError
 *=======================================================================*/

void fatalSlotError(CONSTANTPOOL constantPool, int cpIndex) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag   = CONSTANTPOOL_TAG(constantPool, cpIndex);
    if (thisTag & CP_CACHEBIT) {
        if (thisTag == CONSTANT_Fieldref) {
            sprintf(str_buffer, KVM_MSG_NO_SUCH_FIELD_2STRPARAMS,
                    fieldName((FIELD)(thisEntry->cache)),
                    getFieldSignature((FIELD)thisEntry->cache));
        } else {
            sprintf(str_buffer, KVM_MSG_NO_SUCH_METHOD_2STRPARAMS,
                    methodName((METHOD)thisEntry->cache),
                    getMethodSignature((METHOD)thisEntry->cache));
        }
    } else {
        int nameTypeIndex       = thisEntry->method.nameTypeIndex;
        NameTypeKey nameTypeKey = 
            constantPool->entries[nameTypeIndex].nameTypeKey;
        NameKey nameKey         = nameTypeKey.nt.nameKey;
        MethodTypeKey typeKey   = nameTypeKey.nt.typeKey;

        if (thisTag == CONSTANT_Fieldref) {
            sprintf(str_buffer, KVM_MSG_NO_SUCH_FIELD_2STRPARAMS,
                    change_Key_to_Name(nameKey, NULL),
                    change_Key_to_FieldSignature(typeKey));
        } else {
            sprintf(str_buffer, KVM_MSG_NO_SUCH_METHOD_2STRPARAMS,
                    change_Key_to_Name(nameKey, NULL),
                    change_Key_to_MethodSignature(typeKey));
        }
    }
    if (thisTag == CONSTANT_Fieldref)
        raiseExceptionCharMsg(NoSuchFieldError,str_buffer);
    else
        raiseExceptionCharMsg(NoSuchMethodError,str_buffer);
}

/*=========================================================================
 * FUNCTION:      InstructionProfile()
 * OVERVIEW:      Profile an instruction
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if ENABLEPROFILING

void InstructionProfile() {

#if INCLUDEDEBUGCODE
    /*  Check that the VM is doing ok */
    checkVMstatus();
#endif /*INCLUDEDEBUGCODE*/

    /*  Increment the instruction counter */
    /*  for profiling purposes */
    InstructionCounter++;
}
#endif /* ENABLEPROFILING */

/*=========================================================================
 * FUNCTION:      InstructionTrace()
 * OVERVIEW:      Trace an instruction
 * INTERFACE:
 *   parameters:  instruction pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE

void InstructionTrace(BYTE *traceip) {
    int token = *traceip;

    ASSERTING_NO_ALLOCATION

        /* Print stack contents */
        {
            /* mySP points to just the first stack word. */
            cell *mySP = (cell *)((char *)fp + sizeof(struct frameStruct));
            fprintf(stdout, "       Operands (%ld items): ", 
                    (long)(sp - mySP) + 1);
            for ( ; mySP <= sp; mySP++) {
                fprintf(stdout, "%lx, ", *mySP);
            }
            fprintf(stdout, "\n");
        }

#if (TERSE_MESSAGES)
        fprintf(stdout, "    %ld: %s\n",
                (long)((traceip) - fp_global->thisMethod->u.java.code),
                getByteCodeName(token));
#else /* TERSE_MESSAGES */
        { 
            char buffer[256];
            getClassName_inBuffer(((CLASS)fp->thisMethod->ofClass), buffer);
            fprintf(stdout, "    %d: %s (in %s::%s)\n", 
                    (traceip) - fp->thisMethod->u.java.code,
                    getByteCodeName(token), buffer, methodName(fp->thisMethod));
        }
#endif /* TERSE_MESSAGES */

    END_ASSERTING_NO_ALLOCATION
}

#endif /* INCLUDEDEBUGCODE */

