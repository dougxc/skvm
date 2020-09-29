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
 * SUBSYSTEM: Global definitions
 * FILE:      global.h
 * OVERVIEW:  Global system-wide definitions.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Many others since then...
 *=======================================================================*/

/*=========================================================================
 * COMMENT:
 * This file contains declarations that do not belong to any
 * particular structure or subsystem of the VM. There are many
 * other additional global variable declarations in other files.
 *=======================================================================*/

/*=========================================================================
 * System include files
 *=======================================================================*/

/* The performs per-machine initialization */
#include <machine_md.h>

/*=========================================================================
 * Global compile-time constants and typedefs
 *=======================================================================*/

#undef  TRUE
#undef  FALSE

#define NIL   0

typedef enum {
    FALSE = 0,
    TRUE = 1
} bool_t;

/*=========================================================================
 * Global data type declarations
 *=======================================================================*/

/*=========================================================================
 * NOTE: These type declarations are not quite as portable as they
 * could be.  It might be useful to declare specific type names for
 * all Java-specific types (e.g., jint instead of normal int, jlong
 * instead of long64, and so forth).
 *=======================================================================*/

#define CELL  4        /* Size of a java word (= 4 bytes) */
#define log2CELL 2     /* Shift amount equivalent to dividing by CELL */
#define SHORTSIZE 2

typedef unsigned char     BYTE;
typedef unsigned long     cell;    /* Must be 32 bits long! */

/*=========================================================================
 * System-wide structure declarations
 *=======================================================================*/

typedef struct classStruct*         CLASS;
typedef struct instanceClassStruct* INSTANCE_CLASS;
typedef struct arrayClassStruct*    ARRAY_CLASS;

typedef struct objectStruct*        OBJECT;
typedef struct instanceStruct*      INSTANCE;
typedef struct arrayStruct*         ARRAY;
typedef struct stringInstanceStruct* STRING_INSTANCE;
typedef struct throwableInstanceStruct* THROWABLE_INSTANCE;
typedef struct internedStringInstanceStruct* INTERNED_STRING_INSTANCE;
typedef struct classInstanceStruct* CLASS_INSTANCE;

typedef struct byteArrayStruct*     BYTEARRAY;
typedef struct shortArrayStruct*    SHORTARRAY;
typedef struct pointerListStruct*   POINTERLIST;
typedef struct weakPointerListStruct* WEAKPOINTERLIST;

typedef struct fieldStruct*         FIELD;
typedef struct fieldTableStruct*    FIELDTABLE;
typedef struct methodStruct*        METHOD;
typedef struct methodTableStruct*   METHODTABLE;
typedef struct stackMapStruct*      STACKMAP;
typedef struct icacheStruct*        ICACHE;
typedef struct chunkStruct*         CHUNK;

typedef struct staticChunkStruct*   STATICCHUNK;
typedef struct threadQueue*         THREAD;
typedef struct javaThreadStruct*    JAVATHREAD;
typedef struct monitorStruct*       MONITOR;

typedef struct stackStruct*         STACK;

typedef struct frameStruct*            FRAME;
typedef struct exceptionHandlerStruct* HANDLER;
typedef struct exceptionHandlerTableStruct* HANDLERTABLE;
typedef struct filePointerStruct*      FILEPOINTER;
typedef union constantPoolEntryStruct* CONSTANTPOOL_ENTRY;
typedef struct constantPoolStruct*     CONSTANTPOOL;
typedef char* BYTES;

#if SVM
typedef struct digestStruct*               DIGEST;
typedef struct keyStruct*                  KEY;
typedef struct signatureStruct*            SIGNATURE;
typedef struct permitStruct*               PERMIT;
typedef struct pendingPermitsStruct*       PENDING_PERMITS;
typedef struct trustedClassStruct*         TRUSTED_CLASS;
typedef struct trustedCPEntryStruct*       TRUSTEDCPENTRY;

#endif /* SVM */

