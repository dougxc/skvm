; Copyright 2000-2001 by Sun Microsystems, Inc.,
; 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
; All rights reserved.
; This software is the confidential and proprietary information
; of Sun Microsystems, Inc. ("Confidential Information").  You
; shall not disclose such Confidential Information and shall use
; it only in accordance with the terms of the license agreement
; you entered into with Sun.
;
; File: winFastInterpret.asm
; Description: intel assembly interpreter loop
; Author: Kay Neuenhofen
    
.386
.model flat

; ----------------------------------------------------------------------------
;                         Extern functions and variables
; ----------------------------------------------------------------------------
extern _AliveThreadCount :dword
extern __chkesp :proc
extern _checkTimerQueue :proc
extern _CurrentThread :dword
extern _initializeClass :proc
extern _InlineCache :dword
extern _instantiate :proc
extern _instantiateArray :proc
extern _instantiateMultiArray :proc
extern _InterpreterHandleEvent :proc
extern _invokeNativeFunction :proc
extern _isAssignableTo :proc
extern _isAssignableToFast :proc
extern _fatalError :proc
extern _fatalIcacheMethodError :proc
extern _GlobalState :dword
extern _ll_div :proc
extern _ll_rem :proc
extern _lookupMethod :proc
extern _monitorEnter :proc
extern _monitorExit :proc
extern _popFrame :proc
extern _pushFrame :proc
extern _raiseException :proc
extern _resolveClassReference :proc
extern _SlowInterpret :proc
extern _stopThread :proc
extern _SwitchThread :proc
extern _thisObjectGCSafe :dword
extern _Timeslice :dword
extern _throwException :proc

; ----------------------------------------------------------------------------
;                                 Defines
; ----------------------------------------------------------------------------

; vm registers
java_ip equ esi 
java_sp equ edi
java_lp equ ebp

; opcodes
OP_IINC equ 084h
OP_ILOAD equ 015h
OP_ISTORE equ 036h
OP_LLOAD equ 016h
OP_LSTORE equ 037h
OP_ALOAD equ 019h
OP_ASTORE equ 03ah
OP_RETURN equ 0b1h

include defines.asm

; ----------------------------------------------------------------------------
;                                 Macros
; ----------------------------------------------------------------------------
VMSAVE  macro
    mov     dword ptr [_GlobalState + IP_OFFSET], java_ip
    mov     dword ptr [_GlobalState + SP_OFFSET], java_sp
    mov     dword ptr [_GlobalState + LP_OFFSET], java_lp
endm

VMRESTORE macro
    mov     java_ip, dword ptr [_GlobalState + IP_OFFSET]
    mov     java_sp, dword ptr [_GlobalState + SP_OFFSET]
    mov     java_lp, dword ptr [_GlobalState + LP_OFFSET]
endm

CALL_SLOW_INTERPRET macro  
    VMSAVE                      
    push    eax             
    call    _SlowInterpret  
    add     esp,4           
    VMRESTORE                   
    jmp     reschedulePoint 
endm
    
NEXT_BYTECODE macro
    ;jmp        next0
    movzx   eax, byte ptr [java_ip]             ; eax = current bytecode
    jmp     [Jumptable + 4 * eax]               ; dispatch to matching bytecode handler
endm

RESCHEDULE macro
    jmp     reschedulePoint
endm

