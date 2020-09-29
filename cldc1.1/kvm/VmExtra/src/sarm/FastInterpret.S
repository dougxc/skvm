/*
 * Copyright (c) 2001 Sun Microsystems, Inc. All Rights Reserved.
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

/*****
 README:

This file contains an assembly-language interpreter loop for the
ARM Chip.  It can be optimized for either the ARM7 or for the ARM9/StrongArm
chip by either setting
     -DARM7=1
or   -DARM9=1

This file contains code that is intended to be run by the C preprocessor and
then compiled by a GNU assembler.  Metrowerks and armcc compilers use slightly
different assemblers.  In order to generate code that compiles for these
two compilers, you must preprocess this file, and then run it through
a Perl script that can be found in the Make file

In all cases, when a byte code is called, t0 contains the top element of the 
stack.  kvm_sp points to the top element that is actually written to the stack.
If the stack is empty, then t0 will contain the topmost element of the frame 
structure, just so things will work.

*****/

#if ARM9
#define ARM7 0
#endif

#ifndef ARM7
#define ARM7 1
#endif


/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Bytecode interpreter
 * FILE:      ARMexecute.S
 * OVERVIEW:  This file contains the assembly-language interpreter
 *            for the ARM architecture.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/


#include "offsets.h"
#include "messages.h"

#if GLOBALSTATE_ip_OFFSET != 0 || GLOBALSTATE_sp_OFFSET != 4 \
       || GLOBALSTATE_lp_OFFSET != 8 || GLOBALSTATE_cp_OFFSET != 12
# error Assembler code requires a precise layout of GlobalState
#endif

#define t0  r0
#define t1  r1
#define t2  r2
#define t3  r3
#define s0  r4
#define s1  r5
#define jt  r6
#define kvm_ip r7
#define kvm_sp r8
#define kvm_lp r9
#define kvm_cp r10
#define kvm_fp r11
#define t4  r12
#define t5  lr


/* If we are inside VMSAVE...VMRESTORE, then we can use the following as
 * global registers, since these registers have been flushed out to
 * GlobalState.
 */
#define x0     r7
#define x1     r8
#define x2     r9
#define x3     r10
#define x4     r11

/* The following definitions take care of endian problems.
 *
 * We use txy to refer to a pair of registers in which tx contains the
 * first word and ty contains the second word.  If the pair of words is a 
 * long, then Txy_HIGH refers to whichever of the two halves contains the
 * most significant 31 bits and the sign bit, and Txy_LOW refers to the
 * half containing the least-significant 32 bits.
 *
 * Note that if the two top elements of the stack are longs, and we perform
 * the instruction POPM(t1-t3), then the topmost long is now in t30 and the
 * second long is now in t1-t2.
 *
 * In other cases, we will have the opposite.  We will have a pair of
 * values in which "high" is the high order bits (and sign) and "low" is
 * the least significant bits.  The two macros
 *     HIGH_LOW_FIRST(high, low)
 *     HIGH_LOW_SECOND(high, low)
 * refer to whichever of "high" or "low" should be the first or second
 * word, respectively.
 */

#if BIG_ENDIAN
#  define t30_HIGH    t3
#  define t30_LOW     t0
#  define t12_HIGH    t1
#  define t12_LOW     t2
#  define t45_HIGH    t4
#  define t45_LOW     t5
#  define HIGH_LOW_FIRST(high, low)        high
#  define HIGH_LOW_SECOND(high, low)       low
#else
#  define t30_LOW     t3
#  define t30_HIGH    t0
#  define t12_LOW     t1
#  define t12_HIGH    t2
#  define t45_LOW     t4
#  define t45_HIGH    t5
#  define HIGH_LOW_FIRST(high, low)        low
#  define HIGH_LOW_SECOND(high, low)       high
#endif


/* Push or pop values off the KVM stack */

#define POP1_IF(cc, reg)  ldr##cc reg, [kvm_sp], $-4
#define POP1(reg)         ldr     reg, [kvm_sp], $-4

#define PUSH1_IF(cc, reg) str##cc reg, [kvm_sp, $4]!
#define PUSH1(reg)        str     reg, [kvm_sp, $4]!

#define GET_IF(cc, reg)  ldr##cc reg, [kvm_sp]
#define GET(reg)         ldr     reg, [kvm_sp]

#define PUT_IF(cc, reg)  str##cc reg, [kvm_sp]
#define PUT(reg)         str     reg, [kvm_sp]


/* Push or pop multiple values off the KVM stack.
 * Currently, the argument has to be a single range, such as t0-t3.  These
 * macros can't handle ranges such as "t0-t3, t5", but we don't currently
 * need that.
 */
#define POPM_IF(cc, regs)  ldm##cc##fa kvm_sp !, {regs}
#define POPM(regs)         ldmfa       kvm_sp !, {regs}

#define PUSHM_IF(cc, regs) stm##cc##fa kvm_sp !, {regs}
#define PUSHM(regs)        stmfa       kvm_sp !, {regs}
#define PUSHMx             stmfa       kvm_sp !,

#define GETM_IF(cc, regs)  ldm##cc##fa  kvm_sp, {regs}
#define GETM(regs)         ldmfa        kvm_sp, {regs}

/* Macros to deal with the global state */

/* Save the current state.  reg must contain GlobalState */
#define VM_REGISTERS  {kvm_ip, kvm_sp, kvm_lp, kvm_cp, kvm_fp}
#define VMSAVE_IF(cc,reg)    stm##cc##ia reg, VM_REGISTERS
#define VMSAVE(reg)          stmia       reg, VM_REGISTERS

#define VMRESTORE_IF(cc,reg) ldm##cc##ia reg, VM_REGISTERS
#define VMRESTORE(reg)       ldmia       reg, VM_REGISTERS

/* Save the state, call the specified function, and upon return, restore
 * the state and reschedule.
*/
#define CALL_SAVED_AND_RESCHEDULE_IF(cc, func)                \
        ldr##cc    s0, Addr_GlobalState;                      \
        GET_LOCAL_ADDRESS_IF(cc,lr,restoreAndReschedule_s0);  \
        VMSAVE_IF(cc, s0);                                    \
        /* INTERWORK note:                                    \
         * Even though the book says to use BL, it seems that \
         * the Linker replaces the function address with      \
         * the address of the veneer. Should this not work,   \
         * we may need to implement all the veneer here in    \
         * this code. Ugh.                                    \
         * This may need to be bx##cc.                        \
         */                                                   \
        b##cc func

#define CALL_SAVED_AND_RESCHEDULE(func) \
        CALL_SAVED_AND_RESCHEDULE_IF(al,func)


/* The process of finishing a byte code consists of three steps:
 *   UPDATE_OPCODE(size)
 *   JUMP1()
 *   JUMP2()
 *
 * There are three variants:
 *
 * 1) These macros use t5=lr, a volatile register.  If for some reason these
 * macros are being used around a function call, you must use a different
 * scratch register.  Use the _USING variant.
 *
 * 2) The can be conditionalized to only occur based on the condition codes
 * by using the _IF variant.
 *
 * 3) If there is no reasonable way to separate JUMP1() from JUMP2(), then
 * use JUMP() instead, which combines the operations of both.  Note that on
 * the ARM9, there is a gain if you can separate JUMP1() and JUMP2() by one
 * cycle.   
 *  
 * ARM7: It is fastest just to load the memory directly into the pc.
 * ARM9: It is fastest to load the memory to a register, do something else
 *       for a cycle, and then move the register to a pc.  There is no
 *       gain if the two can't be separated by a cycle.
 * StrongArm: It is fastest to lead the memory to a register, do something
 *       else for two cycles, and then move the register to a pc.  There is
 *       no gain if the two instructions are separated by one cycle, and
 *       you actually lose a cycle if the two instructions are next to each
 *       other.
 */

#define NEXT_BYTECODE_ADDRESS(reg)            [jt, reg, lsl $2]

#if ARM7

#define UPDATE_OPCODE_USING_IF(cc, reg, size) ldr##cc##b  reg, [kvm_ip, size]!
#define JUMP1_USING_IF(cc, reg)
#define JUMP2_USING_IF(cc, reg)               ldr##cc    pc, [jt, reg, lsl $2]
#define JUMP_USING_IF(cc,reg)                 ldr##cc    pc, [jt, reg, lsl $2]

#else

#define UPDATE_OPCODE_USING_IF(cc, reg, size) ldr##cc##b  reg, [kvm_ip, size]!
#define JUMP1_USING_IF(cc, reg)               ldr##cc     reg, [jt, reg, lsl $2]
#define JUMP2_USING_IF(cc, reg)               mov##cc     pc, reg
#define JUMP_USING_IF(cc,reg)                 ldr##cc     pc, [jt, reg, lsl $2]

#endif

#define UPDATE_OPCODE(size)             UPDATE_OPCODE_USING_IF(al,t5,size)
#define UPDATE_OPCODE_IF(cc, size)      UPDATE_OPCODE_USING_IF(cc,t5,size)
#define UPDATE_OPCODE_USING(reg, size)  UPDATE_OPCODE_USING_IF(al,reg,size)

#define JUMP()                          JUMP_USING_IF(al,t5)
#define JUMP_IF(cc)                     JUMP_USING_IF(cc,t5)
#define JUMP_USING(reg)                 JUMP_USING_IF(al,reg)

#define JUMP1()                         JUMP1_USING_IF(al,t5)
#define JUMP1_IF(cc)                    JUMP1_USING_IF(cc,t5)
#define JUMP1_USING(reg)                JUMP1_USING_IF(al,reg)

#define JUMP2()                         JUMP2_USING_IF(al,t5)
#define JUMP2_IF(cc)                    JUMP2_USING_IF(cc,t5)
#define JUMP2_USING(reg)                JUMP2_USING_IF(al,reg)


#if ARM7
/* On the ARM7, loads are relatively expensive (3 cycles).
 *
 * The fastest way to get the address of a local label is to use
 * adrl, which generates two arithmetic instructions.
 */
#if USING_OLD_GCC_COMPILER
#  define GET_LOCAL_ADDRESS_IF(cc,reg,label)     \
        adrl ## cc      reg, label
#else
#  define GET_LOCAL_ADDRESS_IF(cc,reg,label)     \
        adr ## cc ## l  reg, label
#endif
#else

/* On the ARM9, loads are cheap (1 cycle), provided you don't use the result
 * in the next instruction.
 *
 * The fastest way to get the address of a local label is to put
 * its address into a memory location.
 */

#  define GET_LOCAL_ADDRESS_IF(cc,reg,label)     \
        ldr##cc      reg,Addr_##label
#endif


#define GET_LOCAL_ADDRESS(reg,label)     \
      GET_LOCAL_ADDRESS_IF(al,reg,label)

#define BUBBLE(x)

/* Just for my sanity.  Put (high << 8) + low into result */
#define MAKE_SHORT_IF_S(cc, result, high, low)      \
                       add ## cc ## s   result, low, high, lsl $8

#define MAKE_SHORT_IF(cc,result,high,low) \
                       add ## cc        result, low, high, lsl $8
        
#define MAKE_SHORT(result, high, low)     MAKE_SHORT_IF(al,result,high,low)
#define MAKE_SHORTs(result, high, low)    MAKE_SHORT_IF_S(al,result,high,low)

#define IMPLEMENTATION(x) impl_ ## x


#if ARM7

#define CHECK_CLASS_INITIALIZED_1(reg)                 \
        GET_INSTANCE_CLASS_status(t4, reg);            \
        cmp     t4, $CLASS_READY;                      \
	bne     9f;                                    \
8:

#define CHECK_CLASS_INITIALIZED_2(reg)                 \
        .extern initializeClass ;                      \
9:      ldr     t5, Addr_CurrentThread ;               \
        ldr     t5, [t5];                              \
        GET_INSTANCE_CLASS_initThread(t4, reg);        \
        cmp     t4, t5;                                \
	beq     8b;                                    \
        mov	t0, reg;                               \
        CALL_SAVED_AND_RESCHEDULE(initializeClass)


#else

#define CHECK_CLASS_INITIALIZED_1(reg)                 \
        ldr     t5, Addr_CurrentThread ;               \
        GET_INSTANCE_CLASS_status(t4, reg);            \
        ldr     t5, [t5];                              \
        cmp     t4, $CLASS_READY;                      \
        GET_INSTANCE_CLASS_initThread_IF(ne, t4, reg); \
        cmpne   t4, t5;                                \
	bne     9f

#define CHECK_CLASS_INITIALIZED_2(reg)                 \
        .extern initializeClass ;                      \
