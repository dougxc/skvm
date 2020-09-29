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
 * SUBSYSTEM: Interpreter stack frames
 * FILE:      frame.h
 * OVERVIEW:  Definitions for execution frame & exception handling
 *            manipulation.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (exception handling)
 *            Sheng Liang (chunky stacks), Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines the VM-specific internal runtime structures for
 * manipulating stack frames and performing the necessary exception
 * handling operations.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

/*
 * Number of cells reserved for native methods. Native method must push
 * a new frame when they use more than 3 stack slots.
 */
#define RESERVEDFORNATIVE 3

/* frame pointer => pointer to locals */
#define FRAMELOCALS(fp) ((cell*)(fp) - fp->thisMethod->frameSize)

/* Maximum number of stack and locals that a method can have. 
 * This limit is enforced by the loader 
 */
#define MAXIMUM_STACK_AND_LOCALS 512

/*=========================================================================
 * Chunky stack operations
 *=======================================================================*/

/*
 * KVM execution stacks can grow and shrink dynamically as 
 * necessary.  This allows us to create a large number of
 * Java threads even with very limited heap space. 
 *
 * The data structure for chunky stacks is defined below.
 * The physical Java execution stack consists of a linked
 * list of these structures.  This way, we don't have to 
 * allocate a huge contiguous memory area for each execution
 * stack.  Initially, each thread has just one stack chunk.
 * More chunks are created as necessary.
 */
/*  STACK */
struct stackStruct {
    STACK    next;
    short    size;
    short    xxunusedxx; /* must be multiple of 4 on all platforms */
    cell     cells[STACKCHUNKSIZE];
};

#define getStackHeight() (getSP() - ((cell*)getFP()) - SIZEOF_FRAME + 1)
#define setStackHeight(x) setSP(((cell *)getFP()) + (SIZEOF_FRAME - 1) + x)

#define STACK_CONTAINS(stack, p) \
  ((p) >= (stack)->cells && (p) < (stack)->cells + (stack)->size)

/*=========================================================================
 * COMMENT:
 * The overall stack frame organization for individual stack frames
 * is as follows (starting from the "bottom" of the execution stack
 * towards the "top" of the stack:
 * 1) Method call parameters + local variables
 * 2) The actual frameStruct (see below)
 * 3) Operand stack
 *
 * Note: there is no separate "operand stack" for data manipulation.
 * The necessary operands are always stored on top of the current
 * execution stack frame.  We sometimes use the term operand stack
 * to refer to the portion of the execution stack allocated for the
 * operands and data of the currently executing method.
 *
 * Remember that in virtual function calls, the first local variable
 * (zeroeth actually) always contains the self-reference ('this').
 * In static function calls, the first local variable contains the
 * first method parameter (if any).
 *=======================================================================*/

/*  FRAME (allocated inside execution stacks of threads) */
struct frameStruct {
    METHOD   thisMethod; /*  Pointer to the method currently under execution */
    FRAME    previousFp; /*  Stores the previous frame pointer */
    cell*    previousSp; /*  Stores the previous stack pointer */
    BYTE*    previousIp; /*  Stores the previous program counter */
    OBJECT   syncObject; /*  Holds monitor object if synchronized method call */
    STACK    stack;      /*  Stack chunk containing the frame */
};

/*  HANDLER */
struct exceptionHandlerStruct {
    /*  Note: in the current JVM Specification, all these values */
    /*  are actually shorts (16 bit values), limiting the maximum */
    /*  Java method size to 64k. This is a silly limitation that will */
    /*  probably necessitate changes in the classfile format later on */
    unsigned short startPC;   /* Start and end program counter indices; these */
    unsigned short endPC;     /* determine the code range where the handler is valid */
    unsigned short handlerPC; /* Location that is called upon exception */
    unsigned short exception; /* Class of the exception (as a constant pool index) */
                              /* Note: 0 in 'exception' indicates an 'any' handler */
};

struct exceptionHandlerTableStruct {
    long length;
    struct exceptionHandlerStruct handlers[1];
};

/*=========================================================================
 * Sizes for the above structures
 *=======================================================================*/

#define SIZEOF_FRAME             StructSizeInCells(frameStruct)
#define SIZEOF_HANDLER           StructSizeInCells(exceptionHandlerStruct)
#define SIZEOF_HANDLERTABLE(n) \
   (StructSizeInCells(exceptionHandlerTableStruct) + (n - 1) * SIZEOF_HANDLER)

/*=========================================================================
 * Stack frame execution modes (special return addresses)
 *=======================================================================*/

#define KILLTHREAD          ((BYTE*) 1)

/*=========================================================================
 * Iterators
 *=======================================================================*/