; 386 has no bswap instruction :(
BYTE_SWAP macro reg
    xchg    &reg&l, &reg&h
    ror     e&reg&x, 16
    xchg    &reg&l, &reg&h
endm

IFCOND macro opcode, cc
    mov     eax, dword ptr [java_sp]    ; get value
    sub     java_sp, 4                  ; java_sp--
    test    eax, eax                    ; set condition flags for value
    j&cc    &opcode&_nobranch           ; don't branch if condition fails
    mov     ax, word ptr [java_ip + 1]
    xchg    ah, al
    movsx   eax, ax
    add     java_ip, eax                ; add ((branchbyte1 << 8) | branchbyte) to java_ip
    RESCHEDULE
&opcode&_nobranch:
    add     java_ip, 3                  ; advance to next byte code
    NEXT_BYTECODE
endm

IFICMPCOND macro opcode, cc
    mov     ebx, dword ptr [java_sp]
    mov     eax, dword ptr [java_sp - 4]
    sub     java_sp, 8
    cmp     eax, ebx
    j&cc    &opcode&_nobranch
    mov     ax, word ptr [java_ip + 1]
    xchg    ah, al
    movsx   eax, ax
    add     java_ip, eax
    RESCHEDULE
&opcode&_nobranch:
    add     java_ip, 3
    NEXT_BYTECODE
endm

PUSH_INT_CONST macro value
    add     java_sp, 4
    inc     java_ip
    mov     dword ptr [java_sp], value
    NEXT_BYTECODE
endm

PUSH_LONG_CONST macro value
    add     java_sp, 8
    inc     java_ip
    xor     eax, eax
    mov     dword ptr [java_sp - 0], eax
    mov     dword ptr [java_sp - 4], value
    NEXT_BYTECODE
endm

ILOAD_CONST macro index
    add     java_sp, 4
    inc     java_ip
    mov     eax, dword ptr [java_lp + index]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
endm

ILOAD macro
    add     java_sp, 4                              ; sp++
    movzx   eax, byte ptr [java_ip + 1]             ; eax = index
    add     java_ip, 2                              ; advance to next bytecode
    mov     eax, dword ptr [java_lp + 4 * eax]      ; eax = local_var[index]
    mov     dword ptr [java_sp], eax                ; push local_var[index]
    NEXT_BYTECODE
endm

LLOAD_CONST macro index1, index2
    add     java_sp, 8
    inc     java_ip
    mov     eax, dword ptr [java_lp + index1]
    mov     dword ptr [java_sp - 4], eax
    mov     eax, dword ptr [java_lp + index2]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
endm

LLOAD macro
    add     java_sp, 8
    movzx   eax, byte ptr [java_ip + 1]
    add     java_ip, 2
    mov     ebx, dword ptr [java_lp + 4 * eax]
    mov     dword ptr [java_sp - 4], ebx
    inc     eax
    mov     eax, dword ptr [java_lp + 4 * eax]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
endm

ISTORE_CONST macro index
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    mov     dword ptr [java_lp + index], eax
    NEXT_BYTECODE
endm

ISTORE macro
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    movzx   ecx, byte ptr [java_ip + 1]
    add     java_ip, 2
    mov     dword ptr [java_lp + 4 * ecx], eax
    NEXT_BYTECODE
endm

LSTORE_CONST macro index1, index2
    inc     java_ip
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 8
    mov     dword ptr [java_lp + index2], eax
    mov     eax, dword ptr [java_sp + 4]
    mov     dword ptr [java_lp + index1], eax
    NEXT_BYTECODE
endm

LSTORE macro
    mov     eax, dword ptr [java_sp - 4]
    sub     java_sp, 8
    movzx   ecx, byte ptr [java_ip + 1]
    add     java_ip, 2
    mov     dword ptr [java_lp + 4 * ecx], eax
    inc     ecx
    mov     eax, dword ptr [java_sp + 8]
    mov     dword ptr [java_lp + 4 * ecx], eax
    NEXT_BYTECODE
endm

LONG_DIV macro routine
    mov     eax, dword ptr [java_sp - 0]
    or      eax, dword ptr [java_sp - 4]
    je      handleArithmeticException
    inc     java_ip
    push    dword ptr [java_sp -  0]
    push    dword ptr [java_sp -  4]
    push    dword ptr [java_sp -  8]
    push    dword ptr [java_sp - 12]
    sub     java_sp, 8
    call    routine
    add     esp, 16
    mov     dword ptr [java_sp -  4], eax
    mov     dword ptr [java_sp -  0], edx
    NEXT_BYTECODE
endm

INT_SHIFT macro opcode
    mov     cl, byte ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    opcode  dword ptr [java_sp], cl
    NEXT_BYTECODE
endm

INT_BOOL_OP macro opcode
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    opcode  dword ptr [java_sp], eax
    NEXT_BYTECODE
endm
    
LONG_BOOL_OP macro opcode
    sub     java_sp, 8
    inc     java_ip
    mov     eax, dword ptr [java_sp + 8]
    opcode  dword ptr [java_sp + 0], eax
    mov     eax, dword ptr [java_sp + 4]
    opcode  dword ptr [java_sp - 4], eax
    NEXT_BYTECODE
endm


ARRAY_LOAD_SETUP macro opcode
    mov     eax, dword ptr [java_sp - 4]                
    test    eax, eax                                    
    je      handleNullPointerException  
    mov     ebx, dword ptr [java_sp]        
    cmp     ebx, dword ptr [eax + ARRAY_LENGTH_OFFSET]
    jb      &OPCODE&_ok
    jmp     handleArrayIndexOutOfBoundsException    
&OPCODE&_ok:    
    inc     java_ip                                     
endm

ARRAY_LOAD_WRAP macro 
    mov     dword ptr [java_sp], eax                    
    NEXT_BYTECODE
endm
    
ARRAY_STORE_SETUP macro opcode
    mov     eax, dword ptr [java_sp - 8]                
    test    eax, eax                                    
    je      handleNullPointerException  
    mov     ebx, dword ptr [java_sp - 4]        
    cmp     ebx, dword ptr [eax + ARRAY_LENGTH_OFFSET]
    jb      &opcode&_ok
    jmp     handleArrayIndexOutOfBoundsException    
&opcode&_ok:    
    inc     java_ip     
    mov     ecx, dword ptr [java_sp]
endm

ARRAY_STORE_WRAP macro
    sub     java_sp, 12
    NEXT_BYTECODE
endm

STATIC_SETUP macro opcode, offset
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = cpIndex
    mov     eax, dword ptr [_GlobalState + CP_OFFSET]   ; eax = cp
    mov     eax, dword ptr [eax + 4 * ebx]              ; eax = field/method
    mov     ebx, dword ptr [eax + offset]               ; ebx = field/method->ofClass
    mov     ecx, dword ptr [ebx + CLASS_STATUS_OFFSET]  ; ecx = class->status
    cmp     ecx, CLASS_READY
    je      &opcode&_initialized
    mov     ecx, dword ptr [ebx + CLASS_THREAD_OFFSET]  ; ecx = class->initThread
    cmp     ecx, dword ptr [_CurrentThread]
    je      &opcode&_initialized
    VMSAVE
    push    ebx
    call    _initializeClass
    add     esp, 4
    VMRESTORE
    RESCHEDULE
&opcode&_initialized:
endm

; ----------------------------------------------------------------------------
;                         Local variables and constants
; ----------------------------------------------------------------------------
.data
wakeupTime  qword 0
exitError   dd 0    

breakpoint_str db "breakpoint", 0ah, 0
invalid_timeslice_str db "invalid timeslice", 0ah, 0
abstract_method_str db "Abstract method invoked", 0ah, 0

nullpointer_exception db "java/lang/NullPointerException", 0
arrayindexoutofbounds_exception db "java/lang/ArrayIndexOutOfBoundsException", 0
arithmetic_exception db "java/lang/ArithmeticException", 0
arraystore_exception db "java/lang/ArrayStoreException", 0
classcast_exception db "java/lang/ClassCastException", 0

; ----------------------------------------------------------------------------
;                           Interpreter loop
; ----------------------------------------------------------------------------
.code

_FastInterpret proc
    push ebp                                    ; save ebp, since we're using it as a vm register
    VMRESTORE                                   ; copy vm registers to local registers
    jmp     next0                               ; start executing bytecodes

reschedulePoint:
    mov     eax, dword ptr [_Timeslice]         ; eax = Timeslice
    test    eax, eax                            ; eax == 0?
    je      time_to_reschedule                  ; yes, reschedule
    dec     eax                                 ; no, decrement eax
    mov     dword ptr [_Timeslice], eax         ; Timeslice = eax
    jne     next0                               ; execute next bytecode
time_to_reschedule:
    VMSAVE                                      ; save local vm registers
reschedule_loop:
    cmp     dword ptr [_AliveThreadCount], 0    ; live thread count > 0?
    jg      Reschedule_L01                      ; yes, keep going
    jmp     Interpreter_Done                    ; no, we're done here
Reschedule_L01:
    mov     edx, offset wakeupTime
    push    edx
    call    _checkTimerQueue                    ; call checkTimerQueue(&wakepTime)
    add     esp, 4
    
    mov     eax,dword ptr [wakeupTime + 4]      ; push wakeupTime.hi onto stack
    push    eax
    mov     eax,dword ptr [wakeupTime]          ; push wakeupTime.lo onto stack
    push    eax
    call    _InterpreterHandleEvent             ; call InterpretrHandleEvent(wakeupTime)
    add     esp, 8
    
    call    _SwitchThread                       ; call SwitchThread()
    test    eax, eax                            ; result == true?
    je      reschedule_loop                     ; no, keep looping
    VMRESTORE                                   ; yes, continue bytecode execution

next0:
    movzx   eax, byte ptr [java_ip]             ; eax = current bytecode
    jmp     [Jumptable + 4 * eax]               ; dispatch to matching bytecode handler

; ----------------------------------------------------------------------------
;                           Bytecode implementations
; ----------------------------------------------------------------------------

; -------------------------------- nop - 0x00 --------------------------------
asm_nop:
    inc     java_ip             
    NEXT_BYTECODE

; --------------------------- aconst_null - 0x01 -----------------------------
asm_aconst_null:
    PUSH_INT_CONST 0

; ---------------------------- iconst_m1 - 0x02 ------------------------------
asm_iconst_m1:
    PUSH_INT_CONST -1

; ----------------------------- iconst_0 - 0x03 ------------------------------
asm_iconst_0:
    PUSH_INT_CONST 0

; ----------------------------- iconst_1 - 0x04 ------------------------------
asm_iconst_1:
    PUSH_INT_CONST 1

; ----------------------------- iconst_2 - 0x05 ------------------------------
asm_iconst_2:
    PUSH_INT_CONST 2

; ----------------------------- iconst_3 - 0x06 ------------------------------
asm_iconst_3:
    PUSH_INT_CONST 3

; ----------------------------- iconst_4 - 0x07 ------------------------------
asm_iconst_4:
    PUSH_INT_CONST 4

; ----------------------------- iconst_5 - 0x08 ------------------------------
asm_iconst_5:
    PUSH_INT_CONST 5
    
; ----------------------------- lconst_0 - 0x09 ------------------------------
asm_lconst_0:
    PUSH_LONG_CONST 0
    
; ----------------------------- lconst_0 - 0x0a ------------------------------
asm_lconst_1:
    PUSH_LONG_CONST 1
    
; ------------------------------ bipush - 0x10 -------------------------------
asm_bipush:
    add     java_sp, 4
    movsx   eax, byte ptr [java_ip + 1]
    add     java_ip, 2
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------ sipush - 0x11 -------------------------------
asm_sipush:
    add     java_sp, 4
    mov     ax, word ptr [java_ip + 1]
    add     java_ip, 3
    xchg    al, ah
    movsx   eax, ax
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE

; -------------------------------- ldc - 0x12 --------------------------------
asm_ldc:
    movzx   eax, byte ptr [java_ip + 1]
    add     java_ip, 2
    add     java_sp, 4
    mov     ebx, dword ptr [_GlobalState + CP_OFFSET]
    mov     ebx, dword ptr [ebx + eax * CP_ENTRY_SIZE]
    mov     dword ptr [java_sp], ebx
    NEXT_BYTECODE

; ------------------------------- ldc_w - 0x13 -------------------------------
asm_ldc_w:
    xor     eax, eax
    mov     ax, word ptr [java_ip + 1]
    xchg    ah, al
    add     java_ip, 3
    add     java_sp, 4
    mov     ebx, dword ptr [_GlobalState + CP_OFFSET]
    mov     ebx, dword ptr [ebx + eax * CP_ENTRY_SIZE]
    mov     dword ptr [java_sp], ebx
    NEXT_BYTECODE

; ------------------------------- ldc2_w - 0x14 ------------------------------
asm_ldc2_w:
    xor     eax, eax
    mov     ax, word ptr [java_ip + 1]
    xchg    ah, al
    add     java_ip, 3
    add     java_sp, 8
    mov     ecx, dword ptr [_GlobalState + CP_OFFSET]
    mov     ebx, dword ptr [ecx + eax * CP_ENTRY_SIZE]
    mov     dword ptr [java_sp], ebx
    mov     ebx, dword ptr [ecx + eax * CP_ENTRY_SIZE + CP_ENTRY_SIZE]
    mov     dword ptr [java_sp - 4], ebx
    NEXT_BYTECODE
    
; ------------------------------ iload - 0x15 --------------------------------
; ------------------------------ aload - 0x19 --------------------------------
asm_iload:
asm_aload:
    ILOAD
    
; ------------------------------ lload - 0x16 --------------------------------
asm_lload:
    LLOAD
    
; ----------------------------- iload_0 - 0x1a -------------------------------
asm_iload_0:
    ILOAD_CONST 0
    
; ----------------------------- iload_1 - 0x1b -------------------------------
asm_iload_1:
    ILOAD_CONST 4

; ----------------------------- iload_2 - 0x1c -------------------------------
asm_iload_2:
    ILOAD_CONST 8

; ----------------------------- iload_3 - 0x1d -------------------------------
asm_iload_3:
    ILOAD_CONST 12
    
; ----------------------------- lload_0 - 0x1e -------------------------------
asm_lload_0:
    LLOAD_CONST 0, 4
    
; ----------------------------- lload_1 - 0x1f -------------------------------
asm_lload_1:
    LLOAD_CONST 4, 8

; ----------------------------- lload_2 - 0x20 -------------------------------
asm_lload_2:
    LLOAD_CONST 8, 12

; ----------------------------- lload_3 - 0x21 -------------------------------
asm_lload_3:
    LLOAD_CONST 12, 16
    
; ----------------------------- aload_0 - 0x2a -------------------------------
asm_aload_0:
    ILOAD_CONST 0
    
; ----------------------------- aload_1 - 0x2b -------------------------------
asm_aload_1:
    ILOAD_CONST 4

; ----------------------------- aload_2 - 0x2c -------------------------------
asm_aload_2:
    ILOAD_CONST 8

; ----------------------------- aload_3 - 0x2d -------------------------------
asm_aload_3:
    ILOAD_CONST 12
    
; ------------------------------ iaload - 0x2e -------------------------------
asm_iaload:
    ARRAY_LOAD_SETUP iaload
    sub     java_sp, 4
    mov     eax, dword ptr [eax + 4 * ebx + ARRAY_DATA_OFFSET]  
    ARRAY_LOAD_WRAP
    
; ------------------------------ laload - 0x2f -------------------------------
asm_laload:
    ARRAY_LOAD_SETUP laload
    mov     ecx, dword ptr [eax + 8 * ebx + ARRAY_DATA_OFFSET]  
    mov     dword ptr [java_sp - 4], ecx    
    mov     eax, dword ptr [eax + 8 * ebx + ARRAY_DATA_OFFSET_LONG]  
    ARRAY_LOAD_WRAP
    
; ------------------------------ aaload - 0x32 -------------------------------
asm_aaload:
    ARRAY_LOAD_SETUP aaload
    sub     java_sp, 4
    mov     eax, dword ptr [eax + 4 * ebx + ARRAY_DATA_OFFSET]  
    ARRAY_LOAD_WRAP
        
; ------------------------------ baload - 0x33 -------------------------------
asm_baload:
    ARRAY_LOAD_SETUP baload
    sub     java_sp, 4
    movsx   eax, byte ptr [eax + ebx + ARRAY_DATA_OFFSET]   
    ARRAY_LOAD_WRAP
        
; ------------------------------ caload - 0x34 -------------------------------
asm_caload:
    ARRAY_LOAD_SETUP caload
    sub     java_sp, 4
    movzx   eax, word ptr [eax + 2 * ebx + ARRAY_DATA_OFFSET]   
    ARRAY_LOAD_WRAP
        
; ------------------------------ saload - 0x35 -------------------------------
asm_saload:
    ARRAY_LOAD_SETUP saload
    sub     java_sp, 4
    movsx   eax, word ptr [eax + 2 * ebx + ARRAY_DATA_OFFSET]   
    ARRAY_LOAD_WRAP
    
; ------------------------------ istore - 0x36 -------------------------------
; ------------------------------ astore - 0x3a -------------------------------
asm_istore:
asm_astore:
    ISTORE
    
; ------------------------------ lstore - 0x37 -------------------------------
asm_lstore:
    LSTORE
    
; ----------------------------- istore_0 - 0x3b ------------------------------
asm_istore_0:
    ISTORE_CONST 0
    
; ----------------------------- istore_1 - 0x3c ------------------------------
asm_istore_1:
    ISTORE_CONST 4
    
; ----------------------------- istore_2 - 0x3d ------------------------------
asm_istore_2:
    ISTORE_CONST 8
    
; ----------------------------- istore_3 - 0x3e ------------------------------
asm_istore_3:
    ISTORE_CONST 12
    
; ----------------------------- lstore_0 - 0x3f ------------------------------
asm_lstore_0:
    LSTORE_CONST 0, 4
    
; ----------------------------- lstore_1 - 0x40 ------------------------------
asm_lstore_1:
    LSTORE_CONST 4, 8
    
; ----------------------------- lstore_2 - 0x41 ------------------------------
asm_lstore_2:
    LSTORE_CONST 8, 12
    
; ----------------------------- lstore_3 - 0x42 ------------------------------
asm_lstore_3:
    LSTORE_CONST 12, 16
    
; ----------------------------- astore_0 - 0x4b ------------------------------
asm_astore_0:
    ISTORE_CONST 0
    
; ----------------------------- astore_1 - 0x4c ------------------------------
asm_astore_1:
    ISTORE_CONST 4
    
; ----------------------------- astore_2 - 0x4d ------------------------------
asm_astore_2:
    ISTORE_CONST 8
    
; ----------------------------- astore_3 - 0x4e ------------------------------
asm_astore_3:
    ISTORE_CONST 12

; ------------------------------ iastore - 0x4f ------------------------------
asm_iastore:
    ARRAY_STORE_SETUP iastore
    mov     dword ptr [eax + 4 * ebx + ARRAY_DATA_OFFSET], ecx
    ARRAY_STORE_WRAP    

; ------------------------------ lastore - 0x50 ------------------------------
asm_lastore:
    mov     eax, dword ptr [java_sp - 12]               
    test    eax, eax                                    
    je      handleNullPointerException  
    mov     ebx, dword ptr [java_sp - 8]        
    cmp     ebx, dword ptr [eax + ARRAY_LENGTH_OFFSET]
    jb      lastore_ok
    jmp     handleArrayIndexOutOfBoundsException    
lastore_ok: 
    inc     java_ip     
    mov     ecx, dword ptr [java_sp - 4]
    mov     dword ptr [eax + 8 * ebx + ARRAY_DATA_OFFSET], ecx
    mov     ecx, dword ptr [java_sp]
    mov     dword ptr [eax + 8 * ebx + ARRAY_DATA_OFFSET_LONG], ecx
    sub     java_sp, 16
    NEXT_BYTECODE

; ------------------------------ aastore - 0x53 ------------------------------
asm_aastore:
    mov     eax, dword ptr [java_sp - 8]                ; eax = arrayref                        
    test    eax, eax                                    ; arrayref == null?             
    je      handleNullPointerException                  ; yes, throw npe
    
    mov     ecx, dword ptr [java_sp - 4]                ; ecx = index           
    cmp     ecx, dword ptr [eax + ARRAY_LENGTH_OFFSET]  ; index < 0 || index >= arrayref.length?
    jae     handleArrayIndexOutOfBoundsException        ; yes, throw aioobe
    
    mov     ebx, dword ptr [java_sp]                    ; ebx = value                   
    test    ebx, ebx                                    ; value == null?
    je      aastore_ok                                  ; yes, skip assignable checks
    
    mov     ebx, dword ptr [ebx + INSTANCE_CLASS_OFFSET]; ebx = fromClass (ptr to class of value)   
    mov     edx, dword ptr [eax + INSTANCE_CLASS_OFFSET]
    mov     edx, dword ptr [edx + ELEMENT_CLASS_OFFSET] ; edx = toClass (ptr to class of arrayref elements)
    
    push    edx                                         ; push toClass                                  
    push    ebx                                         ; push fromClass                                    
    
    call    _isAssignableToFast                         ; call isAssignableToFast(fromClass, toClass)                           
    pop     ebx
    pop     edx
    test    eax, eax                                    ; result == true?                   
    jne     aastore_ok                                  ; yes, skip slow check
                                                                
    push    edx
    push    ebx
    VMSAVE                                              ; save local vm regs
    call    _isAssignableTo                             ; call isAssignableTo(fromClass, toClass)                   
    VMRESTORE                                           ; restore local vm regs                                     
    add     esp, 8                                      ; pop args off stack
    test    eax, eax                                    ; result == false?                                      
    je      handleArrayStoreException                   ; yes, throw ase
    
aastore_ok:                                                 
    inc     java_ip                                     ; advance ip to next bytecode
    mov     eax, dword ptr [java_sp - 8]                ; eax = arrayref                        
    mov     ecx, dword ptr [java_sp - 4]                ; ecx = index               
    mov     ebx, dword ptr [java_sp]                    ; ebx = value
    
    mov     dword ptr [eax + 4 * ecx + ARRAY_DATA_OFFSET], ebx ; arrayref.data[index] = value
    sub     java_sp, 12                                 ; pop java args off stack
    NEXT_BYTECODE

; ------------------------------ bastore - 0x54 ------------------------------
asm_bastore:
    ARRAY_STORE_SETUP bastore
    mov     byte ptr [eax + ebx + ARRAY_DATA_OFFSET], cl
    ARRAY_STORE_WRAP    

; ------------------------------ castore - 0x55 ------------------------------
; ------------------------------ sastore - 0x56 ------------------------------
asm_castore:
asm_sastore:
    ARRAY_STORE_SETUP sastore
    mov     word ptr [eax + 2 * ebx + ARRAY_DATA_OFFSET], cx
    ARRAY_STORE_WRAP    

; ------------------------------- pop - 0x57 ---------------------------------
asm_pop:
    inc     java_ip
    sub     java_sp, 4
    NEXT_BYTECODE

; ------------------------------- pop - 0x58 ---------------------------------
asm_pop2:
    inc     java_ip
    sub     java_sp, 8
    NEXT_BYTECODE
    
; ------------------------------- dup - 0x59 ---------------------------------
asm_dup:
    mov     eax, dword ptr [java_sp]
    add     java_sp, 4
    inc     java_ip
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------ dup_x1 - 0x5a -------------------------------
asm_dup_x1:
    mov     eax, dword ptr [java_sp + 0]
    add     java_sp, 4
    inc     java_ip
    mov     dword ptr [java_sp], eax
    xchg    eax, dword ptr [java_sp - 8]
    mov     dword ptr [java_sp - 4], eax
    NEXT_BYTECODE
    
; ------------------------------ dup_x2 - 0x5b -------------------------------
asm_dup_x2:
    mov     eax, dword ptr [java_sp +  0]
    add     java_sp, 4
    inc     java_ip
    mov     dword ptr [java_sp + 0], eax
    xchg    eax, dword ptr [java_sp - 12]
    xchg    eax, dword ptr [java_sp -  8]
    mov     dword ptr [java_sp - 4], eax
    NEXT_BYTECODE
    
; ------------------------------- dup2 - 0x5c --------------------------------
asm_dup2:
    mov     eax, dword ptr [java_sp + 0]
    mov     dword ptr [java_sp + 8], eax
    mov     eax, dword ptr [java_sp - 4]
    mov     dword ptr [java_sp + 4], eax
    inc     java_ip
    add     java_sp, 8
    NEXT_BYTECODE
    
; ----------------------------- dup2_x1 - 0x5d -------------------------------
asm_dup2_x1:
    mov     eax, dword ptr [java_sp + 0]
    mov     dword ptr [java_sp + 8], eax
    xchg    dword ptr [java_sp - 4], eax
    mov     dword ptr [java_sp + 4], eax
    xchg    dword ptr [java_sp - 8], eax
    mov     dword ptr [java_sp + 0], eax
    inc     java_ip
    add     java_sp, 8
    NEXT_BYTECODE
    
; ----------------------------- dup2_x2 - 0x5e -------------------------------
asm_dup2_x2:
    mov     eax, dword ptr [java_sp]
    mov     dword ptr [java_sp +  8], eax
    xchg    dword ptr [java_sp -  8], eax
    mov     dword ptr [java_sp +  0], eax
    mov     eax, dword ptr [java_sp -  4]
    mov     dword ptr [java_sp +  4], eax
    xchg    dword ptr [java_sp - 12], eax
    mov     dword ptr [java_sp -  4], eax
    inc     java_ip
    add     java_sp, 8
    NEXT_BYTECODE
    
; ------------------------------- swap - 0x5f --------------------------------
asm_swap:
    mov     eax, dword ptr [java_sp]
    xchg    eax, dword ptr [java_sp - 4]
    inc     java_ip
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- iadd - 0x60 --------------------------------
asm_iadd:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    add     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- ladd - 0x61 --------------------------------
asm_ladd:
    mov     eax, dword ptr [java_sp - 4]    ; get value2.lo
    sub     java_sp, 8                      ; sp -= 2
    inc     java_ip                         ; advance to next bytecode
    add     dword ptr [java_sp - 4], eax    ; add value2.lo to value1.lo, potentially setting CF
    mov     eax, dword ptr [java_sp + 8]    ; get value2.hi
    adc     dword ptr [java_sp], eax        ; add value2.hi (and potentially CF) to value1.hi
    NEXT_BYTECODE
    
; ------------------------------- isub - 0x64 --------------------------------
asm_isub:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    sub     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- lsub - 0x65 --------------------------------
asm_lsub:
    mov     eax, dword ptr [java_sp - 4]    ; get value2.lo
    sub     java_sp, 8                      ; sp -= 2
    inc     java_ip                         ; advance to next bytecode
    sub     dword ptr [java_sp - 4], eax    ; subtract value2.lo from value1.lo, potentially setting CF
    mov     eax, dword ptr [java_sp + 8]    ; get value2.hi
    sbb     dword ptr [java_sp], eax        ; subtract value2.hi (and potentially CF) from value1.hi
    NEXT_BYTECODE
    
; ------------------------------- imul - 0x68 --------------------------------
asm_imul:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    inc     java_ip
    imul    eax, dword ptr [java_sp]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- lmul - 0x69 --------------------------------
asm_lmul:
    inc     java_ip
    push    dword ptr [java_sp -  0]
    push    dword ptr [java_sp -  4]
    push    dword ptr [java_sp -  8]
    push    dword ptr [java_sp - 12]
    sub     java_sp, 8
    call    _allmul
    mov     dword ptr [java_sp -  4], eax
    mov     dword ptr [java_sp -  0], edx
    NEXT_BYTECODE

_allmul:
    mov         eax,dword ptr [esp+8]
    mov         ecx,dword ptr [esp+10h]
    or          ecx,eax
    mov         ecx,dword ptr [esp+0Ch]
    jne         hard
    mov         eax,dword ptr [esp+4]
    mul         ecx
    ret         10h
hard:
    push        ebx
    mul         ecx
    mov         ebx,eax
    mov         eax,dword ptr [esp+8]
    mul         dword ptr [esp+14h]
    add         ebx,eax
    mov         eax,dword ptr [esp+8]
    mul         ecx
    add         edx,ebx
    pop         ebx
    ret         10h

; ------------------------------- idiv - 0x6c --------------------------------
asm_idiv:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    test    eax, eax
    je      handleArithmeticException
    inc     java_ip
    cmp     dword ptr [java_sp], 080000000h
    jne     idiv_default
    cmp     eax, 0ffffffffh
    jne     idiv_default
    NEXT_BYTECODE
idiv_default:
    mov     eax, dword ptr [java_sp]
    cdq
    idiv    dword ptr [java_sp + 4]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- ldiv - 0x6d --------------------------------
asm_ldiv:
    LONG_DIV _ll_div

; ------------------------------- irem - 0x70 --------------------------------
asm_irem:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    test    eax, eax
    je      handleArithmeticException
    inc     java_ip
    cmp     dword ptr [java_sp], 080000000h
    jne     irem_default
    cmp     eax, 0ffffffffh
    jne     irem_default
    xor     eax, eax
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
irem_default:
    mov     eax, dword ptr [java_sp]
    cdq
    idiv    dword ptr [java_sp + 4]
    mov     dword ptr [java_sp], edx
    NEXT_BYTECODE
    
; ------------------------------- lrem - 0x71 --------------------------------
asm_lrem:
    LONG_DIV _ll_rem

; ------------------------------- ineg - 0x74 --------------------------------
asm_ineg:
    inc     java_ip
    neg dword ptr [java_sp]
    NEXT_BYTECODE
    
; ------------------------------- lneg - 0x75 --------------------------------
asm_lneg:
    inc     java_ip
    neg     dword ptr [java_sp - 4]
    adc     dword ptr [java_sp - 0], 0
    neg     dword ptr [java_sp - 0]
    NEXT_BYTECODE
    
; ------------------------------- ishl - 0x78 --------------------------------
asm_ishl:
    INT_SHIFT sal
        
; ------------------------------- lshl - 0x79 --------------------------------
asm_lshl:
    inc     java_ip
    mov     ecx, dword ptr [java_sp]            ; get shift count from stack
    sub     java_sp, 4                          ; sp--
    mov     eax, dword ptr [java_sp - 4]        ; get value1.lo from stack
    and     ecx, 63                             ; only look at lower six bits of shift count
    cmp     ecx, 32                             ; shift count >= 32?
    jl      lshl_five_bits
    mov     dword ptr [java_sp], eax            ; yes, move value1.lo to value1.hi
    xor     eax, eax                            ; and clear value1.lo
lshl_five_bits:
    shld    dword ptr [java_sp], eax, cl        ; no, shift bits from value1.lo into value1.hi
    shl     eax, cl                             ; shift value1.lo
    mov     dword ptr [java_sp - 4], eax        ; push value1.lo onto stack
    NEXT_BYTECODE
    
; ------------------------------- ishr - 0x7a --------------------------------
asm_ishr:
    INT_SHIFT sar
        
; ------------------------------- lshr - 0x7b --------------------------------
asm_lshr:
    inc     java_ip
    mov     ecx, dword ptr [java_sp]
    sub     java_sp, 4
    mov     eax, dword ptr [java_sp]
    and     ecx, 63
    cmp     ecx, 32
    jl      lshr_five_bits
    mov     dword ptr [java_sp - 4], eax
    sar     eax, 31
lshr_five_bits:
    shrd    dword ptr [java_sp - 4], eax, cl
    sar     eax, cl
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- iushr - 0x7c --------------------------------
asm_iushr:
    INT_SHIFT shr
    
; ------------------------------- lushr - 0x7d --------------------------------
asm_lushr:
    inc     java_ip
    mov     ecx, dword ptr [java_sp]
    sub     java_sp, 4
    mov     eax, dword ptr [java_sp]
    and     ecx, 63
    cmp     ecx, 32
    jl      lushr_five_bits
    mov     dword ptr [java_sp - 4], eax
    xor     eax, eax
lushr_five_bits:
    shrd    dword ptr [java_sp - 4], eax, cl
    shr     eax, cl
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ------------------------------- iand - 0x7e --------------------------------
asm_iand:
    INT_BOOL_OP and
    
; ------------------------------- land - 0x7f --------------------------------
asm_land:
    LONG_BOOL_OP and
    
; -------------------------------- ior - 0x80 --------------------------------
asm_ior:
    INT_BOOL_OP or
    
; -------------------------------- lor - 0x81 --------------------------------
asm_lor:
    LONG_BOOL_OP or
    
; -------------------------------- ixor - 0x82 -------------------------------
asm_ixor:
    INT_BOOL_OP xor
    
; -------------------------------- lxor - 0x83 -------------------------------
asm_lxor:
    LONG_BOOL_OP xor
    
; -------------------------------- iinc - 0x84 -------------------------------
asm_iinc:
    movzx   eax, byte ptr [java_ip + 1]
    movsx   ecx, byte ptr [java_ip + 2]
    add     dword ptr [java_lp + 4 * eax], ecx
    add     java_ip, 3
    NEXT_BYTECODE
    
; --------------------------------- i2l - 0x85 -------------------------------
asm_i2l:
    inc     java_ip                     ; advance to next bytecode
    mov     eax, dword ptr [java_sp]    ; get value into eax
    add     java_sp, 4                  ; sp++
    cdq                                 ; sign-extend eax into edx:eax, making edx result.hi
    mov     dword ptr [java_sp], edx    ; put result.hi at top of stack
    NEXT_BYTECODE
    
; --------------------------------- l2i - 0x88 -------------------------------
asm_l2i:
    inc     java_ip                     ; advance to next bytecode
    sub     java_sp, 4                  ; sp[-1] already holds value.lo (result), so just do sp--
    NEXT_BYTECODE

; --------------------------------- i2b - 0x91 -------------------------------
asm_i2b:
    inc     java_ip                     ; advance to next bytecode
    movsx   eax, byte ptr [java_sp]     ; take low-order eight bit of value and sign-extend to int
    mov     dword ptr [java_sp], eax    ; save result at top of stack
    NEXT_BYTECODE   
    
; --------------------------------- i2c - 0x92 -------------------------------
asm_i2c:
    inc     java_ip                     ; advance to next bytecode
    movzx   eax, word ptr [java_sp]     ; take low-order sixteen bit of value and zero-extend to int
    mov     dword ptr [java_sp], eax    ; save result at top of stack
    NEXT_BYTECODE   
    
; --------------------------------- i2s - 0x93 -------------------------------
asm_i2s:
    inc     java_ip                     ; advance to next bytecode
    movsx   eax, word ptr [java_sp]     ; take low-order sixteen bit of value and sign-extend to int
    mov     dword ptr [java_sp], eax    ; save result at top of stack
    NEXT_BYTECODE   

; -------------------------------- lcmp - 0x94 -------------------------------
asm_lcmp:
    sub     java_sp, 12
    inc     java_ip
    mov     eax, dword ptr [java_sp +  8]
    sub     dword ptr [java_sp +  0], eax
    mov     eax, dword ptr [java_sp + 12]
    sbb     dword ptr [java_sp +  4], eax
    mov     eax, dword ptr [java_sp +  4]
    jno     lcmp_no_underflow
    not     eax
lcmp_no_underflow:
    or      dword ptr [java_sp +  0], eax
    je      lcmp_done
    sar     eax, 30
    and     eax, -2
    inc     eax
    mov     dword ptr [java_sp +  0], eax
lcmp_done:
    NEXT_BYTECODE

    
; ------------------------------- ifeq - 0x99 --------------------------------
asm_ifeq:
    IFCOND ifeq, ne

; ------------------------------- ifne - 0x9a --------------------------------
asm_ifne:
    IFCOND ifne, e

; ------------------------------- iflt - 0x9b --------------------------------
asm_iflt:
    IFCOND iflt, ge

; ------------------------------- ifge - 0x9c --------------------------------
asm_ifge:
    IFCOND ifge, l

; ------------------------------- ifgt - 0x9d --------------------------------
asm_ifgt:
    IFCOND ifgt, le

; ------------------------------- ifle - 0x9e --------------------------------
asm_ifle:
    IFCOND ifle, g

; ----------------------------- if_icmpeq - 0x9f -----------------------------
; ----------------------------- if_acmpeq - 0xa5 -----------------------------
asm_if_icmpeq:
asm_if_acmpeq:
    IFICMPCOND if_icmpeq, ne

; ----------------------------- if_icmpne - 0xa0 -----------------------------
; ----------------------------- if_acmpne - 0xa6 -----------------------------
asm_if_icmpne:
asm_if_acmpne:
    IFICMPCOND if_icmpne, e

; ----------------------------- if_icmplt - 0xa1 -----------------------------
asm_if_icmplt:
    IFICMPCOND if_icmplt, ge

; ----------------------------- if_icmpge - 0xa2 -----------------------------
asm_if_icmpge:
    IFICMPCOND if_icmpge, l

; ----------------------------- if_icmpgt - 0xa3 -----------------------------
asm_if_icmpgt:
    IFICMPCOND if_icmpgt, le

; ----------------------------- if_icmple - 0xa4 -----------------------------
asm_if_icmple:
    IFICMPCOND if_icmple, g

; ------------------------------- goto - 0xa7 --------------------------------
asm_goto:
    mov     ax, word ptr [java_ip + 1]
    xchg    ah, al
    movsx   eax, ax
    add     java_ip, eax
    RESCHEDULE

; ---------------------------- tableswitch - 0xab ----------------------------
asm_tableswitch:
    mov     eax, dword ptr [java_sp]                    ; get key
    sub     java_sp, 4                                  ; sp--
    push    java_ip                                     ; save ip
    lea     java_ip, dword ptr [java_ip + 4]            ; align ip on the 
    and     java_ip, -4                                 ; next four byte boundary
    mov     ecx, dword ptr [java_ip + 4]                ; get low 
    BYTE_SWAP c                                         ; byte-swap low
    mov     edx, dword ptr [java_ip + 8]                ; get high 
    BYTE_SWAP d                                         ; byte-swap high
    cmp     eax, ecx                                    ; index < low
    jl      tablesw_default                             ; yes, use default offset
    cmp     eax, edx                                    ; index > high
    jg      tablesw_default                             ; yes, use default offset
    sub     eax, ecx                                    ; index -= low
    mov     edx, dword ptr [java_ip + 4 * eax + 12]     ; get offset for index
tablesw_addoffset:
    BYTE_SWAP d                                         ; byte-swap offset
    pop     java_ip                                     ; restore ip
    add     java_ip, edx                                ; ip += offset
    RESCHEDULE
tablesw_default:
    mov     edx, dword ptr [java_ip]                    ; get default offset
    jmp     tablesw_addoffset

; --------------------------- lookupswitch - 0xab ----------------------------
asm_lookupswitch:
    mov     eax, dword ptr [java_sp]                    ; get key
    sub     java_sp, 4                                  ; sp--
    BYTE_SWAP a                                         ; byte-swap key
    push    java_ip                                     ; save ip
    lea     java_ip, dword ptr [java_ip + 4]            ; align ip on the
    and     java_ip, -4                                 ; next four byte boundary
    mov     ecx, dword ptr [java_ip + 4]                ; init loop_counter with 'npairs'
    BYTE_SWAP c                                         ; byte-swap loop_counter
    jmp     lookupsw_skip                               
lookupsw_loop:
    cmp     eax, dword ptr [java_ip + 8 * ecx + 8]      ; cmp key to key of current npair
    je      lookupsw_found                              ; break out of loop when found
lookupsw_skip:
    dec     ecx                                         ; loop_counter--                
    jge     lookupsw_loop
    mov     eax, dword ptr [java_ip]                    ; get default offset
    jmp     lookupsw_done
lookupsw_found:
    mov     eax, dword ptr [java_ip + 8 * ecx + 12]     ; get offset for key
lookupsw_done:
    BYTE_SWAP a                                         ; byte swap offset                      
    pop     java_ip                                     ; restore ip
    add     java_ip, eax                                ; ip += offset
    RESCHEDULE


; ------------------------------ ireturn - 0xac ------------------------------
; ------------------------------ lreturn - 0xad ------------------------------
; ------------------------------ areturn - 0xb0 ------------------------------
; ------------------------------- return - 0xb1 ------------------------------
asm_ireturn:
asm_lreturn:
asm_areturn:
asm_return:
    mov     ecx, dword ptr [_GlobalState + FP_OFFSET]       ; ecx = fp
    mov     eax, dword ptr [ecx + FRAME_SYNC_OFFSET]        ; eax = fp->syncObject
    test    eax, eax
    je      return_killthread
    
    mov     edx, offset exitError
    push    edx
    push    eax
    call    _monitorExit
    add     esp, 8
    cmp     eax, MONITOR_STATUS_ERROR
    jne     return_killthread
    mov     eax, dword ptr [exitError]
    jmp     handleException
    
return_killthread:
    inc     java_ip
    mov     ecx, dword ptr [_GlobalState + FP_OFFSET]       ; ecx = fp
    mov     eax, dword ptr [ecx + FRAME_PREV_IP_OFFSET]     ; eax = fp->previousIp
    cmp     eax, KILLTHREAD
    jne     return_common
    
    VMSAVE
    call    _stopThread
    VMRESTORE
    cmp     dword ptr [_AliveThreadCount], 0
    jle     Interpreter_Done
    RESCHEDULE

return_common:
    movzx   ebx, byte ptr [java_ip - 1]
    mov     eax, ebx
    and     eax, 1
    test    eax, eax
    jne     return_uneven
    
    push    dword ptr [java_sp]
    VMSAVE
    ; FIXME: write assembly version of popFrame! That
    ; will hopefully also eliminate the need for vmsave
    ; and vmrestore around the popFrame calls
    call    _popFrame
    VMRESTORE
    add     java_sp, 4
    pop     dword ptr [java_sp]
    RESCHEDULE
return_uneven:
    cmp     ebx, OP_RETURN
    jne     return_long
    VMSAVE
    call    _popFrame
    VMRESTORE
    RESCHEDULE
return_long:
    push    dword ptr [java_sp - 0]
    push    dword ptr [java_sp - 4]
    VMSAVE
    call    _popFrame
    VMRESTORE
    add     java_sp, 8
    pop     dword ptr [java_sp - 4]
    pop     dword ptr [java_sp - 0]
    RESCHEDULE

; ----------------------------- arraylength - 0xbe ---------------------------
asm_arraylength:
    mov     eax, dword ptr [java_sp]                    ; get arrayref from top of stack
    test    eax, eax                                    ; arrayref == 0?
    je      handleNullPointerException                  ; yes, throw NPE
    inc     java_ip                                     ; advance ip to next byte code
    mov     eax, dword ptr [eax + ARRAY_LENGTH_OFFSET]  ; get arrayref->length
    mov     dword ptr [java_sp], eax                    ; put length at top of stack
    NEXT_BYTECODE
    
; ------------------------------- athrow - 0xbe ------------------------------
asm_athrow:
    mov     eax, dword ptr [java_sp]            ; get objectref from stack  
    sub     java_sp, 4                          ; sp--
    test    eax, eax                            ; objectref == null?
    je      handleNullPointerException          ; yes, throw npe
    VMSAVE                                      ; save local vm registers
    mov     ebx, offset _thisObjectGCSafe
    mov     dword ptr [ebx], eax                ; thisObjectGCSafe = objectref
    push    ebx
    call    _throwException                     ; call throwException(&thisObjectGCSafe) 
    add     esp, 4
    mov     dword ptr [_thisObjectGCSafe], 0    ; thisObjectGCSafe = null
    VMRESTORE                                   ; restore local vm registers
    RESCHEDULE

; ---------------------------- monitorenter - 0xc2 ---------------------------
asm_monitorenter:
    mov     eax, dword ptr [java_sp]        ; get objectref from stack  
    sub     java_sp, 4                      ; sp--
    test    eax, eax                        ; objectref == null?
    je      handleNullPointerException      ; yes, throw npe
    inc     java_ip                         ; ip++
    VMSAVE                                  ; save local vm registers
    push    eax
    call    _monitorEnter                   ; call monitorEnter(objectref)
    add     esp, 4
    VMRESTORE                               ; restore local vm registers
    RESCHEDULE                          

; ---------------------------- monitorexit - 0xc3 ---------------------------
asm_monitorexit:
    mov     eax, dword ptr [java_sp]        ; get objectref from stack
    sub     java_sp, 4                      ; sp--
    test    eax, eax                        ; objectref == null?
    je      handleNullPointerException      ; yes, throw npe
    inc     java_ip                         ; ip++
    mov     edx, offset exitError
    push    edx
    push    eax
    call    _monitorExit                    ; call monitorExit(objectref, &exitError)
    add     esp, 8
    cmp     eax, MONITOR_STATUS_ERROR       ; result == MONITOR_STATUS_ERROR?
    jne     monitorexit_done
    mov     eax, dword ptr [exitError]      ; yes, throw appropriate exception
    jmp     handleException
monitorexit_done:
    RESCHEDULE

; -------------------------------- wide - 0xc4 -------------------------------
asm_wide:
    movzx   ebx, byte ptr [java_ip + 1]             ; ebx = wide opcode
    cmp     ebx, OP_IINC                            ; opcode == iinc?
    jne     wide_skip_iinc                          ; no, skip iinc code
    movzx   eax, word ptr [java_ip + 2]             ; yes, eax = index
    xchg    ah, al                                  ; convert to big-endian
    mov     bx, word ptr [java_ip + 4]              ; ebx = const
    xchg    bh, bl                                  ; convert to big-endian
    movsx   ebx, bx                                 ; sign-extend to 32 bit
    add     java_ip, 6                              ; advance to next bytecode
    add     dword ptr [java_lp + 4 * eax], ebx      ; local_var[index] += const
    NEXT_BYTECODE
wide_skip_iinc:
    sub     ebx, OP_ILOAD                           ; subtract iload from opcode
    cmp     ebx, 4                                  
    jle     wide_load                               ; 0 <= opcode <= 4?
    sub     ebx, 28                                 ; no, subtract (istore - 5)
wide_load:
    jmp     [wide_jumptable + 4 * ebx]              ; dispatch to matching handler

aload_wide:
iload_wide:
    add     java_sp, 4                              ; sp++
    movzx   eax, word ptr [java_ip + 2] 
    xchg    ah, al                                  ; eax = index
    add     java_ip, 4                              ; advance to next bytecode
    mov     eax, dword ptr [java_lp + 4 * eax]      ; eax = local_var[index]
    mov     dword ptr [java_sp], eax                ; push local_var[index]
    NEXT_BYTECODE

astore_wide:
istore_wide:
    mov     eax, dword ptr [java_sp]
    sub     java_sp, 4
    movzx   ecx, word ptr [java_ip + 2]
    xchg    ch, cl
    add     java_ip, 4
    mov     dword ptr [java_lp + 4 * ecx], eax
    NEXT_BYTECODE

lload_wide:
    add     java_sp, 8
    movzx   eax, word ptr [java_ip + 2]
    xchg    ah, al
    add     java_ip, 4
    mov     ebx, dword ptr [java_lp + 4 * eax]
    mov     dword ptr [java_sp - 4], ebx
    inc     eax
    mov     eax, dword ptr [java_lp + 4 * eax]
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE

lstore_wide:
    mov     eax, dword ptr [java_sp - 4]
    sub     java_sp, 8
    movzx   ecx, word ptr [java_ip + 2]
    xchg    ch, cl
    add     java_ip, 4
    mov     dword ptr [java_lp + 4 * ecx], eax
    inc     ecx
    mov     eax, dword ptr [java_sp + 8]
    mov     dword ptr [java_lp + 4 * ecx], eax
    NEXT_BYTECODE

wide_jumptable:
    dd iload_wide
    dd lload_wide
    dd fload_wide
    dd dload_wide
    dd aload_wide
    dd istore_wide
    dd lstore_wide
    dd fstore_wide
    dd dstore_wide
    dd astore_wide

; ------------------------------- ifnull - 0xc6 ------------------------------
asm_ifnull:
    IFCOND ifnull, ne

; ----------------------------- ifnonnull - 0xc7 -----------------------------
asm_ifnonnull:
    IFCOND ifnonnull, e
    
; ------------------------------ goto_w - 0xc8 -------------------------------
asm_goto_w:
    mov     eax, dword ptr [java_ip + 1]
    BYTE_SWAP a
    add     java_ip, eax
    RESCHEDULE
    
; ---------------------------- breakpoint - 0xc8 -----------------------------
asm_breakpoint:
    push    offset breakpoint_str
    call    _fatalError
    
; -------------------------- getfield_quick - 0xcb ---------------------------
; -------------------------- getfieldp_quick - 0xcc --------------------------
asm_getfield_quick:             
asm_getfieldp_quick:        
    mov     eax, dword ptr [java_sp]
    test    eax, eax
    je      handleNullPointerException
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl
    add     java_ip, 3
    mov     ecx, dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET]
    mov     dword ptr [java_sp], ecx
    NEXT_BYTECODE
    