9:      mov	t0, reg;                               \
        CALL_SAVED_AND_RESCHEDULE(initializeClass)

#endif





#if MASM
        AREA   text, CODE, READONLY, INTERWORK
        CODE32
        EXPORT  FastInterpret
#else
        .align 4
        .global FastInterpret
        .type FastInterpret,function
#endif

        /* Save registers that need to be saved */
FastInterpret:
        stmfd   sp!, {r4-r11, lr}

        /* We need eight words of stack (as the address of a long64
         * for checkTimerQueue()), but the spec says we should try to
         * keep the stack 8-byte aligned, and we're pushing 36 bytes
         * above.
         */
        /* r1-r3 below are just to throw away the top three stack words */ \
#define INTERPRETER_RETURN_IF(cc)          \
        ldm##cc##fd sp!, {r1-r3, r4-r11, pc}

        ldr     s0, Addr_GlobalState
        sub     sp, sp, $12
        VMRESTORE(s0)
        UPDATE_OPCODE($0)
        adrl     jt, JumpTable
        JUMP1()
        POP1(t0)                /* Prime the top of stack */
        JUMP2()

restoreAndReschedule_s0:
        VMRESTORE(s0)
        POP1(t0)

reschedule:
        /* if (isTimeToReschedule()) {  */

        /* We optimize for the case that Timeslice > 0, and
         * fill in most of the bubbles that way.
         */
        ldr     t2, Addr_Timeslice      /* t2 = &Timeslice */
        ldr     t1, [t2]        /* t1 = Timeslice */
        cmp     t1, $0          /* Timeslice == 0 ? */
        UPDATE_OPCODE_IF(ne,$0) /* Get the next bytecode */
        sub     t1, t1, $1      /* t1 = Timeslice - 1 */
        JUMP1_IF(ne)
        str     t1, [t2]        /* Timeslice-- */
        /* If not time to reschedule, then just dispatch on the byte code */
        JUMP2_IF(ne)

        /*
         * do  {
         *     ulong64 wakeupTime;
         *     if (!areAliveThreads()) {
         *         return;
         *     }
         *     checkTimerQueue(&wakeupTime);
         *     InterpreterHandleEvent(wakeupTime);
         * } while (!SwitchThread());
         */

        ldr     s0, Addr_GlobalState
        ldr     s1, Addr_AliveThreadCount
        PUSH1(t0)
        VMSAVE(s0)

rescheduleLoop:
        .extern InterpreterHandleEvent
        .extern checkTimerQueue
        .extern SwitchThread

        ldr     t1, [s1]        /* If AliveThreadCount <= 0, then return */
        mov     t0, sp          /* Use bubble to get &wakeupTime */
        cmp     t1, $0
        INTERPRETER_RETURN_IF(le)

        /* checkTimerQueue(&wakeupTime). t0 set above */
        bl      checkTimerQueue

        /* InterpreterHandleEvent(wakeupTime) */
        ldmia   sp,  {t0-t1}
        bl      InterpreterHandleEvent

        /* If SwitchThread() == 0 continue loop.
         * Otherwise, branch, based on the next byte code */
        bl      SwitchThread
        cmp     t0, $0
        VMRESTORE_IF(ne, s0)
        UPDATE_OPCODE_IF(ne, $0)
        beq     rescheduleLoop
        JUMP1_IF(ne)
        POP1_IF(ne, t0)
        JUMP2_IF(ne)


IMPLEMENTATION(NOP):
        /* Not much we can do to make this faster! */
        UPDATE_OPCODE($1)
        JUMP()

#define SIMPLE_MOVE_INT_CONSTANT(n) \
        UPDATE_OPCODE($1);          \
        PUSH1(t0);                  \
        JUMP1();                    \
        mov     t0, $n;             \
        JUMP2()

IMPLEMENTATION(ICONST_M1):
        SIMPLE_MOVE_INT_CONSTANT(-1)

IMPLEMENTATION(ACONST_NULL):
IMPLEMENTATION(ICONST_0):
        SIMPLE_MOVE_INT_CONSTANT(0)

IMPLEMENTATION(ICONST_1):
        SIMPLE_MOVE_INT_CONSTANT(1)

IMPLEMENTATION(ICONST_2):
        SIMPLE_MOVE_INT_CONSTANT(2)

IMPLEMENTATION(ICONST_3):
        SIMPLE_MOVE_INT_CONSTANT(3)

IMPLEMENTATION(ICONST_4):
        SIMPLE_MOVE_INT_CONSTANT(4)

IMPLEMENTATION(ICONST_5):
        SIMPLE_MOVE_INT_CONSTANT(5)

#define SIMPLE_MOVE_LONG_CONSTANT(n)              \
        UPDATE_OPCODE($1);                        \
        mov     t1, HIGH_LOW_FIRST($(n>>31),$n);  \
        JUMP1();                                  \
        PUSHM(t0-t1);                             \
        mov     t0, HIGH_LOW_SECOND($(n>>31),$n); \
        JUMP2();

IMPLEMENTATION(LCONST_0):
        SIMPLE_MOVE_LONG_CONSTANT(0)

IMPLEMENTATION(LCONST_1):
        SIMPLE_MOVE_LONG_CONSTANT(1)

IMPLEMENTATION(BIPUSH):
        UPDATE_OPCODE($2)
        PUSH1(t0)
        JUMP1()
        ldrsb   t0, [kvm_ip, $-1]
        JUMP2()

IMPLEMENTATION(SIPUSH):
        UPDATE_OPCODE($3)
        ldrsb   t3, [kvm_ip, $-2] /* high byte */
        ldrb    t4, [kvm_ip, $-1] /* low byte */
        JUMP1()
        PUSH1(t0)
        MAKE_SHORT(t0, t3, t4)
        JUMP2()

IMPLEMENTATION(LDC):
        UPDATE_OPCODE($2)
        ldrb    t3, [kvm_ip, $-1]
        JUMP1()
        PUSH1(t0)
        ldr     t0, [kvm_cp, t3, lsl $2]
        JUMP2()

IMPLEMENTATION(LDC_W):
        UPDATE_OPCODE($3)
        ldrb    t3, [kvm_ip, $-2]       /* high byte of index*/
        ldrb    t4, [kvm_ip, $-1]       /* low byte of index */
        JUMP1()
        MAKE_SHORT(t3, t3, t4)
        PUSH1(t0)
        ldr     t0, [kvm_cp, t3, lsl $2]
        JUMP2()

IMPLEMENTATION(LDC2_W):
        UPDATE_OPCODE($3)
        ldrb    t3, [kvm_ip, $-2]        /* high byte of index*/
        ldrb    t4, [kvm_ip, $-1]       /* low byte of index */
        JUMP1()
        MAKE_SHORT(t3, t3, t4)
        add     t3, kvm_cp, t3, lsl $2  /* address of cp value */
        /* The constant pool is always big endian.
         * The high word is at [t3,$0].  The low word is at [t3,$4]
         */
#if ARM7
        ldmia   t3, { t1-t2 }
        PUSHMx  { t0, HIGH_LOW_FIRST(t1, t2) }
        mov     t0, HIGH_LOW_SECOND(t1, t2)
#else
        ldr     t1, [t3, $ HIGH_LOW_FIRST(0,4)]
        PUSHM(t0-t1)
        ldr     t0, [t3, $ HIGH_LOW_SECOND(0,4)]
#endif
        JUMP2()

        .word IMPLEMENTATION(wide_ILOAD)
IMPLEMENTATION(ILOAD):
IMPLEMENTATION(ALOAD):
        UPDATE_OPCODE($2)
        ldrb    t3, [kvm_ip, $-1] /* lp index */
        JUMP1()
        PUSH1(t0)
        ldr     t0, [kvm_lp, t3, lsl $2]
        JUMP2()

IMPLEMENTATION(wide_ILOAD):
IMPLEMENTATION(wide_ALOAD):
        /* t3 already contains the register index */
	UPDATE_OPCODE($4)
        PUSH1(t0)
        JUMP1()
        ldr     t0, [kvm_lp, t3, lsl $2]
	JUMP2()

        .word IMPLEMENTATION(wide_LLOAD)
IMPLEMENTATION(LLOAD):
        UPDATE_OPCODE($2)
        ldrb    t3, [kvm_ip, $-1] /* lp index */
        JUMP1()
        add     t3, kvm_lp, t3, lsl $2 /* location of variable */
#if ARM7
        ldmia   t3, {t1-t2}            /* get the two values */
        PUSHM(t0-t1)
        mov     t0, t2
#else
        /* On the Arm9, there's no advantage to the ldm instruction.  The
         * code below is one cycle shorter than the above code sequence */
        ldr     t1, [t3]
        PUSHM(t0-t1)
        ldr     t0, [t3, $4]
#endif
        JUMP2()


IMPLEMENTATION(wide_LLOAD):
        /* t3 already contains the register index */
	UPDATE_OPCODE($4)
        add     t3, kvm_lp, t3, lsl $2 /* location of variable */
        JUMP1()
#if ARM7
        ldmia   t3, {t1-t2}            /* get the two values */
        PUSHM(t0-t1)
        mov     t0, t2
#else
        ldr     t1, [t3]
        PUSHM(t0-t1)
        ldr     t0, [t3, $4]
#endif
        JUMP2()


#define SIMPLE_CONSTANT_REGISTER_LOAD(n) \
        UPDATE_OPCODE($1);               \
        PUSH1(t0);                       \
        JUMP1();                         \
        ldr     t0, [kvm_lp, $n*4];      \
        JUMP2()


IMPLEMENTATION(ILOAD_0):
IMPLEMENTATION(ALOAD_0):
        SIMPLE_CONSTANT_REGISTER_LOAD(0)

IMPLEMENTATION(ILOAD_1):
IMPLEMENTATION(ALOAD_1):
        SIMPLE_CONSTANT_REGISTER_LOAD(1)


IMPLEMENTATION(ILOAD_2):
IMPLEMENTATION(ALOAD_2):
        SIMPLE_CONSTANT_REGISTER_LOAD(2)

IMPLEMENTATION(ILOAD_3):
IMPLEMENTATION(ALOAD_3):
        SIMPLE_CONSTANT_REGISTER_LOAD(3)

#if ARM9

#define LONG_CONSTANT_REGISTER_LOAD(n)    \
        UPDATE_OPCODE($1);                \
        ldr     t1, [kvm_lp, $(n*4)];     \
        JUMP1();                          \
        PUSHM(t0-t1);                     \
        ldr     t0, [kvm_lp, $((n*4)+4)]; \
        JUMP2()

#else

#define LONG_CONSTANT_REGISTER_LOAD(n)    \
        UPDATE_OPCODE($1);                \
        LONG_GET_REGISTER_ARM7_ ## n;     \
        JUMP1();                          \
        PUSHM(t0-t1);                     \
        mov     t0, t2;                   \
        JUMP2()

#define LONG_GET_REGISTER_ARM7_0                        ldmia kvm_lp, { t1-t2 }
#define LONG_GET_REGISTER_ARM7_1                        ldmib kvm_lp, { t1-t2 }
#define LONG_GET_REGISTER_ARM7_2   add t1, kvm_lp, $8;  ldmia t1, { t1-t2 }
#define LONG_GET_REGISTER_ARM7_3   add t1, kvm_lp, $12; ldmia t1, { t1-t2 }

#endif

IMPLEMENTATION(LLOAD_0):
        LONG_CONSTANT_REGISTER_LOAD(0)
IMPLEMENTATION(LLOAD_1):
        LONG_CONSTANT_REGISTER_LOAD(1)
IMPLEMENTATION(LLOAD_2):
        LONG_CONSTANT_REGISTER_LOAD(2)
IMPLEMENTATION(LLOAD_3):
        LONG_CONSTANT_REGISTER_LOAD(3)


#define CHECK_ARRAYx(array, index, scratch)               \
        cmp     array, $0;                                \
        GET_ARRAY_length_IF(ne, scratch, array);          \
        beq     handleNullPointerException;               \
        cmp     index, scratch;                           \

#define CHECK_ARRAY(array, index, scratch)                \
        CHECK_ARRAYx(array, index, scratch)               \
        bhs     handleArrayIndexOutOfBoundsException