#define FOR_EACH_HANDLER(__var__, handlerTable) {                        \
     HANDLER __first_handler__ = handlerTable->handlers;                 \
     HANDLER __end_handler__ = __first_handler__ + handlerTable->length; \
     HANDLER __var__;                                                    \
     for (__var__ = __first_handler__; __var__ < __end_handler__; __var__++) {

#define END_FOR_EACH_HANDLER } }

/*=========================================================================
 * Operations on stack frames
 *=======================================================================*/

void pushFrame(METHOD thisMethod);
void popFrame(void);

/*=========================================================================
 * Stack frame printing and debugging operations
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printStackTrace(void);
void printExecutionStack(void);
#else
#  define printStackTrace()
#  define printExecutionStack()
#endif /* INCLUDEDEBUGCODE */

/*=========================================================================
 * Actual exception handling operations
 *=======================================================================*/

void throwException(THROWABLE_INSTANCE_HANDLE exception);

/*=========================================================================
 * Operations for raising exceptions and errors from within the VM
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * There are currently three ways by which the VM can report exceptions
 * and errors it has found during execution:
 *
 * - raiseException is the most elegant one, allowing the VM to report
 * errors via the normal Java exception handling mechanism.  It should
 * be used for reporting only those exceptions and errors which are
 * known to be "safe" and "harmless" from the VM's point of view
 * (i.e., the internal consistency and integrity of the VM should
 * not be endangered in any way).
 *
 * - fatalError is similar to raiseException, except that program
 * execution is terminated immediately. This function is used for
 * reporting those Java-level errors which are not caused by the VM,
 * but which might be harmful for the consistency of the VM.
 * There are currently quite a lot of fatalError calls inside the VM,
 * but in the long term it is expected that most of these could
 * be replaced with raiseException calls.
 *
 * - fatalVMError calls are used for reporting internal (in)consistency
 * problems or other VM-related errors. These errors typically indicate
 * that there are bugs in the VM.
 *
 * MODIFICATION: The exception handling mechanism of the VM has been altered
 *     for the version that will run on PalmOS. The catchable exceptions
 *     thrown by the VM have been narrowed down to:
 *
 *         NullPointerException
 *         IndexOutOfBoundsException
 *
 *     The remainder of the exceptions and errors thrown by the VM are not
 *     catchable. Instead, they display a message which includes the point
 *     where the exception occurred. The point of execution is in
 *     the form <classname>::<methodname>@<bytecode_address>. The bytecode
 *     address is relative to the first bytecode of the method (i.e. the
 *     same as given by javap -c).
 *
 *     One other thing to note is that any uncaught *catchable* exception
 *     (thrown either by the application or the VM) causes the VM to exit
 *     as there is no ThreadGroup class to handle uncaught exceptions.
 *
 *=======================================================================*/

#if __GNUC__
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif
 
void raiseException(const char* exceptionClassName) NORETURN;
void raiseExceptionCharMsg(const char* exceptionClassName,
                           const char* msg) NORETURN;

void fatalVMError(const char* errorMessage) NORETURN;
void fatalError(const char* errorMessage) NORETURN;

/*=========================================================================
 * Stack tracing operations
 *=======================================================================*/

void fillInStackTrace(THROWABLE_INSTANCE_HANDLE exception);
void printExceptionStackTrace(THROWABLE_INSTANCE_HANDLE exception);

/*=========================================================================
 * Exception handler debugging and tracing operations
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printExceptionHandlerTable(HANDLERTABLE handlerTable);
#else
#  define printExceptionHandlerTable(handlerTable)
#endif /* INCLUDEDEBUGCODE */

/*=========================================================================
 * Shortcuts to the Exceptions that the VM may throw
 *=======================================================================*/

extern const char NullPointerException[];
extern const char IndexOutOfBoundsException[];
extern const char ArrayIndexOutOfBoundsException[];
extern const char ArrayStoreException[];
extern const char ArithmeticException[];
extern const char ClassCastException[];
extern const char NegativeArraySizeException[];

/*=========================================================================
 * Shortcuts to the Errors that the VM may throw
 *
 * Note that the declarations have been indented to reflect the Error class
 * hierachy. Also, there are still a few Error classes which correspond to
 * sistuations better handled by fatalError in the KVM.
 *=======================================================================*/

extern const char LinkageError[];
extern const char   ClassCircularityError[];
extern const char   ClassFormatError[];
/*
extern const char     UnsupportedClassVersionError[];
*/
extern const char   ExceptionInInitializerError[];
extern const char   IncompatibleClassChangeError[];
extern const char     AbstractMethodError[];
extern const char     IllegalAccessError[];
extern const char     InstantiationError[];
extern const char     NoSuchFieldError[];
extern const char     NoSuchMethodError[];
extern const char   NoClassDefFoundError[];
extern const char   UnsatisfiedLinkError[];
extern const char   VerifyError[];
extern const char VirtualMachineError[];
extern const char   OutOfMemoryError[];
extern const char   StackOverflowError[];
/*
extern const char   InternalError[];
extern const char   UnknownError[];
 */

#if SVM
extern const char   TrustedClassFormatError[];
extern const char   IllegalSubclassError[];
extern const char   IllegalClassResourceAccessError[];
extern const char   IllegalDomainError[];
#endif /* SVM */

/*=========================================================================
 * Additional macros and definitions
 *=======================================================================*/

/* We call this function when we are doing excessive method tracing. */
#if INCLUDEDEBUGCODE
     void frameTracing(METHOD method, char *glyph, int offset);
#else
#    define frameTracing(method, glyph, offset)
#endif

#define POPFRAMEMACRO                                                           \
    setSP(getFP()->previousSp);    /*  Restore previous stack pointer */        \
    setIP(getFP()->previousIp);    /*  Restore previous instruction pointer */  \
    setFP(getFP()->previousFp);    /*  Restore previous frame pointer */        \
    setLP(FRAMELOCALS(getFP()));   /*  Restore previous locals pointer */       \
    setCP(getFP()->thisMethod->ofClass->constPool);