typedef FILEPOINTER        *FILEPOINTER_HANDLE;
typedef OBJECT             *OBJECT_HANDLE;
typedef INSTANCE           *INSTANCE_HANDLE;
typedef ARRAY              *ARRAY_HANDLE;
typedef BYTEARRAY          *BYTEARRAY_HANDLE;
typedef POINTERLIST        *POINTERLIST_HANDLE;
typedef WEAKPOINTERLIST    *WEAKPOINTERLIST_HANDLE;
typedef JAVATHREAD         *JAVATHREAD_HANDLE;
typedef BYTES              *BYTES_HANDLE;
typedef METHOD             *METHOD_HANDLE;
typedef FRAME              *FRAME_HANDLE;
typedef const char*        *CONST_CHAR_HANDLE;
typedef unsigned char*     *UNSIGNED_CHAR_HANDLE;
typedef STRING_INSTANCE    *STRING_INSTANCE_HANDLE;
typedef THROWABLE_INSTANCE *THROWABLE_INSTANCE_HANDLE;
typedef THREAD             *THREAD_HANDLE;
#if SVM
typedef KEY                *KEY_HANDLE;
typedef SIGNATURE          *SIGNATURE_HANDLE;
typedef DIGEST             *DIGEST_HANDLE;
typedef PERMIT             *PERMIT_HANDLE;
typedef PENDING_PERMITS    *PENDING_PERMITS_HANDLE;
#endif /* SVM */

#define BitSizeToByteSize(n)    (((n) + 7) >> 3)
#define ByteSizeToCellSize(n)   (((n) + (CELL - 1)) >> log2CELL)
#define StructSizeInCells(structName) ((sizeof(struct structName) + 3) >> 2)
#define UnionSizeInCells(structName) ((sizeof(union structName) + 3) >> 2)

/* Field and Method key types */
typedef unsigned short NameKey;
typedef unsigned short MethodTypeKey;
typedef unsigned short FieldTypeKey;

typedef union {
    struct {
        unsigned short nameKey;
        unsigned short typeKey; /* either MethodTypeKey or FieldTypeKey */
    } nt;
    unsigned long i;
} NameTypeKey;

/* Machines such as the Palm that use something other than FILE* for stdin
 * and stdout should redefine this in their machine_md.h
 */
#ifndef LOGFILEPTR
#define LOGFILEPTR FILE*
#endif

/*=========================================================================
 * Locally defined include files
 *=======================================================================*/

#include <messages.h>

#include <main.h>
#include <long.h>
#include <garbage.h>
#include <interpret.h>
#include <hashtable.h>

#if ENABLE_JAVA_DEBUGGER
#include <debuggerStreams.h>
#include <debugger.h>
#endif /* ENABLE_JAVA_DEBUGGER */

#include <class.h>
#include <thread.h>
#include <pool.h>
#include <fields.h>
#include <frame.h>
#include <loader.h>
#include <native.h>
#include <cache.h>
#if SVM
#include <crypto.h>
#include <crypto_provider.h>
#endif /* SVM */

#include <runtime.h>
#include <profiling.h>
#include <verifier.h>
#include <log.h>
#include <property.h>

/* The network protocol implementations.  These */
/* are not part of the CLDC Specification. */
#include <commProtocol.h>
#include <datagramProtocol.h>
#include <socketProtocol.h>

/*=========================================================================
 * Miscellaneous global variables
 *=======================================================================*/

/* Shared string buffer that is used internally by the VM */
extern char str_buffer[];

/* Requested heap size when starting the VM from command line */
extern long RequestedHeapSize;

/*=========================================================================
 * Global execution modes
 *=======================================================================*/

/*  Flags for toggling certain global modes on and off */
extern bool_t JamEnabled;
extern bool_t JamRepeat;

/*=========================================================================
 * Macros for controlling global execution tracing modes
 *=======================================================================*/

/* The following isn't really intended to be unreadable, but it simplifies
 * the maintenance of the various execution tracing flags.
 * 
 * NOTE: Logically these operations belong to VmExtra directory,
 * since this code is not useful for those ports that do not support
 * command line operation.  However, since this code is intimately
 * tied with the tracing capabilities of the core KVM, we'll keep
 * these definitions in this file for the time being.
 *
 * The intent is that you can call
 *    FOR_EACH_TRACE_FLAG(Macro_Of_Two_Arguments)
 * where Macro_Of_Two_Arguments is a two-argument macro whose first argument
 * is the name of a variable, and the second argument is the string the user
 * enters to turn on that flag.
 */
#define FOR_EACH_TRACE_FLAG(Macro) \
     FOR_EACH_ORDINARY_TRACE_FLAG(Macro) \
     FOR_EACH_DEBUGGER_TRACE_FLAG(Macro) \
     FOR_EACH_JAM_TRACE_FLAG(Macro) \
     FOR_EACH_SVM_TRACE_FLAG(Macro)