/* This is optimized for the case that nothing is wrong */
#define SIMPLE_ARRAYLOAD(loaderSuffix, shift)             \
        POP1(t1);        /* t1 = array, t0 = index */     \
        CHECK_ARRAYx(t1, t0, t4);                         \
        UPDATE_OPCODE_IF(lo, $1);                         \
        JUMP1_IF(lo);                                     \
        addlo   t1, t1, t0, lsl $shift;                   \
        ldrlo ##loaderSuffix t0, [t1, $ARRAY_data_OFFSET];\
        JUMP2_IF(lo);                                     \
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(IALOAD):
IMPLEMENTATION(AALOAD):
        POP1(t1)        /* t1 = array, t0 = index */
        CHECK_ARRAYx(t1, t0, t4)
        UPDATE_OPCODE_IF(lo, $1)
        JUMP1_IF(lo)
        addlo   t1, t1, t0, lsl $2
        ldrlo   t0, [t1, $ARRAY_data_OFFSET]
        JUMP2_IF(lo)
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(LALOAD):
        POP1(t1);        /* t1 = array, t0 = index */
        CHECK_ARRAYx(t1, t0, t4)
        UPDATE_OPCODE_IF(lo, $1)
        addlo   t3, t1, t0, lsl $3 /* address of data */
        JUMP1_IF(lo)
#if ARM7
        addlo    t3, t3, $ARRAY_data_OFFSET
        ldmloia  t3, {t1-t2}
        PUSH1_IF(lo,t1)
        movlo   t0,t2
#else
        ldrlo    t1, [t3, $ARRAY_data_OFFSET]
        ldrlo    t0, [t3, $ARRAY_data_OFFSET + 4]
        PUSH1_IF(lo, t1)
#endif
        JUMP2_IF(lo)
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(BALOAD):
        SIMPLE_ARRAYLOAD(sb, 0)

IMPLEMENTATION(CALOAD):
        SIMPLE_ARRAYLOAD(h, 1)

IMPLEMENTATION(SALOAD):
        SIMPLE_ARRAYLOAD(sh, 1)

        .word IMPLEMENTATION(wide_ISTORE)
IMPLEMENTATION(ISTORE):
IMPLEMENTATION(ASTORE):
        UPDATE_OPCODE($2)
        ldrb    t3, [kvm_ip, $-1] /* lp index */
        JUMP1()
        str     t0, [kvm_lp, t3, lsl $2]
        POP1(t0)
        JUMP2()

IMPLEMENTATION(wide_ISTORE):
IMPLEMENTATION(wide_ASTORE):	
        /* t3 already contains the register index */
        UPDATE_OPCODE($4)
        str     t0, [kvm_lp, t3, lsl $2]
	JUMP1()
        POP1(t0)
        JUMP2()

        .word IMPLEMENTATION(wide_LSTORE)
IMPLEMENTATION(LSTORE):
        UPDATE_OPCODE($2)
        ldrb    t3, [kvm_ip, $-1] /* lp index */
        mov     t2, t0
        POPM(t0-t1)
        JUMP1()
        add     t3, kvm_lp, t3, lsl $2 /* address at which to store */
        stmia   t3, {t1-t2}
        JUMP2()

IMPLEMENTATION(wide_LSTORE):
        /* t3 already contains the register index */
	UPDATE_OPCODE($4)
        mov     t2, t0
        POPM(t0-t1)
        JUMP1()
        add     t3, kvm_lp, t3, lsl $2 /* address at which to store */
        stmia   t3, {t1-t2}
        JUMP2()

#define SIMPLE_CONSTANT_REGISTER_STORE(n)  \
        UPDATE_OPCODE($1);                 \
        str     t0, [kvm_lp, $n*4];        \
        JUMP1();                           \
        POP1(t0);                          \
        JUMP2()

IMPLEMENTATION(ISTORE_0):
IMPLEMENTATION(ASTORE_0):
        SIMPLE_CONSTANT_REGISTER_STORE(0)

IMPLEMENTATION(ISTORE_1):
IMPLEMENTATION(ASTORE_1):
        SIMPLE_CONSTANT_REGISTER_STORE(1)

IMPLEMENTATION(ISTORE_2):
IMPLEMENTATION(ASTORE_2):
        SIMPLE_CONSTANT_REGISTER_STORE(2)

IMPLEMENTATION(ISTORE_3):
IMPLEMENTATION(ASTORE_3):
        SIMPLE_CONSTANT_REGISTER_STORE(3)


#if ARM7

#define LONG_CONSTANT_REGISTER_STORE(n)    \
        UPDATE_OPCODE($1);                 \
        str     t0, [kvm_lp, $(n*4)+4];    \
        JUMP1();                           \
        POPM(t0-t1);                       \
        str     t1, [kvm_lp, $(n*4)];      \
        JUMP2()

#else

#define LONG_CONSTANT_REGISTER_STORE(n)    \
        UPDATE_OPCODE($1);                 \
        str     t0, [kvm_lp, $(n*4)+4];    \
        POP1(t1);                          \
        JUMP1();                           \
        POP1(t0);                          \
        str     t1, [kvm_lp, $(n*4)];      \
        JUMP2()

#endif

IMPLEMENTATION(LSTORE_0):
        LONG_CONSTANT_REGISTER_STORE(0)

IMPLEMENTATION(LSTORE_1):
        LONG_CONSTANT_REGISTER_STORE(1)

IMPLEMENTATION(LSTORE_2):
        LONG_CONSTANT_REGISTER_STORE(2)

IMPLEMENTATION(LSTORE_3):
        LONG_CONSTANT_REGISTER_STORE(3)

#define SIMPLE_ARRAY_STORE(storeSuffix, shift)                \
        POPM(t1-t2); /* t1 = array, t2 = index, t0=value */   \
        CHECK_ARRAYx(t1, t2, t4);                             \
        UPDATE_OPCODE_IF(lo, $1);                             \
        addlo   t1, t1, t2, lsl $shift;                       \
        JUMP1_IF(lo);                                         \
        strlo ## storeSuffix t0, [t1, $ARRAY_data_OFFSET];    \
        POP1_IF(lo, t0);                                      \
        JUMP2_IF(lo);                                         \
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(IASTORE):
        POPM(t1-t2) /* t1 = array, t2 = index, t0=value */
        CHECK_ARRAYx(t1, t2, t4)
        UPDATE_OPCODE_IF(lo, $1)
        addlo   t1, t1, t2, lsl $2
        JUMP1_IF(lo)
        strlo   t0, [t1, $ ARRAY_data_OFFSET]
        POP1_IF(lo, t0)
        JUMP2_IF(lo)
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(LASTORE):
        mov     s0, t0
        POPM(t0-t3)             /* t1 = array, t2 = index, t3/t4=value */
        CHECK_ARRAYx(t1, t2, t4)
        UPDATE_OPCODE_IF(lo,$1)
        addlo   t1, t1, t2, lsl $3
        JUMP1_IF(lo)
        strlo   t3, [t1, $ARRAY_data_OFFSET]
        strlo   s0, [t1, $(ARRAY_data_OFFSET + 4)]
        JUMP2_IF(lo)
        b       handleArrayIndexOutOfBoundsException

IMPLEMENTATION(BASTORE):
        SIMPLE_ARRAY_STORE(b, 0)

IMPLEMENTATION(SASTORE):
IMPLEMENTATION(CASTORE):
        SIMPLE_ARRAY_STORE(h, 1)

IMPLEMENTATION(AASTORE):
        .extern isAssignableTo
        .extern isAssignableToFast
        GETM(t1-t2)
        PUSH1(t0)
        /* t0 = value, t1 = array, t2 = index */
        CHECK_ARRAY(t1, t2, t4)

        cmp     t0, $0
        beq     2f              /* astore is okay if object NULL*/

        GET_INSTANCE_ofClass(s1, t1)   /* type of array */
        GET_INSTANCE_ofClass(s0, t0)   /* type of object */
        GET_ARRAY_CLASS_elemClass(s1, s1) /* type of array element */

        /* First try isAssignableToFast() */
        mov     t0, s0
        mov     t1, s1
        bl      isAssignableToFast
        cmp     t0, $0
        bne     1f              /* astore is okay */

        /* Next try isAssignableTo() */
        mov     t0, s0
        ldr     s0, Addr_GlobalState
        mov     t1, s1
        VMSAVE(s0)
        bl      isAssignableTo
        VMRESTORE(s0)
        cmp     t0, $0
        beq     handleArrayStoreException

1:      /* astore is okay */
        UPDATE_OPCODE($1)
        POPM(t0-t3)             /* t0 = tos t1 = array, t2 = index, t3=value */
        JUMP1()
        add     t1, t1, $ARRAY_data_OFFSET
        str     t3, [t1, t2, lsl $2]
        JUMP2()

2:      /* astore is okay, and we're storing NULL.  All arguments are
         * already stored in registers.
         */
        UPDATE_OPCODE($1)
        sub     kvm_sp, kvm_sp, $12     /* Remove arguments from stack */
        /* t1 = array, t2 = index, t0 = value = NULL */
        add     t1, t1, $ARRAY_data_OFFSET
        JUMP1()
        str     t0, [t1, t2, lsl $2]
        POP1(t0)
        JUMP2()

IMPLEMENTATION(POP):
#if !BIG_ENDIAN
IMPLEMENTATION(L2I):
#endif
        /* Throw away the current top of stack, and get a new one */
        UPDATE_OPCODE($1)
        POP1(t0)
        JUMP()

IMPLEMENTATION(POP2):
        UPDATE_OPCODE($1)
        sub     kvm_sp, kvm_sp, $4
        JUMP1()
        POP1(t0)
        JUMP2()

IMPLEMENTATION(DUP):
        UPDATE_OPCODE($1)
        PUSH1(t0)
        JUMP()

/* The various DUP bytecodes have simple implementations on the ARM chip.
 */
IMPLEMENTATION(DUP_X1):
        UPDATE_OPCODE($1)
        /* Stack = B (A) */
        POP1(t1)
        /* Stack = (A)      t1 = B */
        JUMP1()
        PUSHM(t0-t1)
        /* Stack = A B (A) */
        JUMP2()

IMPLEMENTATION(DUP_X2):
        UPDATE_OPCODE($1)
        /* Stack = C B (A) */
        POPM(t1-t2)
        /* Stack = (A)     t1 = C, t2 = B*/
        JUMP1()
        PUSHM(t0-t2)
        /* Stack = A C B (A) */
        JUMP2()

IMPLEMENTATION(DUP2):
        UPDATE_OPCODE($1)
        /* Stack = B (A) */
        GET(t1)
        /* Stack = B (A)   t1 = B */
        JUMP1()
        PUSHM(t0-t1)
        /* Stack = B A B (A)  */
        JUMP2()

IMPLEMENTATION(DUP2_X1):
        UPDATE_OPCODE($1)
        /* Stack = C B (A) */
        POPM(t1-t2)
        /* Stack = (A)     t1 = C, t2 = B*/
        JUMP1()
        PUSH1(t2)
        /* Stack = B (A)   t1 = C, t2 = B*/
        PUSHM(t0-t2)
        /* Stack = B A C B (A) */
        JUMP2()

IMPLEMENTATION(DUP2_X2):
        UPDATE_OPCODE($1)
        /* Stack = D C B (A) */
        POPM(t1-t3)
        /* Stack = (A)     t1 = D, t2 = C, t3 = B */
        JUMP1()
        PUSH1(t3)
        /* Stack = B (A)     t1 = D, t2 = C, t3 = B */
        PUSHM(t0-t3)
        /* Stack = B A D C B (A)   */
        JUMP2()

IMPLEMENTATION(SWAP):
        UPDATE_OPCODE($1)
        /* The swap instruction seems to execute slowly on some ARMs */
        POP1(t1)
        JUMP1()
        PUSH1(t0)
        mov     t0,t1
        JUMP2()


#define SIMPLE_INT_ARITH_OP(opcode)   \
        UPDATE_OPCODE($1);            \
        POP1(t1);                     \
        JUMP1();                      \
        opcode  t0, t1, t0;           \
        JUMP2()

#define SIMPLE_LONG_ARITH_OP(low, high)       \
        UPDATE_OPCODE($1);                    \
        /* t3:t0 = t1:t2 op t3:t0 */          \
        POPM(t1-t3);                          \
        JUMP1();                              \
        low     t30_LOW, t12_LOW, t30_LOW;    \
        high    t30_HIGH, t12_HIGH, t30_HIGH; \
        PUSH1(t3);                            \
        JUMP2()

IMPLEMENTATION(IADD):
        SIMPLE_INT_ARITH_OP(add)

IMPLEMENTATION(LADD):
        SIMPLE_LONG_ARITH_OP(adds, adc)