; -------------------------- getfield2_quick - 0xcd --------------------------
asm_getfield2_quick:                
    mov     eax, dword ptr [java_sp]
    test    eax, eax
    je      handleNullPointerException
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl
    add     java_ip, 3
    mov     ecx, dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET]
    mov     dword ptr [java_sp], ecx
    inc     ebx
    add     java_sp, 4
    mov     ecx, dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET]
    mov     dword ptr [java_sp], ecx
    NEXT_BYTECODE
    
; -------------------------- putfield_quick - 0xce ---------------------------
asm_putfield_quick:
    sub     java_sp, 8
    mov     eax, dword ptr [java_sp + 4]
    test    eax, eax
    je      handleNullPointerException
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl
    add     java_ip, 3
    mov     ecx, dword ptr [java_sp + 8]
    mov     dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET], ecx
    NEXT_BYTECODE
    
; -------------------------- putfield2_quick - 0xcf --------------------------
asm_putfield2_quick:
    sub     java_sp, 12
    mov     eax, dword ptr [java_sp + 4]
    test    eax, eax
    je      handleNullPointerException
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl
    add     java_ip, 3
    mov     ecx, dword ptr [java_sp + 8]
    mov     dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET], ecx
    inc     ebx
    mov     ecx, dword ptr [java_sp + 12]
    mov     dword ptr [eax + 4 * ebx + INSTANCE_DATA_OFFSET], ecx
    NEXT_BYTECODE
    