/* The ordinary trace flags are those included in any debugging build */
#if INCLUDEDEBUGCODE
#   define FOR_EACH_ORDINARY_TRACE_FLAG(Macro)  \
         Macro(tracememoryallocation,         "-traceallocation") \
         Macro(tracegarbagecollection,        "-tracegc") \
         Macro(tracegarbagecollectionverbose, "-tracegcverbose") \
         Macro(traceclassloading,             "-traceclassloading") \
         Macro(traceclassloadingverbose,      "-traceclassloadingverbose") \
         Macro(traceverifier,                 "-traceverifier") \
         Macro(tracestackmaps,                "-tracestackmaps") \
         Macro(tracebytecodes,                "-tracebytecodes") \
         Macro(tracemethodcalls,              "-tracemethods") \
         Macro(tracemethodcallsverbose,       "-tracemethodsverbose") \
         Macro(tracestackchunks,              "-tracestackchunks") \
         Macro(traceframes,                   "-traceframes") \
         Macro(traceexceptions,               "-traceexceptions") \
         Macro(traceevents,                   "-traceevents") \
         Macro(tracemonitors,                 "-tracemonitors") \
         Macro(tracethreading,                "-tracethreading") \
         Macro(tracenetworking,               "-tracenetworking") 
#else 
#    define FOR_EACH_ORDINARY_TRACE_FLAG(Macro)
#endif

/* The debugger tracing flags are those included only if we support KWDP */
#if INCLUDEDEBUGCODE && ENABLE_JAVA_DEBUGGER
#  define FOR_EACH_DEBUGGER_TRACE_FLAG(Macro)  \
               Macro(tracedebugger, "-tracedebugger") 
#else 
#  define FOR_EACH_DEBUGGER_TRACE_FLAG(Macro)
#endif

/* The debugger tracing flags are those included only if we support KWDP */
#if INCLUDEDEBUGCODE && USE_JAM
#  define FOR_EACH_JAM_TRACE_FLAG(Macro)  \
               Macro(tracejam, "-tracejam") 
#else 
#  define FOR_EACH_JAM_TRACE_FLAG(Macro)
#endif

/* Declare each of the trace flags to be external.  Note that we define
 * a two-variable macro, then use that macro as an argument to
 * FOR_EACH_TRACE_FLAG 
 */

#if SVM
#  define FOR_EACH_SVM_TRACE_FLAG(Macro)  \
          Macro(tracesvm,"-tracesvm")
#else
#  define FOR_EACH_SVM_TRACE_FLAG(Macro)
#endif /* SVM */
               
#define DECLARE_TRACE_VAR_EXTERNAL(varName, userName) \
    extern int varName;
FOR_EACH_TRACE_FLAG(DECLARE_TRACE_VAR_EXTERNAL)

/*=========================================================================
 * Most frequently called functions are "inlined" here
 *=======================================================================*/

/*=========================================================================
 * Quick operations for stack manipulation (for manipulating stack
 * frames and operands).
 *=======================================================================*/

#define topStack                        (*getSP())
#define secondStack                     (*(getSP()-1))
#define thirdStack                      (*(getSP()-2))
#define fourthStack                     (*(getSP()-3))

#define topStackAsType(_type_)          (*(_type_ *)(getSP()))
#define secondStackAsType(_type_)       (*(_type_ *)(getSP() - 1))
#define thirdStackAsType(_type_)        (*(_type_ *)(getSP() - 2))
#define fourthStackAsType(_type_)       (*(_type_ *)(getSP() - 3))

#define oneMore                         getSP()++
#define oneLess                         getSP()--
#define moreStack(n)                    getSP() += (n)
#define lessStack(n)                    getSP() -= (n)

#define popStack()                      (*getSP()--)
#define popStackAsType(_type_)          (*(_type_ *)(getSP()--))

#define pushStack(data)                 *++getSP() = (data)
#define pushStackAsType(_type_, data)   *(_type_ *)(++getSP()) = (data)

/*=========================================================================
 * The equivalent macros to the above for use when the stack being
 * manipulated is not for the currently executing thread. In all cases
 * the additional parameter is the THREAD pointer.
 *=======================================================================*/