IMPLEMENTATION(ISUB):
        SIMPLE_INT_ARITH_OP(sub)

IMPLEMENTATION(LSUB):
        SIMPLE_LONG_ARITH_OP(subs, sbc)

IMPLEMENTATION(IMUL):
        SIMPLE_INT_ARITH_OP(mul)

IMPLEMENTATION(LMUL):
        POPM(t1-t3)
        /* t4:t5 = t1:t2 * t3:t0 */
        umull   t45_LOW, t45_HIGH, t12_LOW, t30_LOW
        UPDATE_OPCODE_USING(s0, $1)
        mla     t45_HIGH, t12_LOW, t30_HIGH, t45_HIGH
        mla     t45_HIGH, t30_LOW, t12_HIGH, t45_HIGH
        JUMP1_USING(s0)
        PUSH1(t4)
        mov     t0, t5
        JUMP2_USING(s0)

#define INT_DIVISION(name)                     \
        .extern FastInterpret_ ## name;        \
        cmp     t0, $0;                        \
        beq     handleDivideByZeroException;   \
        mov     t1, t0;                        \
        UPDATE_OPCODE_USING(s0, $1);           \
        POP1(t0);                              \
        /* Make this a "call and go" */        \
        ldr     lr, NEXT_BYTECODE_ADDRESS(s0); \
        b       FastInterpret_ ## name

#define LONG_DIVISION(name)                  \
        .extern FastInterpret_ ## name;      \
        mov     t3, t0;                      \
        POPM(t0-t2);                         \
        orrs    t5, t2, t3;                  \
        beq     handleDivideByZeroException; \
        UPDATE_OPCODE_USING(s0, $1);         \
        bl      FastInterpret_ ## name;      \
        JUMP1_USING(s0);                     \
        PUSH1(t0);                           \
        mov     t0, t1;                      \
        JUMP2_USING(s0)


IMPLEMENTATION(IDIV):
        INT_DIVISION(idiv)

IMPLEMENTATION(LDIV):
        LONG_DIVISION(ldiv)

IMPLEMENTATION(IREM):
        INT_DIVISION(irem)

IMPLEMENTATION(LREM):
        LONG_DIVISION(lrem)

IMPLEMENTATION(INEG):
        UPDATE_OPCODE($1)
        rsb     t0, t0, $0
        JUMP()

IMPLEMENTATION(LNEG):
        UPDATE_OPCODE($1)
        POP1(t3)
        /* Value is in t3:t0 */
        rsbs    t30_LOW,  t30_LOW, $0
        JUMP1()
        rsc     t30_HIGH, t30_HIGH, $0
        PUSH1(t3)
        JUMP2()

#define SIMPLE_INT_SHIFT(shifter)      \
        UPDATE_OPCODE($1);             \
        POP1(t1);                      \
        JUMP1();                       \
        and     t0, t0, $31;           \
        mov     t0, t1, shifter t0;    \
        JUMP2()

#define LONG_SHIFT_HEADER()            \
        ands    s0, t0, $63;           \
        beq     IMPLEMENTATION(POP);   \
        UPDATE_OPCODE($1);             \
        POPM(t1-t2);                   \
        JUMP1();                       \
        rsbs    s1, s0, $32            \
        /* s0 = shift; s1 = 32-shift.
         * If s1 <= 0, then s0 >= 32, and we are doing a simple shift
         * If s1 > 0, then s0 < 32, and the shift is a little bit more complex
         */

IMPLEMENTATION(ISHL):
        SIMPLE_INT_SHIFT(lsl)

IMPLEMENTATION(LSHL):
        LONG_SHIFT_HEADER()

        /* Simple shift */
        suble   s0, s0, $32
        movle   t30_HIGH, t12_LOW, lsl s0
        movle   t30_LOW, $0
        /* Complex shift */
        movgt   t30_HIGH, t12_HIGH, lsl s0
        orrgt   t30_HIGH, t30_HIGH, t12_LOW, lsr s1
        movgt   t30_LOW, t12_LOW, lsl s0

        PUSH1(t3)
        JUMP2()

IMPLEMENTATION(ISHR):
        SIMPLE_INT_SHIFT(asr)

IMPLEMENTATION(LSHR):
        LONG_SHIFT_HEADER()

        /* Simple shift */
        suble   s0, s0, $32
        movle   t30_LOW, t12_HIGH, asr s0
        movle   t30_HIGH, t12_HIGH, asr $32
        /* Complex shift */
        movgt   t30_LOW, t12_LOW, lsr s0
        orrgt   t30_LOW, t30_LOW, t12_HIGH, lsl s1
        movgt   t30_HIGH, t12_HIGH, asr s0

        PUSH1(t3)
        JUMP2()

IMPLEMENTATION(IUSHR):
        SIMPLE_INT_SHIFT(lsr)

IMPLEMENTATION(LUSHR):
        LONG_SHIFT_HEADER()

        /* Simple shift */
        suble   s0, s0, $32
        movle   t30_LOW, t12_HIGH, lsr s0
        movle   t30_HIGH, $0
        /* Complex shift */
        movgt   t30_LOW, t12_LOW, lsr s0
        orrgt   t30_LOW, t30_LOW, t12_HIGH, lsl s1
        movgt   t30_HIGH, t12_HIGH, lsr s0

        PUSH1(t3)
        JUMP2()

IMPLEMENTATION(IAND):
        SIMPLE_INT_ARITH_OP(and)

IMPLEMENTATION(LAND):
        SIMPLE_LONG_ARITH_OP(and, and)

IMPLEMENTATION(IOR):
        SIMPLE_INT_ARITH_OP(orr)

IMPLEMENTATION(LOR):
        SIMPLE_LONG_ARITH_OP(orr, orr)

IMPLEMENTATION(IXOR):
        SIMPLE_INT_ARITH_OP(eor)

IMPLEMENTATION(LXOR):
        SIMPLE_LONG_ARITH_OP(eor, eor)

        .word IMPLEMENTATION(wide_IINC)
IMPLEMENTATION(IINC):
        /* Top of stack is unaffected */
        ldrb    t3, [kvm_ip, $1] /* index */
        ldrsb   t1, [kvm_ip, $2] /* signed offset */
        UPDATE_OPCODE($3)
        ldr     t2, [kvm_lp, t3, lsl $2] /* old value of register */
        JUMP1()
        add     t2, t2, t1      /* update the register */
        str     t2, [kvm_lp, t3, lsl $2]
        JUMP2()

IMPLEMENTATION(wide_IINC):
        /* We carefully don't touch t0 containing the stack top */
        ldrsb   t1, [kvm_ip, $4]    /* High byte, increment */
        ldrb    t2, [kvm_ip, $5]    /* Low byte, increment */
        UPDATE_OPCODE($6)
        ldr     s0, [kvm_lp, t3, lsl $2] /* Get old value of register */
        MAKE_SHORT(t1, t1, t2)      /* Increment */
        JUMP1()
        add     s0, s0, t1          /* update the register */
        str     s0, [kvm_lp, t3, lsl $2] /* Save new value of register */
        JUMP2()

IMPLEMENTATION(I2L):
        UPDATE_OPCODE($1)
#if BIG_ENDIAN
        mov     t1, t0, asr #31
        JUMP1()
        PUSH1(t1)
#else
        PUSH1(t0)
        JUMP1()
        mov     t0, t0, asr #31
#endif
        JUMP2()

#if BIG_ENDIAN
IMPLEMENTATION(L2I):
        UPDATE_OPCODE($1)
        /* Remove the high word sitting on the stack */
        add     kvm_sp, kvm_sp, #4
        JUMP()
#else
        /* same as op_POP */
#endif


IMPLEMENTATION(I2B):
        UPDATE_OPCODE($1);
        mov     t0, t0, lsl $24
        JUMP1()
        mov     t0, t0, asr $24
        JUMP2()

IMPLEMENTATION(I2C):
        UPDATE_OPCODE($1);
        mov     t0, t0, lsl $16
        JUMP1()
        mov     t0, t0, lsr $16
        JUMP2()

IMPLEMENTATION(I2S):
        UPDATE_OPCODE($1);
        mov     t0, t0, lsl $16
        JUMP1()
        mov     t0, t0, asr $16
        JUMP2()

#define LCMP_IFXX(macro, arg1, arg2, cc, notcc) \
        macro ## 1(arg1,arg2);                  \
        macro ## 2(arg1,arg2);                  \
        UPDATE_OPCODE_IF(notcc, $3);            \
        POP1(t0);                               \
        JUMP_IF(notcc);                         \
        ldr ## cc ## sb   t3, [kvm_ip, $1];     \
        ldr ## cc ## b    t4, [kvm_ip, $2];     \
        MAKE_SHORTs(t3, t3, t4);                \
        UPDATE_OPCODE_IF(gt, t3);               \
        addle   kvm_ip, kvm_ip, t3;             \
        JUMP_IF(gt);                            \
        ble     reschedule;                     \
        nop; nop; nop; nop

#define LCMP_IFEQ1(X,Y) cmp     X ## _LOW,  Y ## _LOW
#define LCMP_IFEQ2(X,Y) cmpeq   X ## _HIGH, Y ## _HIGH

#define LCMP_IFLT1(X,Y) cmp     X ## _LOW, Y ## _LOW
#define LCMP_IFLT2(X,Y) sbcs    t0, X ## _HIGH, Y ## _HIGH  /* t0 scratch */

IMPLEMENTATION(LCMP):
        UPDATE_OPCODE($1)
        POPM(t1-t3)
        subs    t4, lr, $OP_IFEQ
        cmp     t4, $(OP_IFLE - OP_IFEQ)
        /* We immediately follow this code with 16 instructions each
         * for lcmp/ifeq lcmp/ifne .... lcmp/ifle
        */
        addls   pc, pc, t4, lsl $6    /* 16 instructions per byte code */
        b       lcmp_normal

/* This must come two instructions after the addls above */
IMPLEMENTATION(LCMP_IFEQ):
        LCMP_IFXX(LCMP_IFEQ, t12, t30, eq, ne)

IMPLEMENTATION(LCMP_IFNE):
        LCMP_IFXX(LCMP_IFEQ, t12, t30, ne, eq)

IMPLEMENTATION(LCMP_IFLT):
        LCMP_IFXX(LCMP_IFLT, t12, t30, lt, ge)

IMPLEMENTATION(LCMP_IFGE):
        LCMP_IFXX(LCMP_IFLT, t12, t30, ge, lt)

IMPLEMENTATION(LCMP_IFGT):
        LCMP_IFXX(LCMP_IFLT, t30, t12, lt, ge)

IMPLEMENTATION(LCMP_IFLE):
        LCMP_IFXX(LCMP_IFLT, t30, t12, ge, lt)

lcmp_normal:
        /* This is an lcmp not immediately followed by ifXX.  This will
         * probably occur only in the test suite.
         *
         * t1:t2 = t1:t2 - t3:t0
         */
        subs    t12_LOW, t12_LOW, t30_LOW
        sbcs    t12_HIGH, t12_HIGH, t30_HIGH
        /* The condition codes are set correctly for testing < and >=, but
         * not for >, <=, ==, or !=
         */
        movlt  t0, $-1
        movge  t0, $1           /* Wrong if ==, but fixed below */
        JUMP1()
        orrs   t12_LOW, t12_LOW, t12_HIGH  /* Are both words 0? */
        moveq  t0, $0
        JUMP2()


/* This is about half way through the code.  By putting the
 * constant pool in the middle, it is reachable from all parts of
 * the code
 */


Addr_AliveThreadCount:
        .extern AliveThreadCount
        .word AliveThreadCount
Addr_CurrentThread:
        .extern CurrentThread
        .word CurrentThread
Addr_GlobalState:
        .extern GlobalState
        .word GlobalState
#if ENABLEFASTBYTECODES
Addr_InlineCache:
        .extern InlineCache
        .word InlineCache
#endif
Addr_PrimitiveArrayClasses:
        .extern PrimitiveArrayClasses
        .word PrimitiveArrayClasses
Addr_thisObjectGCSafe:
        .extern thisObjectGCSafe
        .word thisObjectGCSafe
Addr_Timeslice:
        .extern Timeslice
        .word Timeslice

Addr_ArithmeticException:
        .extern ArithmeticException
        .word ArithmeticException
Addr_ArrayIndexOutOfBoundsException:
        .extern ArrayIndexOutOfBoundsException
        .word ArrayIndexOutOfBoundsException
Addr_ArrayStoreException:
        .extern ArrayStoreException
        .word ArrayStoreException