; -------------------------- getstatic_quick - 0xd0 --------------------------
; -------------------------- getstaticp_quick - 0xd1 -------------------------
asm_getstatic_quick:                
asm_getstaticp_quick:   
    STATIC_SETUP getstatic, FIELD_CLASS_OFFSET
    add     java_ip, 3
    add     java_sp, 4
    mov     ecx, dword ptr [eax + FIELD_ADDR_OFFSET]
    mov     ecx, dword ptr [ecx]
    mov     dword ptr [java_sp], ecx
    NEXT_BYTECODE
    
; -------------------------- getstatic2_quick - 0xd2 -------------------------
asm_getstatic2_quick:
    STATIC_SETUP getstatic2, FIELD_CLASS_OFFSET
    add     java_ip, 3
    add     java_sp, 8
    mov     ecx, dword ptr [eax + FIELD_ADDR_OFFSET]
    mov     ebx, dword ptr [ecx]
    mov     dword ptr [java_sp - 4], ebx
    mov     ebx, dword ptr [ecx + 4]
    mov     dword ptr [java_sp  - 0], ebx
    NEXT_BYTECODE

; -------------------------- putstatic_quick - 0xd3 --------------------------
asm_putstatic_quick:    
    STATIC_SETUP putstatic, FIELD_CLASS_OFFSET
    add     java_ip, 3
    mov     ecx, dword ptr [eax + FIELD_ADDR_OFFSET]
    mov     eax, dword ptr [java_sp]
    mov     dword ptr [ecx], eax
    sub     java_sp, 4
    NEXT_BYTECODE