#define topStackForThread(t)         (*((t)->spStore))
#define secondStackForThread(t)      (*((t)->spStore - 1))
#define thirdStackForThread(t)       (*((t)->spStore - 2))
#define fourthStackForThread(t)      (*((t)->spStore - 3))

#define popStackForThread(t)         (*((t)->spStore--))
#define pushStackForThread(t, data)  (*(++(t)->spStore) = (data))
#define popStackAsTypeForThread(t, _type_) \
                                     (*(_type_*)((t)->spStore--))
#define pushStackAsTypeForThread(t, _type_, data)    \
                                     (*(_type_ *)(++(t)->spStore) = (data))
#define moreStackForThread(t, n)     ((t)->spStore += (n))
#define lessStackForThread(t, n)     ((t)->spStore -= (n))
#define oneMoreForThread(t)          ((t)->spStore++)
#define oneLessForThread(t)          ((t)->spStore--)

#define spForThread(t)               ((t)->spStore)

/*=========================================================================
 * Exception handling mechanism definitions
 *
 * 15/11/01: Doug Simon
 *     This mechanism has been extended so that Throwable objects
 *     can now be thrown. This is to support (near) complete JLS error
 *     semantics.
 *
 *     There is still only one catch statement and as such, polymorphic
 *     exception handling has to be implemented manually.
 *=======================================================================*/

/*
 * This struct models the nesting of try/catch scopes.
 */
struct throwableScopeStruct {
    /*
     * State relevant to the platform dependent mechanism used to
     * implement the flow of control (e.g. setjmp/longjmp).
     */
    jmp_buf*   env;
    /*
     * A THROWABLE_INSTANCE object.
     */
    THROWABLE_INSTANCE throwable;
    /*
     * The number of temporary roots at the point of TRY.
     */
    int     tmpRootsCount;
    /*
     * The enclosing try/catch (if any).
     */
    struct throwableScopeStruct* outer;
};

typedef struct throwableScopeStruct* THROWABLE_SCOPE;

/*
 * This warning is taken from the cexcept package
 * (http://www.cs.berkeley.edu/~amc/cexcept) as it accurately reflects the
 * semantics of TRY.
 * 
 * IMPORTANT: Jumping into or out of a Try clause (for example via
 * return, break, continue, goto, longjmp) is forbidden--the compiler
 * will not complain, but bad things will happen at run-time.  Jumping
 * into or out of a Catch clause is okay, and so is jumping around
 * inside a Try clause.  In many cases where one is tempted to return
 * from a Try clause, it will suffice to use Throw, and then return
 * from the Catch clause.  Another option is to set a flag variable and
 * use goto to jump to the end of the Try clause, then check the flag
 * after the Try/Catch statement.
 */
#if INCLUDEDEBUGCODE
#define TRACE_EXCEPTION(name) \
if (traceexceptions) \
{ \
    int level = 0; \
    struct throwableScopeStruct* s = ThrowableScope; \
    while (s->outer != NULL) { \
        s = s->outer; \
        level++; \
    } \
    s = ThrowableScope; \
    fprintf(stdout, \
        "%s(%d):\tenv: 0x%lx, throwable: 0x%lx, tmpRootsCount: %d\n", \
        name, \
        level,(long)s->env,(long)s->throwable,s->tmpRootsCount); \
}
/*
 * There must not be any allocation guard active when entering a TRY block
 * as an exception throw will always cause at least one allocation (for the
 * exception object) thus potentially invalidating any pointers being
 * guarded by the allocation guard. Note however that this does not preclude
 * using an allocation guard inside a TRY block.
 */
#define ASSERT_NO_ALLOCATION_GUARD \
    if (NoAllocation > 0) { \
        fatalError(KVM_MSG_TRY_BLOCK_ENTERED_WHEN_ALLOCATION_FORBIDDEN); \
    }
#else
#define TRACE_EXCEPTION(name)
#define ASSERT_NO_ALLOCATION_GUARD
#endif