Addr_ClassCastException:
        .extern ClassCastException
        .word ClassCastException
Addr_NullPointerException:
        .extern NullPointerException
        .word NullPointerException

#if !ARM7
Addr_restoreAndReschedule_s0:
        .word restoreAndReschedule_s0
#endif


#if ARM7
#define SIMPLE_IF_1(cc, notcc)             \
        cmp     t0, $0;                    \
        POP1(t0);                          \
        SIMPLE_IF_ARM7_END(cc, notcc)

#define SIMPLE_IF_2(cc, notcc)             \
        mov     t2, t0;                    \
        POPM(t0-t1);                       \
        cmp     t1, t2;                    \
        SIMPLE_IF_ARM7_END(cc, notcc)

#define SIMPLE_IF_ARM7_END(cc, notcc)       \
        UPDATE_OPCODE_IF(notcc, $3);        \
        JUMP_IF(notcc);                     \
        ldrsb   t3, [kvm_ip, $1];           \
        ldrb    t4, [kvm_ip, $2];           \
        MAKE_SHORTs(t3, t3, t4);            \
        UPDATE_OPCODE_IF(gt, t3);           \
        JUMP_IF(gt);                        \
        add     kvm_ip, kvm_ip, t3;         \
        b       reschedule                  \

#else

#define SIMPLE_IF_1(cc, notcc)              \
        cmp     t0, $0;                     \
        UPDATE_OPCODE_IF(notcc, $3);        \
        POP1(t0);                           \
        JUMP_IF(notcc);                     \
        ldrsb   t3, [kvm_ip, $1];           \
        SIMPLE_IF_ARM9_END

#define SIMPLE_IF_2(cc, notcc)              \
        POP1(t1);                           \
        ldrsb   t3, [kvm_ip, $1];           \
        cmp     t1, t0;                     \
        UPDATE_OPCODE_IF(notcc, $3);        \
        POP1(t0);                           \
        JUMP_IF(notcc);                     \
        SIMPLE_IF_ARM9_END

#define SIMPLE_IF_ARM9_END                  \
        ldrb    t4, [kvm_ip, $2];           \
        MAKE_SHORTs(t3, t3, t4);            \
        UPDATE_OPCODE(t3);                  \
        JUMP_IF(gt);                        \
        ble     reschedule                  \

#endif


IMPLEMENTATION(IFEQ):
IMPLEMENTATION(IFNULL):
        SIMPLE_IF_1(eq, ne)

IMPLEMENTATION(IFNE):
IMPLEMENTATION(IFNONNULL):
        SIMPLE_IF_1(ne, eq)

IMPLEMENTATION(IFLT):
        SIMPLE_IF_1(lt, ge)

IMPLEMENTATION(IFGE):
        SIMPLE_IF_1(ge, lt)

IMPLEMENTATION(IFGT):
        SIMPLE_IF_1(gt, le)

IMPLEMENTATION(IFLE):
        SIMPLE_IF_1(le, gt)

IMPLEMENTATION(IF_ICMPEQ):
IMPLEMENTATION(IF_ACMPEQ):
        SIMPLE_IF_2(eq, ne)

IMPLEMENTATION(IF_ICMPNE):
IMPLEMENTATION(IF_ACMPNE):
        SIMPLE_IF_2(ne, eq)

IMPLEMENTATION(IF_ICMPLT):
        SIMPLE_IF_2(lt, ge)

IMPLEMENTATION(IF_ICMPGE):
        SIMPLE_IF_2(ge, lt)

IMPLEMENTATION(IF_ICMPGT):
        SIMPLE_IF_2(gt, le)

IMPLEMENTATION(IF_ICMPLE):
        SIMPLE_IF_2(le, gt)

IMPLEMENTATION(GOTO):
        /* This does not touch the top of stack */
        ldrsb   t3, [kvm_ip, $1]
        ldrb    t4, [kvm_ip, $2]
        MAKE_SHORTs(t3, t3, t4)
        UPDATE_OPCODE(t3)
        ble     reschedule
        JUMP()


#if BIG_ENDIAN
     /* On big-endian machines, there's nothing to do */
#  define BSWAP_SETUP(magic)
#  define BSWAP(reg, magic, tmp)
#  define BSWAPs(reg, magic, tmp)  cmp reg, $0
#else

#  define BSWAP_SETUP(magic) \
           mov magic, $0xFF00; orr magic, magic, magic, lsl $16
#  define BSWAP(reg, magic, tmp)        /* reg =  ABCD */   \
        and  tmp, magic, reg, ror $8;   /* tmp  = D0B0 */   \
        and  reg, magic, reg, ror $16;  /* reg =  C0A0 */   \
        orr  reg, tmp, reg, lsr $8      /* reg =  DCBA */

/* Note, we have to use adds rather than orrs as the last instruction
 * of BSWAPs, since we need to clear the V bit of the condition codes
 * in order for us to be able to use signed comparisons.
 */
#  define BSWAPs(reg, magic, tmp)       /* reg =  ABCD */   \
        and  tmp, magic, reg, ror $8;   /* tmp  = D0B0 */   \
        and  reg, magic, reg, ror $16;  /* reg =  C0A0 */   \
        adds reg, tmp, reg, lsr $8      /* reg =  DCBA */
#endif


IMPLEMENTATION(TABLESWITCH):
        /* Set t0 to the key
         * Set t5 to the start of the table
         * s0 to the constant required for byte swapping
         */
        add     t5, kvm_ip, $4
        BSWAP_SETUP(s0)
        bic     t5, t5, $3

        /*
         * Get the first three values from the table
         *     t1 = default offset
         *     t2 = low
         *     t3 = high
         * and update
         *     t5 = start of offsets
         */
        ldmia   t5 !, { t1, t2, t3}

        BSWAP(t2,s0,s1)
        BSWAP(t3,s0,s1)
        cmp     t3, t0          /* high >= value */
        subges  t0, t0, t2      /* and value >= low, also value -= low */
        ldrge   t1, [t5, t0, lsl $2]

        /* No matter what, t1 contains the (byte swapped) offset */
        BSWAPs(t1,s0,s1)
        UPDATE_OPCODE(t1)
        POP1(t0)
        JUMP1_IF(gt)
        ble     reschedule
        JUMP2_IF(gt)

#define key        t0
#define delta      t1
#define halfDelta  t2
#define firstPair  t3
#define x          t4
#define y          t5

IMPLEMENTATION(LOOKUPSWITCH):
        BSWAP_SETUP(s0)
        bic     x, kvm_ip, $3     /* x is one word before "base" */
        ldr     delta, [x, $8]    /* number of pairs, at base[1] */
        BSWAP(delta, s0, s1)
        add     firstPair, x, $12 /* pointer to first pair, at base[2] */
        sub     delta, delta, $1  /* delta = numberOfPairs - 1 */

1:      /* Main loop of lookupswitch */

        /* if delta < 0, then the item isn't in the table */
        cmp     delta, $0
        blt     2f

        /* look at the item right in the middle */
        mov     halfDelta, delta, asr $1
        ldr     x, [firstPair, halfDelta, lsl $3] /* x = middle value */
        add     y, firstPair, halfDelta, lsl $3   /* y = &middle value */
        BSWAP(x,s0,s1)
        cmp     x, key              /* compare middle value and value */

        /* if (value > key) {
         *      delta = halfDelta - 1;
         *      continue;
         * }
         */
        subgt   delta, halfDelta, $1
        /* if (value < key) {
         *     firstPair = middlePair + 1;
         *     delta = delta - halfDelta - 1
         *     cotinue;
         * }
         */
        addlt   firstPair, y, $8
        rsblt   y, halfDelta, delta
        sublt   delta, y, $1
        bne     1b

2:      /* If we've gotten here by falling through, then we have found
         *     the pair.   y points at the pair, and the condition codes
         *     are set to 'eq'.
         * If we've branched here, then the key isn't in the table.
         *     The condition codes are set to 'lt'.
         */
        biclt   y, kvm_ip, $3
        /* In either case, the offset is at [y, $4] */
        ldr     x, [y, $4]      /* y = middlePair->offset */
        /* We're optimizing for the case that most branches are forward */
        BSWAPs(x,s0,s1)
        UPDATE_OPCODE(x)
        POP1(t0)
        JUMP1_IF(gt)
        ble     reschedule
        JUMP2_IF(gt)


#undef delta
#undef key
#undef base
#undef firstPair
#undef halfDelta
#undef x
#undef y


#if FRAME_previousFp_OFFSET == 0 && FRAME_previousIp_OFFSET == 4 \
      && FRAME_previousSp_OFFSET == 8
#else
#  error Bad offset assumptions
#endif

#define HANDLE_RETURN(count)                                               \
        SAVE_RETURN_VALUES_ ## count;                                      \
        GET_FRAME_syncObject(t1, kvm_fp);                                  \
        /* We handle the monitorExit case out of line */                   \
        cmp     t1, $0;         /* synchronized == NULL? */                \
        bne     2f;                                                        \
                                                                           \
1:      /* Undo the stack */                                               \
        GET_FRAME_previousIp(t4, kvm_fp);                                  \
        cmp     t4, $KILLTHREAD;                                           \
        beq     return_killThread;                                         \
                                                                           \
        ldmia   kvm_fp, { t3, kvm_ip, kvm_sp };                            \
        GET_FRAME_thisMethod(t5, t3);  /* t5 = thisMethod */               \
        mov     kvm_fp, t3;                                                \
                                                                           \
        GET_METHOD_frameSize(t2, t5);   /* t2 = frameSize  */              \
        GET_METHOD_ofClass(t4, t5);     /* t4 = thisMethod->ofClass */     \
        sub     kvm_lp, kvm_fp, t2, lsl $2;                                \
        GET_INSTANCE_CLASS_constPool(kvm_cp, t4);                          \
        /* Set up new top of stack */                                      \
        RESTORE_TOP_OF_STACK_ ## count;                                    \
        b       reschedule;                                                \
                                                                           \
2:      /* monitorExit(object, &error) */                                  \
        .extern monitorExit;						   \
        SAVE_T0_IN_S0_IF_NECESSARY_ ## count(al);                          \
        mov	t0, t1;                /* object */                        \
        mov     t1, sp;							   \
        bl      monitorExit;						   \
        /* If no error, return to caller */				   \
        cmp     t0, $MonitorStatusError;				   \
        RESTORE_T0_FROM_S0_IF_NECESSARY_ ## count(ne);                     \
        bne     1b;							   \
        /* Call the error handler */					   \
        ldr     t0, [sp];						   \
        b       handleException;


#define SAVE_RETURN_VALUES_0                   /* Nothing to save */
#define RESTORE_TOP_OF_STACK_0                 POP1(t0)
#define SAVE_T0_IN_S0_IF_NECESSARY_0(cc)
#define RESTORE_T0_FROM_S0_IF_NECESSARY_0(cc)

#define SAVE_RETURN_VALUES_1                   /* Top of stack is in t0 */
#define RESTORE_TOP_OF_STACK_1                 /* Top of stack is in t0 */
#define SAVE_T0_IN_S0_IF_NECESSARY_1(cc)       mov ## cc s0, t0
#define RESTORE_T0_FROM_S0_IF_NECESSARY_1(cc)  mov ## cc t0, s0

#define SAVE_RETURN_VALUES_2                   POP1(s1)
#define RESTORE_TOP_OF_STACK_2                 PUSH1(s1)
#define SAVE_T0_IN_S0_IF_NECESSARY_2(cc)       mov ## cc s0, t0
#define RESTORE_T0_FROM_S0_IF_NECESSARY_2(cc)  mov ## cc t0, s0

IMPLEMENTATION(RETURN):
        HANDLE_RETURN(0)

IMPLEMENTATION(IRETURN):
IMPLEMENTATION(ARETURN):
        HANDLE_RETURN(1)

IMPLEMENTATION(LRETURN):
        HANDLE_RETURN(2)

return_killThread:
        /* kill this thread, and possibly return from the interpreter
         *
         * VMSAVE
         *    stopThread();
         * VMRESTORE
         * if (areAliveThreads()) {
         *     goto reschedule;
         * } else {
         *     return;
         * }
         */
        .extern stopThread

        ldr     s0, Addr_GlobalState
        ldr     s1, Addr_AliveThreadCount
        VMSAVE(s0)
        bl      stopThread

        ldr     t0, [s1]        /* t0 = AliveThreadCount */
        VMRESTORE(s0)
        cmp     t0, $0
        bgt     reschedule
        INTERPRETER_RETURN_IF(le)