; -------------------------- putstatic_quick - 0xd4 --------------------------
asm_putstatic2_quick:   
    STATIC_SETUP putstatic2, FIELD_CLASS_OFFSET
    add     java_ip, 3
    mov     ecx, dword ptr [eax + FIELD_ADDR_OFFSET]
    mov     ebx, dword ptr [java_sp - 4]
    mov     dword ptr [ecx], ebx
    mov     ebx, dword ptr [java_sp]
    mov     dword ptr [ecx + 4], ebx
    sub     java_sp, 8
    NEXT_BYTECODE

; ------------------------ invokevirtual_quick - 0xd6 ------------------------
asm_invokevirtual_quick:    
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = iCacheIndex
    mov     ecx, ebx
    shl     ebx, 3
    shl     ecx, 2
    add     ebx, ecx
    mov     ecx, dword ptr [_InlineCache]               ; ecx = InlineCache
    add     ecx, ebx                                    ; ecx = thisICache
    mov     eax, dword ptr [ecx]                        ; eax = thisMethod
    movzx   edx, word ptr [eax + METHOD_ARGCOUNT_OFFSET]; edx = thisMethod->argCount
    dec     edx
    shl     edx, 2
    mov     ebx, java_sp
    sub     ebx, edx                                    ; ebx = sp - argCount + 1
    mov     ebx, dword ptr [ebx]                        ; ebx = thisObject
    test    ebx, ebx
    je      handleNullPointerException
    mov     edx, dword ptr [ebx + INSTANCE_CLASS_OFFSET]; edx = dynamicClass
    cmp     dword ptr [eax + METHOD_CLASS_OFFSET], edx  ; defaultClass == dynamicClass?
    je      invokevirtual_nolookup                      ; yes, skip method lookup
    push    ecx                                         ; no, save thisICache
    push    ebx                                         ; save thisObject
    mov     ecx, dword ptr [_GlobalState + FP_OFFSET]   ; push args
    mov     ecx, dword ptr [ecx + FRAME_METHOD_OFFSET]
    mov     ecx, dword ptr [ecx + METHOD_CLASS_OFFSET]
    push    ecx
    mov     ecx, dword ptr [eax + METHOD_KEY_OFFSET]
    push    ecx
    push    edx
    VMSAVE
    call    _lookupMethod                               ; call loopupMethod(
                                                        ;   dynamicClass, 
                                                        ;   thisMethod->nameTypeKey, 
                                                        ;   fp_global->thisMethod->ofClass
                                                        ; )
                                                        ;
                                                        ; upon return, eax holds new and 
                                                        ; improved thisMethod
    add     esp, 0ch
    VMRESTORE
    pop     ebx                                         ; restore thisObject
    pop     ecx                                         ; restore thisICache
    mov     dword ptr [ecx], eax                        ; thisICache->contents = thisMethod