#define TRY                                                    \
    {                                                          \
        struct throwableScopeStruct __scope__;                 \
        int __state__;                                         \
        jmp_buf __env__;                                       \
        __scope__.outer = ThrowableScope;                      \
        ThrowableScope = &__scope__;                           \
        ThrowableScope->env = &__env__;                        \
        ThrowableScope->tmpRootsCount = TemporaryRootsLength;  \
        ASSERT_NO_ALLOCATION_GUARD                             \
        TRACE_EXCEPTION("TRY")                                 \
        if ((__state__ = setjmp(__env__)) == 0) {
       
       
/*
 * Any non-null THROWABLE_INSTANCE passed into a CATCH clause
 * is protected as a temporary root.
 */
#define CATCH(__throwable__)                                   \
        }                                                      \
        TRACE_EXCEPTION("CATCH")                               \
        ThrowableScope = __scope__.outer;                      \
        TemporaryRootsLength = __scope__.tmpRootsCount;        \
        if (__state__ != 0) {                                  \
            START_TEMPORARY_ROOTS                              \
                 DECLARE_TEMPORARY_ROOT(THROWABLE_INSTANCE,    \
                     __throwable__,__scope__.throwable);

#define END_CATCH                                              \
            END_TEMPORARY_ROOTS                                \
        }                                                      \
    }

/*
 * This macro is required for jumping out of a CATCH block with a goto.
 * This is used in FastInterpret so that the interpreter loop is re-entered
 * with a fresh TRY statement.
 */
#define END_CATCH_AND_GOTO(label)                              \
            END_TEMPORARY_ROOTS                                \
            goto label;                                        \
        }                                                      \
    }


#define THROW(__throwable__)                                   \
    {                                                          \
        THROWABLE_INSTANCE __t__ = __throwable__;              \
        TRACE_EXCEPTION("THROW")                               \
        if (__t__ == NULL)                                     \
            fatalVMError("THROW called with NULL");            \
        ThrowableScope->throwable = __t__;                     \
        longjmp(*((jmp_buf*)ThrowableScope->env),1);           \
    }

/*=========================================================================
 * This is a non-nesting use of setjmp/longjmp used solely for the purpose
 * of exiting the VM in a clean way. By separating it out from the
 * exception mechanism above, we don't need to worry about whether or not
 * we are throwing an exception or exiting the VM in an CATCH block.
 *=======================================================================*/

#define VM_START                                               \
    {                                                          \
        jmp_buf __env__;                                       \
        VMScope = &(__env__);                                  \
        if (setjmp(__env__) == 0) {
        
#define VM_EXIT(__code__)                                      \
        VMExitCode = __code__;                                 \
        longjmp(*((jmp_buf*)VMScope),1)
        
#define VM_FINISH(__code__)                                    \
        } else {                                               \
            int __code__ = VMExitCode;
            
#define VM_END_FINISH                                          \
        }                                                      \
    }

extern void* VMScope;
extern int   VMExitCode;
extern THROWABLE_SCOPE ThrowableScope;

#ifndef FATAL_ERROR_EXIT_CODE
#define FATAL_ERROR_EXIT_CODE 127
#endif

#ifndef UNCAUGHT_EXCEPTION_ERROR_EXIT_CODE
#define UNCAUGHT_EXCEPTION_EXIT_CODE 128
#endif

/*=========================================================================
 * Operations for handling memory fetches and stores
 *=======================================================================*/

/*=========================================================================
 * These macros define Java-specific memory read/write operations
 * for reading high-endian numbers.
 *=======================================================================*/

/* Get a 32-bit value from the given memory location */
#define getCell(addr) \
                  ((((long)(((unsigned char *)(addr))[0])) << 24) |  \
                   (((long)(((unsigned char *)(addr))[1])) << 16) |  \
                   (((long)(((unsigned char *)(addr))[2])) << 8)  |  \
                   (((long)(((unsigned char *)(addr))[3])) << 0)) 

#if BIG_ENDIAN
#define getAlignedCell(addr) (*(long *)(addr))
#else 
#define getAlignedCell(addr) getCell(addr)
#endif

/* Get an unsigned 16-bit value from the given memory location */
#define getUShort(addr) \
                  ((((unsigned short)(((unsigned char *)(addr))[0])) << 8)  | \
                   (((unsigned short)(((unsigned char *)(addr))[1])) << 0))

/* Get a 16-bit value from the given memory location */
#define getShort(addr) ((short)(getUShort(addr)))

/* Store a 16-bit value in the given memory location */ 
#define putShort(addr, value) \
              ((unsigned char *)(addr))[0] = (unsigned char)((value) >> 8); \
              ((unsigned char *)(addr))[1] = (unsigned char)((value) & 0xFF)