IMPLEMENTATION(NEWARRAY):
        .extern instantiateArray
        mov     t1, t0          /* length */
        ldr     t3, Addr_PrimitiveArrayClasses
        ldrb    t2, [kvm_ip, $1] /* array type  code */
        ldr     s0, Addr_GlobalState
        ldr     t0, [t3, t2, lsl $2] /* primitive array type */
        VMSAVE(s0)
        bl      instantiateArray
        VMRESTORE(s0)

        UPDATE_OPCODE($2)       /* Optimize for assuming no problem */
        cmp     t0, $0
        JUMP_IF(ne)

        /* Undo the UPDATE_OPCODE($2) above */
        add     kvm_ip, kvm_ip, $-2
        POP1(t0) /* need to prime top of stack again */
        b       reschedule

IMPLEMENTATION(ARRAYLENGTH):
        UPDATE_OPCODE($1)
        cmp     t0, $0
        JUMP1_IF(ne)
        GET_ARRAY_length_IF(ne, t0, t0)
        JUMP2_IF(ne)

        /* Undo the UPDATE_OPCODE($1) above */
        add     kvm_ip, kvm_ip, $-1
        b       handleNullPointerException

IMPLEMENTATION(ATHROW):
        .extern throwException

        cmp     t0, $0
        beq     handleNullPointerException

        ldr     s1, Addr_thisObjectGCSafe
        ldr     s0, Addr_GlobalState
        str     t0, [s1]        /* thisObjectGCSafe = exception */

        VMSAVE(s0)
        /* throwException(&thisObjectGCSafe) */
        mov     t0, s1
        bl      throwException
        VMRESTORE(s0)

        /* thisObjectGCSafe = NULL */
        mov     t1, $0
        str     t1, [s1]
        POP1(t0)                /* Get new top of stack */
        b       reschedule

IMPLEMENTATION(MONITORENTER):
        .extern monitorEnter

        /* OBJECT object = popStackAsType(OBJECT);
         * CHECK_NOT_NULL(object)
         * ip++;
         * VMSAVE
         *    monitorEnter(object);
         * VMRESTORE
         */
        cmp     t0, $0
        addne   kvm_ip, kvm_ip, $1

        /* WARNING TO PORTERS:  If the following line gives you an assembler
         * error, you are using an older GNU assembler. Set the flag:
         *      -DUSING_OLD_GCC_COMPILER
	 * to get this line to assemble correctly
         */
        CALL_SAVED_AND_RESCHEDULE_IF(ne, monitorEnter)

        /* The above does not return.  We only fall through if t0==NULL */
        b     handleNullPointerException


IMPLEMENTATION(MONITOREXIT):
        .extern monitorExit

        /*
         * OBJECT object = popStackAsType(OBJECT);
         * CHECK_NOT_NULL(object)
         * ip++; /* In case we context switch in monitorExit one day
         */
        cmp     t0, $0
        beq     handleNullPointerException

        add     kvm_ip, kvm_ip, $1 /* in case we ever context switch */
        /*
         * if (monitorExit(object, &exitFailure) == MonitorStatusError) {
         *     exception = exitFailure;
         *     goto handleException;
         * }
         */
        mov     t1, sp
        bl      monitorExit
        cmp     t0, $MonitorStatusError
        POP1_IF(ne, t0)         /* Get new top of stack */
        bne     reschedule
        ldr     t0, [sp]
        b       handleException

IMPLEMENTATION(WIDE):
        ldrb    s0, [kvm_ip, $1]    /* Opcode */
        ldrb    t3, [kvm_ip, $2]    /* High byte, register */
        ldrb    t4, [kvm_ip, $3]    /* Low byte, register */
	ldr	s0, NEXT_BYTECODE_ADDRESS(s0);
        MAKE_SHORT(t3, t3, t4)
	ldr	pc, [s0, $-4]

IMPLEMENTATION(GOTO_W):
        /* This opcode does not affect the stack */
        ldrsb   t1, [kvm_ip, $1]     /* so we know sign of branch */
        ldrb    t2, [kvm_ip, $2]
        ldrb    t3, [kvm_ip, $3]
        ldrb    t4, [kvm_ip, $4]
        MAKE_SHORT(t1, t1, t2)
        MAKE_SHORT(t3, t3, t4)
        adds    t1, t3, t1, lsl $16
        UPDATE_OPCODE(t1)
        ble     reschedule
        JUMP()

IMPLEMENTATION(BREAKPOINT):
        .extern fatalError

        adrl    t0, Msg_BreakPoint
        bl      fatalError


IMPLEMENTATION(GETFIELD_FAST):
IMPLEMENTATION(GETFIELDP_FAST):
        /* index = getShort(ip + 1);
         * instance = popStackAsType(INSTANCE);
         * CHECK_NOT_NULL(instance);
         * pushStack(instance->data[index].cell);
         *
         * Optimized for the non-NULL case!
         */
        cmp     t0, $0

        UPDATE_OPCODE_IF(ne, $3)
        ldrneb  t3, [kvm_ip, $-2]        /* high byte */
        ldrneb  t4, [kvm_ip, $-1]        /* low byte */
        addne   t0, t0, $INSTANCE_data_OFFSET
        JUMP1_IF(ne)
        MAKE_SHORT_IF(ne, t3, t3, t4)
        ldrne   t0, [t0, t3, lsl $2]
        JUMP2_IF(ne)

        /* We fall through on a NullPointerException */
        b       handleNullPointerException


IMPLEMENTATION(GETFIELD2_FAST):
        /* Optimized for the non-NULL case! */
        cmp     t0, $0

        UPDATE_OPCODE_IF(ne, $3)
        ldrneb  t3, [kvm_ip, $-2]        /* high byte */
        ldrneb  t4,  [kvm_ip, $-1]       /* low byte */
        addne   t0, t0, $INSTANCE_data_OFFSET
        MAKE_SHORT_IF(ne, t3, t3, t4)
        JUMP1_IF(ne)
#if ARM7
        addne   t3, t0, t3, lsl $2
        ldmneia t3, {t1-t2}
        PUSH1_IF(ne, t1)
        movne   t0, t2
#else
        ldrne   t1, [t0, t3, lsl $2] !
        ldrne   t0, [t0, $4]
        PUSH1_IF(ne, t1)
#endif
        JUMP2_IF(ne)

        b       handleNullPointerException

IMPLEMENTATION(PUTFIELD_FAST):
        /* Optimized for the non-NULL case! */
        mov     t2, t0
#if ARM7
        POPM(t0-t1)                     /* t0=tos, t1=object, t2=value */
#else
        /* This is just as fast on the StrongARM, but eliminates the
         * bubble on loading t1
         */
        POP1(t1)
        POP1(t0)
#endif
        cmp     t1, $0
        ldrneb  t3, [kvm_ip, $1]        /* high byte */
        ldrneb  t4, [kvm_ip, $2]        /* low byte */
        UPDATE_OPCODE_IF(ne, $3)
        addne   t1, t1, $INSTANCE_data_OFFSET
        JUMP1_IF(ne)
        MAKE_SHORT_IF(ne, t3, t3, t4)
        strne   t2, [t1, t3, lsl $2]
        JUMP2_IF(ne)

        b       handleNullPointerException

IMPLEMENTATION(PUTFIELD2_FAST):
        /* Optimized for the non-NULL case! */
        mov     s0, t0
        POPM(t0-t2)                     /* t0=tos, t1=object, t2/s0=value */
        cmp     t1, $0
        ldrneb  t3, [kvm_ip, $1]        /* high byte */
        ldrneb  t4, [kvm_ip, $2]        /* low byte */
        UPDATE_OPCODE_IF(ne, $3)
        addne   t1, t1, $INSTANCE_data_OFFSET
        MAKE_SHORT_IF(ne, t3, t3, t4)
        JUMP1_IF(ne)
#if ARM7
        addne   t1, t1, t3, lsl $2
        stmneia t1, {t2, s0}
#else
        strne   t2, [t1, t3, lsl $2] !
        strne   s0, [t1, $4]
#endif
        JUMP2_IF(ne)

        b       handleNullPointerException

IMPLEMENTATION(GETSTATIC_FAST):
IMPLEMENTATION(GETSTATICP_FAST):
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        PUSH1(t0)
        MAKE_SHORT(t3, t3, t4)
        ldr     t1, [kvm_cp, t3, lsl $2] /* field */
        GET_FIELD_ofClass(t2, t1)
	CHECK_CLASS_INITIALIZED_1(t2) 
        UPDATE_OPCODE($3)
        GET_FIELD_staticAddress(t2, t1)
        JUMP1()
        ldr     t0, [t2]
        JUMP2()
	CHECK_CLASS_INITIALIZED_2(t2) 

IMPLEMENTATION(GETSTATIC2_FAST):
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        PUSH1(t0)
        MAKE_SHORT(t3, t3, t4)
        ldr     t1, [kvm_cp, t3, lsl $2] /* field */
        GET_FIELD_ofClass(t2, t1)
	CHECK_CLASS_INITIALIZED_1(t2)
        GET_FIELD_staticAddress(t2, t1)
        UPDATE_OPCODE($3)
#if ARM7
        ldmia   t2, {t2-t3}
        JUMP1()
        PUSH1(t2)
        mov     t0,t3
#else
        ldr     t1, [t2]
        JUMP1()
        ldr     t0, [t2, $4]
        PUSH1(t1)
#endif
        JUMP2()
	CHECK_CLASS_INITIALIZED_2(t2)


IMPLEMENTATION(PUTSTATIC_FAST):
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        MAKE_SHORT(t3, t3, t4)
        ldr     t1, [kvm_cp, t3, lsl $2] /* field */
        GET_FIELD_ofClass(t2, t1)
	CHECK_CLASS_INITIALIZED_1(t2)
        UPDATE_OPCODE($3)
        GET_FIELD_staticAddress(t2, t1)
	JUMP1()
	str	t0, [t2]
	POP1(t0)
	JUMP2()
	CHECK_CLASS_INITIALIZED_2(t2)


IMPLEMENTATION(PUTSTATIC2_FAST):
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        MAKE_SHORT(t3, t3, t4)
        ldr     t1, [kvm_cp, t3, lsl $2] /* field */
        GET_FIELD_ofClass(t2, t1)
	CHECK_CLASS_INITIALIZED_1(t2)
	UPDATE_OPCODE($3)
        GET_FIELD_staticAddress(t2, t1)
	POP1(t1)	
	str	t0, [t2, $4]
	JUMP1()
	str	t1, [t2]    
	POP1(t0)
	JUMP2();
	CHECK_CLASS_INITIALIZED_2(t2)


#if ENABLEFASTBYTECODES
IMPLEMENTATION(INVOKEVIRTUAL_FAST):
        /* iCacheIndex = getUShort(ip + 1);
         * thisICache = GETINLINECACHE(iCacheIndex);
         */
        PUSH1(t0)
        ldr     t1, Addr_InlineCache
        ldrb    t3, [kvm_ip, $1] /* high byte */
        ldrb    t4, [kvm_ip, $2] /* low byte */
        ldr     t1, [t1]        /* InlineCache */
        MAKE_SHORT(t3, t3, t4)  /* cache index */

        /* t0 gets the contents of the nth entry, s0 address of nth entry */
	GET_ICACHE_contents_nthX(t0, s1, t1, t3) 

        GET_METHOD_argCount(t2, t0)  /* argCount = thisMethod->argCount; */
        GET_METHOD_ofClass(t3, t0)   /* defaultClass = thisMethod->ofClass; */

        /* thisObject = *(OBJECT*)(sp-argCount+1);
         * CHECK_NOT_NULL(thisObject);
         * dynamicClass = thisObject->ofClass;
         */
        sub     t2, kvm_sp, t2, lsl $2
        ldr     t1, [t2, $4]    /* thisObject */
        cmp     t1, $0
        beq     handleNullPointerException
        GET_INSTANCE_ofClass(t2, t1) /* dynamic class */

        /* if (dynamicClass == defaultClass) { do call } */
        cmp     t2, t3
        moveq   s1, $3
        beq     callGeneral

        /* t0 = thisMethod
         * t1 = thisObject
         * t2 = dynamicClass
         * s1 = thisIcache
         */

        /*
         * VMSAVE
         *   thisMethod = lookupMethod(dynamicClass, thisMethod->nameTypeKey,
         *                             fp_global->thisMethod->ofClass);
         *    thisICache->contents = (cell*)thisMethod;
         * VMRESTORE
         */
        .extern lookupMethod

        ldr     s0, Addr_GlobalState
        VMSAVE(s0)
        mov     x0, t0          /* save thisMethod */
        mov     x1, t1          /* save thisObject */
        GET_METHOD_nameTypeKey(t1, x0)
        mov     t0, t2          /* dynamic class */
        GET_FRAME_thisMethod(t4, kvm_fp)
        GET_METHOD_ofClass(t2, t4)
        bl      lookupMethod

        cmp     t0, $0
        SET_ICACHE_contents_IF(ne,t0, s1)
        mov     t1, x1          /* thisObject */
        mov     t2, x0          /* original value of thisMethod */
        VMRESTORE(s0)

        /* if new value isn't NULL, then it is the method we want */
        movne   s1, $3
        bne     callGeneral

        /* We have a fatal error of a method not found */
        mov     t0, s1          /* thisIcache */
        .extern fatalIcacheMethodError
        bl      fatalIcacheMethodError