invokevirtual_nolookup:
    test    eax, eax                                    ; thisMethod == null?
    je      icache_error                                ; yes, fatal error
    mov     ecx, 3                                      ; no, ecx = ipIncrement
    ; fall-through to call_method

; expected parameters:
; eax: thisMethod
; ebx: thisObject
; ecx: ipIncrement
call_method:
    mov     edx, dword ptr [eax + METHOD_ACCESSFLAGS_OFFSET]    ; edx = thisMethod->accessFlags
    and     edx, ACC_NATIVE                                     ; ACC_NATIVE bit set?
    je      call_method_not_native                              ; no, skip native call
    add     java_ip, ecx                                        ; yes, ip += ipIncrement
    VMSAVE                                                      ; save local vm regs
    push    eax
    call    _invokeNativeFunction                               ; call invokeNativeFunction(thisMethod)
    add     esp, 4
    VMRESTORE                                                   ; restore local regs
    RESCHEDULE
call_method_not_native:
    mov     edx, dword ptr [eax + METHOD_ACCESSFLAGS_OFFSET]    ; edx = thisMethod->accessFlags
    and     edx, ACC_ABSTRACT                                   ; ACC_ABSTRACT flag set?
    je      call_method_not_abstract                            ; no, skip abort
    push    offset abstract_method_str
    call    _fatalError                                         ; yes, call fatalError(ABSTRACT_METHOD_ERROR)
    add     esp, 4
call_method_not_abstract:
    mov     dword ptr [_thisObjectGCSafe], ebx                  ; thisObjectGCSafe = thisObject
    push    eax                                                 ; save thisMethod
    push    ecx                                                 ; save ipIncrement
    VMSAVE                                                      ; save local vm regs
    push    eax
    call    _pushFrame                                          ; call pushFrame(thisMethod)
    add     esp, 4
    VMRESTORE                                                   ; restore local vm regs
    test    eax, eax                                            ; result == true?
    pop     ecx                                                 ; restore ecx = ipIncrement
    pop     eax                                                 ; restore eax = thisMethod
    je      call_method_done                                    ; no, skip to end
    mov     edx, dword ptr [_GlobalState + FP_OFFSET]           
    add     dword ptr [edx + FRAME_PREV_IP_OFFSET], ecx         ; yes, fp->previousIP += ipIncrement
    mov     edx, dword ptr [eax + METHOD_ACCESSFLAGS_OFFSET]    ; edx = thisMethod->accessFlags
    and     edx, ACC_SYNCHRONIZED                               ; ACC_SYNCHRONIZED bit set?
    je      call_method_done                                    ; no, skip to end
    mov     edx, dword ptr [_thisObjectGCSafe]                  ; edx = thisObjectSafe
    push    edx
    VMSAVE                                                      ; save local vm regs                            
    push    edx
    call    _monitorEnter                                       ; call monitorEnter(thisObjectGCSafe)                                       
    add     esp, 4
    VMRESTORE                                                   ; restore local vm regs
    pop     ecx                                                 ; ecx = thisObjectGCSafe
    mov     edx, dword ptr [_GlobalState + FP_OFFSET]
    add     dword ptr [edx + FRAME_SYNC_OFFSET], ecx            ; fp->syncObject = thisObjectGCSafe