#endif

#if ENABLEFASTBYTECODES
IMPLEMENTATION(INVOKESPECIAL_FAST):
        /* cpIndex = getUShort(ip + 1);
         * thisMethod = (METHOD)(cp->entries[cpIndex].cache);
         * thisObject = *(OBJECT*)(sp-thisMethod->argCount+1);
         * CHECK_NOT_NULL(thisObject)
         */
        PUSH1(t0)
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        MAKE_SHORT(t3, t3, t4)
        ldr     t0, [kvm_cp, t3, lsl $2] /* method */
        GET_METHOD_argCount(t3, t0)
        sub     t3, kvm_sp, t3, lsl $2 /* point just before thisObject */
        ldr     t1, [t3, $4]    /* t1 = thisObject */
        mov     s1, $3          /* s1 = opcode size */
        cmp     t1, $0
        bne     callGeneral
        b       handleNullPointerException
#endif

#if ENABLEFASTBYTECODES
IMPLEMENTATION(INVOKESTATIC_FAST):
        /* unsigned int cpIndex = getUShort(ip + 1);
         * thisMethod = (METHOD)cp->entries[cpIndex].cache;
         * if (!CLASS_INITIALIZED(thisMethod->ofClass)) {
         *    ...
         * }
         * thisObject = (OBJECT)thisMethod->ofClass;
         */
        PUSH1(t0)
        ldrb    t3, [kvm_ip, $1]        /* high byte */
        ldrb    t4, [kvm_ip, $2]        /* low byte */
        MAKE_SHORT(t3, t3, t4)
        ldr     t0, [kvm_cp, t3, lsl $2] /* method */
        GET_METHOD_ofClass(t1, t0)
    	CHECK_CLASS_INITIALIZED_1(t1)
        mov     s1, $3          /* instruction size */
        /* t0 contains the method
         * t1 contains "this", i.e the object to synchronize on
         * s1 contains size
         */
        b       callGeneral
    	CHECK_CLASS_INITIALIZED_2(t1)
#endif


#if ENABLEFASTBYTECODES
IMPLEMENTATION(INVOKEINTERFACE_FAST):
        /* iCacheIndex = getUShort(ip + 1);
         * thisICache = GETINLINECACHE(iCacheIndex);
         * thisMethod = (METHOD)thisICache->contents;
         */
        PUSH1(t0)
        ldr     t1, Addr_InlineCache
        ldrb    t3, [kvm_ip, $1] /* high byte */
        ldrb    t4, [kvm_ip, $2] /* low byte */
        ldr     t1, [t1]        /* InlineCache */
        MAKE_SHORT(t3, t3, t4)  /* cache index */

        /* t0 gets the contents of the nth entry, s0 address of nth entry */
	GET_ICACHE_contents_nthX(t0, s1, t1, t3) 

        ldrb    t2, [kvm_ip, $3]
        GET_METHOD_ofClass(t3, t0)   /* defaultClass = thisMethod->ofClass; */

        /* thisObject = *(OBJECT*)(sp-argCount+1);
         * CHECK_NOT_NULL(thisObject);
         * dynamicClass = thisObject->ofClass;
         */
        sub     t2, kvm_sp, t2, lsl $2
        ldr     t1, [t2, $4]    /* thisObject */
        cmp     t1, $0
        beq     handleNullPointerException
        GET_INSTANCE_ofClass(t2, t1) /* dynamic class */

        /* if (dynamicClass == defaultClass) { do call } */
        cmp     t2, t3
        moveq   s1, $5
        beq     callGeneral

        /* t0 = thisMethod
         * t1 = thisObject
         * t2 = dynamicClass
         * s1 = thisIcache
         */

        /*
         * VMSAVE
         *   thisMethod = lookupMethod(dynamicClass, thisMethod->nameTypeKey,
         *                             fp_global->thisMethod->ofClass);
         *    thisICache->contents = (cell*)thisMethod;
         * VMRESTORE
         */
        .extern lookupMethod
        ldr     s0, Addr_GlobalState
        VMSAVE(s0)
        mov     x0, t0          /* save thisMethod */
        mov     x1, t1          /* save thisObject */
        GET_METHOD_nameTypeKey(t1, x0)
        GET_FRAME_thisMethod(t4, kvm_fp)
        mov     t0, t2          /* dynamic class */
        GET_METHOD_ofClass(t2, t4)
        bl      lookupMethod

        cmp     t0, $0
        SET_ICACHE_contents_IF(ne, t0, s1)
        mov     t1, x1          /* thisObject */
        mov     t2, x0          /* original value of thisMethod */
        VMRESTORE(s0)
        beq     1f              /* method not found */

        GET_METHOD_accessFlags(t5, t0); /* get new method access flags */
        and     t5, t5, $(ACC_PUBLIC | ACC_STATIC)
        cmp     t5, $ACC_PUBLIC
        moveq   s1, $5
        beq     callGeneral

1:
        .extern fatalIcacheMethodError
        /* We could not find the appropriate method */
        mov     t0,  s1
        bl      fatalIcacheMethodError
#endif

#if ENABLEFASTBYTECODES
IMPLEMENTATION(NEW_FAST):
        .extern instantiate

        ldrb    t3, [kvm_ip, $1]
        ldrb    t4, [kvm_ip, $2]
        ldr     s0, Addr_GlobalState
        PUSH1(t0)
        MAKE_SHORT(t3, t3, t4)
        ldr     t0, [kvm_cp, t3, lsl $2] /* cp entry */
	CHECK_CLASS_INITIALIZED_1(t0)

        VMSAVE(s0)
        bl      instantiate
        VMRESTORE(s0)

        cmp     t0, $0
        UPDATE_OPCODE_IF(ne, $3)
	POP1_IF(eq, t0)     /* prime top of stack before reschedule */
        beq     reschedule

        /* 99% of the time "new" is followed by "dup" */
        cmp     t5, $OP_DUP
        UPDATE_OPCODE_IF(eq, $1)
        PUSH1_IF(eq, t0)
        JUMP()
    	CHECK_CLASS_INITIALIZED_2(t0)
#endif


#if ENABLEFASTBYTECODES
IMPLEMENTATION(ANEWARRAY_FAST):
        mov     t1, t0          /* Length */
        ldr     t5, Addr_InlineCache
        ldrb    t3, [kvm_ip, $1] /* high byte */
        ldrb    t4, [kvm_ip, $2] /* low byte */
        ldr     t5, [t5]         /* InlineCache */
        MAKE_SHORT(t3, t3, t4)   /* cache index */
        ldr     s0, Addr_GlobalState
        GET_ICACHE_contents_nth(t0, t5, t3)

        VMSAVE(s0)
        bl      instantiateArray        /* instantiateArray(class, length) */
        VMRESTORE(s0)

        cmp     t0, $0
        UPDATE_OPCODE_IF(ne, $3)
	POP1_IF(eq, t0)		/* prime top of stack before resched */
	JUMP_IF(ne)
        beq    reschedule
#endif

IMPLEMENTATION(MULTIANEWARRAY_FAST):
        .extern instantiateMultiArray
        ldrb    t3, [kvm_ip, $1] /* high byte */
        ldrb    t4, [kvm_ip, $2] /* low byte */
        PUSH1(t0)
        ldrb    t2, [kvm_ip, $3] /* dimension */
        MAKE_SHORT(t3, t3, t4)   /* cp index */
        ldr     t0, [kvm_cp, t3, lsl $2] /* array class */
        sub     s1, kvm_sp, t2, lsl $2 /* just below dimension args */
        ldr     s0, Addr_GlobalState
        add     t1, s1, $4

        VMSAVE(s0)
        bl      instantiateMultiArray
        VMRESTORE(s0)

        cmp     t0, $0
        UPDATE_OPCODE_IF(ne, $4)
	POP1_IF(eq, t0)		/* prime top of stack before resched */
        JUMP1_IF(ne)
        movne    kvm_sp, s1      /* saved new stack value */
        JUMP2_IF(ne)
        beq      reschedule

IMPLEMENTATION(CHECKCAST_FAST):
        .extern isAssignableTo
        .extern isAssignableToFast
        cmp     t0, $0
        UPDATE_OPCODE_IF(eq, $3)             /* object == NULL, return */
        JUMP_IF(eq)

        PUSH1(t0)

        GET_INSTANCE_ofClass(s0, t0)   /* s0 = object->ofClass */
        ldrb    t3, [kvm_ip, $1]
        ldrb    t4, [kvm_ip, $2]
        MAKE_SHORT(t1, t3, t4)      /* t1 = cpIndex */
        ldr     s1, [kvm_cp, t1, lsl $2] /* s1 = class */

        mov     t0, s0
        mov     t1, s1
        bl      isAssignableToFast
        cmp     t0, $0
        UPDATE_OPCODE_IF(ne, $3)
        POP1_IF(ne, t0)
        JUMP_IF(ne)

        mov     t0, s0
        ldr     s0, Addr_GlobalState
        mov     t1, s1
        VMSAVE(s0)
        bl      isAssignableTo
        cmp     t0, $0
        VMRESTORE(s0)

        UPDATE_OPCODE_IF(ne, $3)
        POP1_IF(ne, t0)
        JUMP_IF(ne)

        ldr     t0, Addr_ClassCastException
        b       handleException


IMPLEMENTATION(INSTANCEOF_FAST):
        .extern isAssignableToFast
        .extern isAssignableTo

        cmp     t0, $0            /* Is the object NULL? */
        UPDATE_OPCODE_IF(eq, $3)
        JUMP_IF(eq)               /* Null looks like FALSE */

        GET_INSTANCE_ofClass(s0, t0)
        ldrb    t3, [kvm_ip, $1]  /* high byte */
        ldrb    t4, [kvm_ip, $2]  /* low byte */
        MAKE_SHORT(t1, t3, t4)    /* cp index */
        ldr     s1, [kvm_cp, t1, lsl $2] /* cp entry */

        mov     t0, s0
        mov     t1, s1
        bl      isAssignableToFast

        /* We expect that isAssignableToFast will take care of most
         * of the cases. 
         */
        cmp     t0, $0
        UPDATE_OPCODE_IF(ne, $3)
        movne     t0, $1
        JUMP_IF(ne)

1:      mov     t0, s0
        ldr     s0, Addr_GlobalState
        mov     t1, s1
        VMSAVE(s0)
        bl      isAssignableTo
        VMRESTORE(s0)

        /* Turn all non-zero values into '1' */
        UPDATE_OPCODE($3)
        cmp     t0, $0
        JUMP1()
        movne   t0, $1
        JUMP2()


IMPLEMENTATION(CUSTOMCODE):
        /* cell *stack = (cell*)(fp + 1);
         * CustomCodeCallbackFunction func =
         *      *(CustomCodeCallbackFunction *)stack;
         * VMSAVE
         *    func(NULL);
         * VMRESTORE
         */
         /* Push top of stack back onto stack */
         PUSH1(t0)

        ldr     s0, Addr_GlobalState
        mov     t0, $0
        ldr     t1, [kvm_fp, $SIZEOF_FRAME_bytes]   /* Function to call */

        VMSAVE(s0)
        GET_LOCAL_ADDRESS(lr,restoreAndReschedule_s0)
#if INTERWORK
	bx	t1
#else
        mov     pc, t1
#endif

	.word	IMPLEMENTATION(wide_FLOAD)
IMPLEMENTATION(FLOAD):
IMPLEMENTATION(DLOAD):
IMPLEMENTATION(FSTORE):
IMPLEMENTATION(DSTORE):
	/* Fall through */