call_method_done:
    mov     dword ptr [_thisObjectGCSafe], 0                    ; thisObjectGCSafe = null
    RESCHEDULE

; ------------------------ invokespecial_quick - 0xd7 ------------------------
asm_invokespecial_quick:    
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = cpIndex
    mov     eax, dword ptr [_GlobalState + CP_OFFSET]   ; eax = cp
    mov     eax, dword ptr [eax + 4 * ebx]              ; eax = thisMethod
    movzx   edx, word ptr [eax + METHOD_ARGCOUNT_OFFSET]; edx = thisMethod->argCount
    dec     edx                                         
    shl     edx, 2
    mov     ebx, java_sp
    sub     ebx, edx                                    ; ebx = sp - thisMethod->argCount + 1
    mov     ebx, dword ptr [ebx]                        ; ebx = thisObject
    test    ebx, ebx
    je      handleNullPointerException
    mov     ecx, 3                                      ; ecx = ipIncrement
    jmp     call_method

; ------------------------ invokestatic_quick - 0xd8 -------------------------
asm_invokestatic_quick: 
    STATIC_SETUP invokestatic, METHOD_CLASS_OFFSET
    mov     ecx, 3
    jmp     call_method

; ----------------------- invokeinterface_quick - 0xd8 -----------------------
asm_invokeinterface_quick:      
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = iCacheIndex
    mov     ecx, ebx
    shl     ebx, 3
    shl     ecx, 2
    add     ebx, ecx
    mov     ecx, dword ptr [_InlineCache]               ; ecx = InlineCache
    add     ecx, ebx                                    ; ecx = thisICache
    mov     eax, dword ptr [ecx]                        ; eax = thisMethod
    movzx   edx, byte ptr [java_ip + 3]                 ; edx = argCount
    dec     edx
    shl     edx, 2
    mov     ebx, java_sp
    sub     ebx, edx                                    ; ebx = sp - argCount + 1
    mov     ebx, dword ptr [ebx]                        ; ebx = thisObject
    test    ebx, ebx
    je      handleNullPointerException
    mov     edx, dword ptr [ebx + INSTANCE_CLASS_OFFSET]; edx = dynamicClass
    cmp     dword ptr [eax + METHOD_CLASS_OFFSET], edx  ; defaultClass == dynamicClass?
    je      invokeinterface_nolookup                    ; yes, skip method lookup
    push    ecx                                         ; no, save thisICache
    push    ebx                                         ; save thisObject
    mov     ecx, dword ptr [_GlobalState + FP_OFFSET]   ; push args
    mov     ecx, dword ptr [ecx + FRAME_METHOD_OFFSET]
    mov     ecx, dword ptr [ecx + METHOD_CLASS_OFFSET]
    push    ecx
    mov     ecx, dword ptr [eax + METHOD_KEY_OFFSET]
    push    ecx
    push    edx
    VMSAVE
    call    _lookupMethod                               ; call loopupMethod(
                                                        ;   dynamicClass, 
                                                        ;   thisMethod->nameTypeKey, 
                                                        ;   fp_global->thisMethod->ofClass
                                                        ; )
                                                        ;
                                                        ; upon return, eax holds new and 
                                                        ; improved thisMethod
    add     esp, 0ch
    VMRESTORE
    pop     ebx                                         ; restore thisObject
    pop     ecx                                         ; restore thisICache
    mov     dword ptr [ecx], eax                        ; thisICache->contents = thisMethod
invokeinterface_nolookup:
    test    eax, eax                                    ; thisMethod == null?
    je      icache_error                                ; yes, fatal error
    mov     edx, dword ptr [eax + METHOD_ACCESSFLAGS_OFFSET]; edx = thisMethod->accessFlags
    and     edx, ACC_PUBLIC_OR_STATIC
    cmp     edx, ACC_PUBLIC                             ; (thisMethod->accessFlags & (ACC_PUBLIC | ACC_STATIC)) == ACC_PUBLIC?
    jne     icache_error                                ; no, fatal error
    mov     ecx, 5                                      ; yes, ecx = ipIncrement
    jmp     call_method                                 ; call shared method invocation code

; expected parameters:
; ecx: thisICache
icache_error:
    push    ecx         
    call    _fatalIcacheMethodError
    add     esp, 4
    NEXT_BYTECODE

; ----------------------------- new_quick - 0xda -----------------------------
asm_new_quick:      
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = cpIndex
    mov     eax, dword ptr [_GlobalState + CP_OFFSET]   ; eax = cp
    mov     eax, dword ptr [eax + 4 * ebx]              ; eax = thisClass
    mov     ecx, dword ptr [eax + CLASS_STATUS_OFFSET]  ; ecx = class->status
    cmp     ecx, CLASS_READY
    je      new_initialized
    mov     ecx, dword ptr [eax + CLASS_THREAD_OFFSET]  ; ecx = class->initThread
    cmp     ecx, dword ptr [_CurrentThread]
    je      new_initialized
    VMSAVE
    push    eax
    call    _initializeClass
    add     esp, 4
    VMRESTORE
    RESCHEDULE
new_initialized:
    VMSAVE
    push    eax
    call    _instantiate
    add     esp, 4
    VMRESTORE
    test    eax, eax
    je      new_done
    add     java_sp, 4
    add     java_ip, 3
    mov     dword ptr [java_sp], eax
new_done:
    NEXT_BYTECODE

; -------------------------- anewarray_quick - 0xdb --------------------------
asm_anewarray_quick:    
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = iCacheIndex
    mov     ecx, ebx
    shl     ebx, 3
    shl     ecx, 2
    add     ebx, ecx
    mov     ecx, dword ptr [_InlineCache]               ; ecx = InlineCache
    add     ecx, ebx
    mov     ecx, dword ptr [ecx]                        ; ecx = thisClass
    mov     ebx, dword ptr [java_sp]                    ; ebx = arraylength
    VMSAVE
    push    ebx
    push    ecx
    call    _instantiateArray
    add     esp, 8
    VMRESTORE
    test    eax, eax
    je      anewarray_done
    add     java_ip, 3
    mov     dword ptr [java_sp], eax
anewarray_done:
    NEXT_BYTECODE

; ----------------------- multianewarray_quick - 0xdc ------------------------
asm_multianewarray_quick:
    movzx   ebx, word ptr [java_ip + 1]
    xchg    bh, bl                                      ; ebx = cpIndex
    mov     eax, dword ptr [_GlobalState + CP_OFFSET]   ; eax = cp
    mov     eax, dword ptr [eax + 4 * ebx]              ; eax = thisClass
    movzx   edx, byte ptr [java_ip + 3]                 ; edx = dimensions
    mov     ecx, edx
    dec     ecx
    shl     ecx, 2                                      ; ecx = (dimensions - 1) * 4
    push    ecx
    mov     ebx, java_sp
    sub     ebx, ecx                                    ; ebx = sp - dimensions + 1
    VMSAVE
    push    edx
    push    ebx
    push    eax
    call    _instantiateMultiArray
    add     esp, 12
    VMRESTORE
    pop     ecx
    test    eax, eax
    je      multianewarray_done
    sub     java_sp, ecx                                ; sp -= (dimensions - 1)
    add     java_ip, 4
    mov     dword ptr [java_sp], eax
multianewarray_done:
    NEXT_BYTECODE

; -------------------------- checkcast_quick - 0xdd --------------------------
asm_checkcast_quick:
    mov     eax, dword ptr [java_sp]            ; eax = objectref
    test    eax, eax
    je      checkcast_quick_done
    movzx   ebx, word ptr [java_ip + 1]         ; ebx = cpIndex
    xchg    bh, bl
    mov     ecx, dword ptr [_GlobalState + CP_OFFSET]
    mov     ecx, dword ptr [ecx + ebx * CP_ENTRY_SIZE + CP_ENTRY_CLASS_OFFSET]
    mov     eax, dword ptr [eax + INSTANCE_CLASS_OFFSET]
    push    ecx
    push    eax
    call    _isAssignableToFast
    pop     ebx
    pop     ecx
    test    eax, eax
    jne     checkcast_quick_done
    push    ecx
    push    ebx
    VMSAVE
    call    _isAssignableTo
    add     esp, 8
    VMRESTORE
    test    eax, eax
    je      handleClassCastException
checkcast_quick_done:
    add     java_ip, 3
    NEXT_BYTECODE
    
; ------------------------- instanceof_quick - 0xde --------------------------
asm_instanceof_quick:   
    mov     eax, dword ptr [java_sp]            ; eax = objectref
    test    eax, eax
    je      instanceof_quick_done
    movzx   ebx, word ptr [java_ip + 1]         ; ebx = cpIndex
    xchg    bh, bl
    mov     ecx, dword ptr [_GlobalState + CP_OFFSET]
    mov     ecx, dword ptr [ecx + ebx * CP_ENTRY_SIZE + CP_ENTRY_CLASS_OFFSET]
    mov     eax, dword ptr [eax + INSTANCE_CLASS_OFFSET]
    push    ecx
    push    eax
    call    _isAssignableToFast
    pop     ebx
    pop     ecx
    test    eax, eax
    jne     instanceof_quick_done
    push    ecx
    push    ebx
    VMSAVE
    call    _isAssignableTo
    add     esp, 8
    VMRESTORE
instanceof_quick_done:
    add     java_ip, 3
    mov     dword ptr [java_sp], eax
    NEXT_BYTECODE
    
; ---------------------------- customcode - 0xdf -----------------------------
asm_customcode:     
    mov     eax, dword ptr [_GlobalState + FP_OFFSET]
    add     eax, FP_INC
    mov     eax, dword ptr [eax]
    VMSAVE
    push    0
    call    eax
    add     esp, 4
    VMRESTORE
    RESCHEDULE
    
; ----------------------------------------------------------------------------
; Bytecodes we won't do in assembly because
; a) they deal with floating point (currently not enabled in kvm)
; b) they are (almost) never used
; c) they are too hard to do :)
; ----------------------------------------------------------------------------
asm_getstatic:
asm_putstatic:
asm_getfield:
asm_putfield:
asm_invokevirtual:
asm_invokespecial:
asm_invokestatic:
asm_invokeinterface:
asm_new:
asm_newarray:
asm_anewarray:
asm_checkcast:
asm_instanceof:
asm_multianewarray:
asm_jsr:
asm_jsr_w:
asm_ret:
asm_fconst_0:
asm_fconst_1:
asm_fconst_2:
asm_dconst_0:
asm_dconst_1:
asm_fload:
asm_dload:
asm_fload_0:
asm_fload_1:
asm_fload_2:
asm_fload_3:
asm_dload_0:
asm_dload_1:
asm_dload_2:
asm_dload_3:
asm_faload:
asm_daload:
asm_fstore:
asm_dstore:
asm_fstore_0:
asm_fstore_1:
asm_fstore_2:
asm_fstore_3:
asm_dstore_0:
asm_dstore_1:
asm_dstore_2:
asm_dstore_3:
asm_fastore:
asm_dastore:
asm_fadd:
asm_dadd:
asm_fsub:
asm_dsub:
asm_fmul:
asm_dmul:
asm_fdiv:
asm_ddiv:
asm_frem:
asm_drem:
asm_fneg:
asm_dneg:
asm_i2f:
asm_i2d:
asm_l2f:
asm_l2d:
asm_f2i:
asm_f2l:
asm_f2d:
asm_d2i:
asm_d2l:
asm_d2f:
asm_fcmpl:
asm_fcmpg:
asm_dcmpl:
asm_dcmpg:
asm_freturn:
asm_dreturn:
asm_unused_ba:
asm_unused_d5:
asm_unused_e0:
asm_unused_e1:
asm_unused_e2:
asm_unused_e3:
asm_unused_e4:
asm_unused_e5:
asm_unused_e6:
asm_unused_e7:
asm_unused_e8:
asm_unused_e9:
asm_unused_ea:
asm_unused_eb:
asm_unused_ec:
asm_unused_ed:
asm_unused_ee:
asm_unused_ef:
asm_unused_f0:
asm_unused_f1:
asm_unused_f2:
asm_unused_f3:
asm_unused_f4:
asm_unused_f5:
asm_unused_f6:
asm_unused_f7:
asm_unused_f8:
asm_unused_f9:
asm_unused_fa:
asm_unused_fb:
asm_unused_fc:
asm_unused_fd:
asm_unused_fe:
asm_unused_ff:
fload_wide:
dload_wide:
fstore_wide:
dstore_wide:
    CALL_SLOW_INTERPRET

; dispatch handling of individual bytecodes to the appropriate routines
Jumptable:
    dword asm_nop
    dword asm_aconst_null
    dword asm_iconst_m1
    dword asm_iconst_0
    dword asm_iconst_1
    dword asm_iconst_2
    dword asm_iconst_3
    dword asm_iconst_4
    dword asm_iconst_5
    dword asm_lconst_0
    dword asm_lconst_1
    dword asm_fconst_0
    dword asm_fconst_1
    dword asm_fconst_2
    dword asm_dconst_0
    dword asm_dconst_1
    dword asm_bipush
    dword asm_sipush
    dword asm_ldc
    dword asm_ldc_w
    dword asm_ldc2_w
    dword asm_iload
    dword asm_lload
    dword asm_fload
    dword asm_dload
    dword asm_aload
    dword asm_iload_0
    dword asm_iload_1
    dword asm_iload_2
    dword asm_iload_3
    dword asm_lload_0
    dword asm_lload_1
    dword asm_lload_2
    dword asm_lload_3
    dword asm_fload_0
    dword asm_fload_1
    dword asm_fload_2
    dword asm_fload_3
    dword asm_dload_0
    dword asm_dload_1
    dword asm_dload_2
    dword asm_dload_3
    dword asm_aload_0
    dword asm_aload_1
    dword asm_aload_2
    dword asm_aload_3
    dword asm_iaload
    dword asm_laload
    dword asm_faload
    dword asm_daload
    dword asm_aaload
    dword asm_baload
    dword asm_caload
    dword asm_saload
    dword asm_istore
    dword asm_lstore
    dword asm_fstore
    dword asm_dstore
    dword asm_astore
    dword asm_istore_0
    dword asm_istore_1
    dword asm_istore_2
    dword asm_istore_3
    dword asm_lstore_0
    dword asm_lstore_1
    dword asm_lstore_2
    dword asm_lstore_3
    dword asm_fstore_0
    dword asm_fstore_1
    dword asm_fstore_2
    dword asm_fstore_3
    dword asm_dstore_0
    dword asm_dstore_1
    dword asm_dstore_2
    dword asm_dstore_3
    dword asm_astore_0
    dword asm_astore_1
    dword asm_astore_2
    dword asm_astore_3
    dword asm_iastore
    dword asm_lastore
    dword asm_fastore
    dword asm_dastore
    dword asm_aastore
    dword asm_bastore
    dword asm_castore
    dword asm_sastore
    dword asm_pop
    dword asm_pop2
    dword asm_dup
    dword asm_dup_x1
    dword asm_dup_x2
    dword asm_dup2
    dword asm_dup2_x1
    dword asm_dup2_x2
    dword asm_swap
    dword asm_iadd
    dword asm_ladd
    dword asm_fadd
    dword asm_dadd
    dword asm_isub
    dword asm_lsub
    dword asm_fsub
    dword asm_dsub
    dword asm_imul
    dword asm_lmul
    dword asm_fmul
    dword asm_dmul
    dword asm_idiv
    dword asm_ldiv
    dword asm_fdiv
    dword asm_ddiv
    dword asm_irem
    dword asm_lrem
    dword asm_frem
    dword asm_drem
    dword asm_ineg
    dword asm_lneg
    dword asm_fneg
    dword asm_dneg
    dword asm_ishl
    dword asm_lshl
    dword asm_ishr
    dword asm_lshr
    dword asm_iushr
    dword asm_lushr
    dword asm_iand
    dword asm_land
    dword asm_ior
    dword asm_lor
    dword asm_ixor
    dword asm_lxor
    dword asm_iinc
    dword asm_i2l
    dword asm_i2f
    dword asm_i2d
    dword asm_l2i
    dword asm_l2f
    dword asm_l2d
    dword asm_f2i
    dword asm_f2l
    dword asm_f2d
    dword asm_d2i
    dword asm_d2l
    dword asm_d2f
    dword asm_i2b
    dword asm_i2c
    dword asm_i2s
    dword asm_lcmp
    dword asm_fcmpl
    dword asm_fcmpg
    dword asm_dcmpl
    dword asm_dcmpg
    dword asm_ifeq
    dword asm_ifne
    dword asm_iflt
    dword asm_ifge
    dword asm_ifgt
    dword asm_ifle
    dword asm_if_icmpeq
    dword asm_if_icmpne
    dword asm_if_icmplt
    dword asm_if_icmpge
    dword asm_if_icmpgt
    dword asm_if_icmple
    dword asm_if_acmpeq
    dword asm_if_acmpne
    dword asm_goto
    dword asm_jsr
    dword asm_ret
    dword asm_tableswitch
    dword asm_lookupswitch
    dword asm_ireturn
    dword asm_lreturn
    dword asm_freturn
    dword asm_dreturn
    dword asm_areturn
    dword asm_return
    dword asm_getstatic
    dword asm_putstatic
    dword asm_getfield
    dword asm_putfield
    dword asm_invokevirtual
    dword asm_invokespecial
    dword asm_invokestatic
    dword asm_invokeinterface
    dword asm_unused_ba
    dword asm_new
    dword asm_newarray
    dword asm_anewarray
    dword asm_arraylength
    dword asm_athrow
    dword asm_checkcast
    dword asm_instanceof
    dword asm_monitorenter
    dword asm_monitorexit
    dword asm_wide
    dword asm_multianewarray
    dword asm_ifnull
    dword asm_ifnonnull
    dword asm_goto_w
    dword asm_jsr_w                         ; c9
    dword asm_breakpoint                    ; ca
    dword asm_getfield_quick                ; cb
    dword asm_getfieldp_quick               ; cc
    dword asm_getfield2_quick               ; cd
    dword asm_putfield_quick                ; ce
    dword asm_putfield2_quick               ; cf
    dword asm_getstatic_quick               ; d0
    dword asm_getstaticp_quick              ; d1
    dword asm_getstatic2_quick              ; d2
    dword asm_putstatic_quick               ; d3
    dword asm_putstatic2_quick              ; d4
    dword asm_unused_d5                     ; d5
    dword asm_invokevirtual_quick           ; d6
    dword asm_invokespecial_quick           ; d7
    dword asm_invokestatic_quick            ; d8
    dword asm_invokeinterface_quick         ; d9
    dword asm_new_quick                     ; da
    dword asm_anewarray_quick               ; db
    dword asm_multianewarray_quick          ; dc
    dword asm_checkcast_quick               ; dd
    dword asm_instanceof_quick              ; de
    dword asm_customcode                    ; df
    dword asm_unused_e0
    dword asm_unused_e1
    dword asm_unused_e2
    dword asm_unused_e3
    dword asm_unused_e4
    dword asm_unused_e5
    dword asm_unused_e6
    dword asm_unused_e7
    dword asm_unused_e8
    dword asm_unused_e9
    dword asm_unused_ea
    dword asm_unused_eb
    dword asm_unused_ec
    dword asm_unused_ed
    dword asm_unused_ee
    dword asm_unused_ef
    dword asm_unused_f0
    dword asm_unused_f1
    dword asm_unused_f2
    dword asm_unused_f3
    dword asm_unused_f4
    dword asm_unused_f5
    dword asm_unused_f6
    dword asm_unused_f7
    dword asm_unused_f8
    dword asm_unused_f9
    dword asm_unused_fa
    dword asm_unused_fb
    dword asm_unused_fc
    dword asm_unused_fd
    dword asm_unused_fe
    dword asm_unused_ff
        
handleNullPointerException: 
    mov     eax, offset nullpointer_exception
    jmp     handleException
        
handleArrayIndexOutOfBoundsException: 
    mov     eax, offset arrayindexoutofbounds_exception
    jmp     handleException
        
handleArithmeticException: 
    mov     eax, offset arithmetic_exception
    jmp     handleException
        
handleArrayStoreException: 
    mov     eax, offset arraystore_exception
    jmp     handleException
        
handleClassCastException: 
    mov     eax, offset classcast_exception
    jmp     handleException

; expected parameters:
; eax: pointer to exception string
handleException:
    VMSAVE
    push    eax
    call    _raiseException
    add     esp, 4
    VMRESTORE
    RESCHEDULE

Interpreter_Done:
    pop     ebp
    ret
_FastInterpret endp

end