#if !ENABLEFASTBYTECODES
IMPLEMENTATION(INVOKEVIRTUAL_FAST):
IMPLEMENTATION(INVOKESPECIAL_FAST):
IMPLEMENTATION(INVOKESTATIC_FAST):
IMPLEMENTATION(INVOKEINTERFACE_FAST):
IMPLEMENTATION(NEW_FAST):
IMPLEMENTATION(ANEWARRAY_FAST):
#endif

        /* These are the actual "slow" bytecodes */
IMPLEMENTATION(GETSTATIC):
IMPLEMENTATION(PUTSTATIC):
IMPLEMENTATION(GETFIELD):
IMPLEMENTATION(PUTFIELD):
IMPLEMENTATION(INVOKEVIRTUAL):
IMPLEMENTATION(INVOKESPECIAL):
IMPLEMENTATION(INVOKESTATIC):
IMPLEMENTATION(INVOKEINTERFACE):
IMPLEMENTATION(NEW):
IMPLEMENTATION(ANEWARRAY):
IMPLEMENTATION(CHECKCAST):
IMPLEMENTATION(INSTANCEOF):
/* IMPLEMENTATION(WIDE): */
IMPLEMENTATION(MULTIANEWARRAY):
       /* These are illegal, but can be handled by the SlowInterpreter */
IMPLEMENTATION(JSR):
IMPLEMENTATION(RET):
IMPLEMENTATION(UNUSED_BA):
IMPLEMENTATION(JSR_W):
IMPLEMENTATION(UNUSED_D5):
IMPLEMENTATION(FCONST_0):
IMPLEMENTATION(FCONST_1):
IMPLEMENTATION(FCONST_2):
IMPLEMENTATION(DCONST_0):
IMPLEMENTATION(DCONST_1):
IMPLEMENTATION(FLOAD_0):
IMPLEMENTATION(FLOAD_1):
IMPLEMENTATION(FLOAD_2):
IMPLEMENTATION(FLOAD_3):
IMPLEMENTATION(DLOAD_0):
IMPLEMENTATION(DLOAD_1):
IMPLEMENTATION(DLOAD_2):
IMPLEMENTATION(DLOAD_3):
IMPLEMENTATION(FALOAD):
IMPLEMENTATION(DALOAD):
IMPLEMENTATION(FSTORE_0):
IMPLEMENTATION(FSTORE_1):
IMPLEMENTATION(FSTORE_2):
IMPLEMENTATION(FSTORE_3):
IMPLEMENTATION(DSTORE_0):
IMPLEMENTATION(DSTORE_1):
IMPLEMENTATION(DSTORE_2):
IMPLEMENTATION(DSTORE_3):
IMPLEMENTATION(FASTORE):
IMPLEMENTATION(DASTORE):
IMPLEMENTATION(FADD):
IMPLEMENTATION(DADD):
IMPLEMENTATION(FSUB):
IMPLEMENTATION(DSUB):
IMPLEMENTATION(FMUL):
IMPLEMENTATION(DMUL):
IMPLEMENTATION(FDIV):
IMPLEMENTATION(DDIV):
IMPLEMENTATION(FREM):
IMPLEMENTATION(DREM):
IMPLEMENTATION(FNEG):
IMPLEMENTATION(DNEG):
IMPLEMENTATION(I2F):
IMPLEMENTATION(I2D):
IMPLEMENTATION(L2F):
IMPLEMENTATION(L2D):
IMPLEMENTATION(F2I):
IMPLEMENTATION(F2L):
IMPLEMENTATION(F2D):
IMPLEMENTATION(D2I):
IMPLEMENTATION(D2L):
IMPLEMENTATION(D2F):
IMPLEMENTATION(FCMPL):
IMPLEMENTATION(FCMPG):
IMPLEMENTATION(DCMPL):
IMPLEMENTATION(DCMPG):
IMPLEMENTATION(FRETURN):
IMPLEMENTATION(DRETURN):

IMPLEMENTATION(wide_FLOAD):
IMPLEMENTATION(wide_DLOAD):
IMPLEMENTATION(wide_FSTORE):
IMPLEMENTATION(wide_DSTORE):

illegal:
        .extern SlowInterpret
        PUSH1(t0)
        ldrb    t0, [kvm_ip]
        CALL_SAVED_AND_RESCHEDULE(SlowInterpret)


callGeneral:
        /* t0 contains the method
         * t1 contains "this"
         * s1 contains size
         */
        GET_METHOD_accessFlags(t3, t0)
        tst     t3, $(ACC_NATIVE | ACC_ABSTRACT)
        bne     callNativeAbstract

        mov     jt, t0          /* jt = thisMethod */
        mov     s0, t1          /* s0 = object. */

        GET_METHOD_frameSize(t1, jt)  /* t1 = thisMethod->frameSize */
        GET_METHOD_argCount(t2, jt)   /* t2 = thisMethod->argCount */
        GET_FRAME_stack(t3, kvm_fp)   /* t3 = stack (current Stack) */
        GET_METHOD_maxStack(t4, jt)   /* t4 = thisMethod->u.java.maxStack */

        /* thisLocalCount = thisFrameSize - thisArgCount */
        /* thisMethodHeight = thisLocalCount + thisMethod->u.java.maxStack + C*/
        add     t4, t4, t1
        sub     t4, t4, t2
        add     t4, t4, $(SIZEOF_FRAME_cells + RESERVEDFORNATIVE)
                                     /* t4 = thisMethodHeight */

        GET_STACK_size(t5, t3)       /* t5 = stack->size */

        /* if (getSP() - stack->cells + thisMethodHeight >= stack->size) */
        sub     t0, kvm_sp, t3       /* sp - stack */
        sub     t0, t0, $STACK_cells_OFFSET

        add     t0, t0, t4, lsl $2
        cmp     t0, t5, lsl $2

        bge     hardPushFrame

        sub     t0, kvm_sp, t2, lsl $2  /* t0 = prev_sp */
        add     kvm_lp, t0, $4          /* lp = prev_sp + 1 */

#if FRAME_previousFp_OFFSET  == 0
        str     kvm_fp, [kvm_lp, t1, lsl $2]
        add     kvm_fp, kvm_lp, t1, lsl $2
#else 
        add     t1, kvm_lp, t1, lsl $2  /* t1 = newFrame = lp + frameSize */
        SET_FRAME_previousFp(kvm_fp, t1)
	mov	kvm_fp, t1
#endif
        add     kvm_ip, kvm_ip, s1      /* fp->previousIP += invokerSize */
        SET_FRAME_previousIp(kvm_ip, kvm_fp)
        SET_FRAME_previousSp(t0,     kvm_fp)
        SET_FRAME_stack(     t3,     kvm_fp)
        SET_FRAME_thisMethod(jt,     kvm_fp)

        GET_METHOD_ofClass(t1, jt)
        GET_METHOD_javaCode(kvm_ip, jt)
        GET_INSTANCE_CLASS_constPool(kvm_cp, t1)

pushFrameContinue:
        /*
         * if (thisMethod->accessFlags & ACC_SYNCHRONIZED) {
         *     VMSAVE
         *         monitorEnter(thisObjectGCSafe);
         *     VMRESTORE
         *     fp->syncObject = thisObjectGCSafe;
         * }
         * thisObjectGCSafe = NULL;
         * reschedule;
         */

        /* jt contains the method, 
         * s0 contains the object to synchronize on */
        GET_METHOD_accessFlags(t0, jt)
        adr     jt, JumpTable       /* restore the jumptable */ 
        tst     t0, $ACC_SYNCHRONIZED

        mov     t0, $0
        SET_FRAME_syncObject(t0, kvm_fp)

        /* Set the stack height to be empty, with the top of stack
         * (really, the last element of the FRAME) in t0
         */
        add     kvm_sp, kvm_fp, $(SIZEOF_FRAME_bytes - 8)	
#if FRAME_syncObject_OFFSET != SIZEOF_FRAME_bytes - 4
        GET_IF(eq, t0)
#else
        /* t0 is already zero, so we don't need to get the value */
#endif
        beq       reschedule

        /* Try to inline monitorEnter in the most common case */
        GET_INSTANCE_monitor(t1, s0)
        ldr     t2, Addr_CurrentThread
        cmp     t1, $0
        ldreq   t2, [t2]
        SET_FRAME_syncObject_IF(eq, s0, kvm_fp)
        addeq   t2, t2, $MHC_SIMPLE_LOCK
        SET_INSTANCE_monitor_IF(eq, t2, s0)
        beq     reschedule

        /* We need to synchronize on the object in s0 */
        .extern monitorEnter
        add     kvm_sp, kvm_sp, $4     /* Make the stack look normal */
        mov     t0, s0
        ldr     s1, Addr_thisObjectGCSafe
        ldr     s0, Addr_GlobalState
        str     t0, [s1]        /* GC protect the object */

        VMSAVE(s0)
        bl      monitorEnter
        VMRESTORE(s0)

        ldr     t0, [s1]        /* t0 = this Object */
        mov     t1, $0          /* thisObjectGCSafe = 0 */
        str     t1, [s1]

        SET_FRAME_syncObject(t0, kvm_fp)

#if FRAME_syncObject_OFFSET == SIZEOF_FRAME_bytes - 4
        sub   kvm_sp, kvm_sp, $4
#else
        POP1(t0)
#endif
        b       reschedule

hardPushFrame:
        .extern pushFrame

        /* jt contains the method.
         * s1 contains the opcode size
         * s0 contains "this"
         */
        ldr     t1, Addr_GlobalState
        mov     t0, jt          /* argument to pushFrame */
        VMSAVE(t1)
        ldr     x0, Addr_thisObjectGCSafe
        mov     x1, t1          /* GlobalState */
        str     s0, [x0]        /* Protect around call to pushFrame */
        bl      pushFrame       /* pushFrame(thisMethod) */

#if ARM7
        mov     s0, $0
        swp     s0, s0, [x0]    /* Restore s0, thisObjectGCSafe = NULL */
#else
        /* swp is very slow on the StrongArm.  Better not to use it */
        mov     t1, $0
        ldr     s0, [x0]
        str     t1, [x0]
#endif
        cmp     t0, $0
        VMRESTORE(x1)           /* This overwrites x1 */

        GET_FRAME_previousIp_IF(ne, t0, kvm_fp)
        addne   t0, t0, s1
        SET_FRAME_previousIp_IF(ne, t0, kvm_fp)

        /* Continue back at pushFrameContinue */
        GET_METHOD_accessFlags_IF(ne, t3, jt)
        bne     pushFrameContinue

        /* PushFrame has failed.  We just need to reschedule, since
         * stack overflow has already been throw
         */
        adr     jt, JumpTable
        POP1(t0)
        beq     reschedule

callNativeAbstract:
        .extern invokeNativeFunction
        .extern fatalError

        /* t0 contains the method
         * t1 contains "this"
         * s1 contains size
         * t3 contains the flags
         */
        tst     t3, $ACC_ABSTRACT
        addeq   kvm_ip, s1, kvm_ip
        CALL_SAVED_AND_RESCHEDULE_IF(eq, invokeNativeFunction)

        adr     t0, Msg_Abstract
        bl      fatalError      /* Does not return */


handleNullPointerException:
        ldr     t0, Addr_NullPointerException
        b       handleException

handleArrayIndexOutOfBoundsException:
        ldr     t0, Addr_ArrayIndexOutOfBoundsException
        b       handleException

handleDivideByZeroException:
        ldr     t0, Addr_ArithmeticException
        b       handleException

handleArrayStoreException:
        ldr     t0, Addr_ArrayStoreException
        b       handleException

handleException:
        .extern raiseException
        CALL_SAVED_AND_RESCHEDULE(raiseException)


Msg_BreakPoint:
       .asciz KVM_MSG_BREAKPOINT


Msg_Abstract:
       .asciz KVM_MSG_ABSTRACT_METHOD_INVOKED

Msg_IllegalWide:
       .asciz KVM_MSG_ILLEGAL_WIDE_BYTECODE_EXTENSION

JumpTable:
#define MAKE_JUMP_TABLE_ENTRY(name, value)    .word IMPLEMENTATION(name);
#define MAKE_ILLEGAL_TABLE_ENTRY(name, value) .word illegal;
FOR_EACH_NORMAL_BYTE_CODE(MAKE_JUMP_TABLE_ENTRY)
FOR_EACH_EXTRA_BYTE_CODE(MAKE_ILLEGAL_TABLE_ENTRY)

#if MASM
        END
#else
 .Lfe1:
        .size FastInterpret, .Lfe1 - FastInterpret
#endif



