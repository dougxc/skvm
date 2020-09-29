/*
 * Copyright 2000-2001 D'Crypt Pte Ltd
 * All rights reserved.
 */

/*=========================================================================
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * FILE:      machine_md.h (for DCryptor)
 * OVERVIEW:  This file is included in every compilation.  It contains
 *            definitions that are specific to the DCryptor port
 *            of the KVM.
 * AUTHOR:    K. H. Ong (original port for 1.0.2 KVM)
 *            Doug Simon (updated for 1.0.3 KVM codebase)
 * NOTE:      This file overrides many of the default compilation 
 *            flags and macros defined in VmCommon/h/main.h.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*
 * Instead of including the standard C header files, we reproduce the
 * declarations for the C functions required by the KVM. This will catch
 * the use of any other C functions that are not supported.
 */
#include <stddef.h>
#include <setjmp.h>

/* String manipulation */
char  *strcat(char *dest, const char *src);
char  *strchr(const char *s, int c);
int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strcpy(char *dest, const char *src);
char  *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *s);

/* Memory manipulation */
void  *memcpy(void *dest, const void *src, size_t n);
void  *memmove(void *dest, const void *src, size_t n);
void  *memset(void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);

/* Conversion/formating */
int    atoi(const char* nptr);
int    sprintf(char *str, const char *format, ...);
int    sscanf(const char *str, const char *format, ...);

/*
 * On DCryptor, all output goes to the UART port so file handles don't really
 * make sense. We simply need to provide place holders for them.
 */
#define LOGFILEPTR void*
#define stdout 0
#define stderr 0
#define fprintf __dcrypt_fprintf
#define putchar __dcrypt_putchar

int    __dcrypt_fprintf(LOGFILEPTR file, const char *formatStr, ...);
void   __dcrypt_putchar(int c);

/*=========================================================================
 * Platform-specific datatype definitions
 *=======================================================================*/

/*
 * The endianess definition comes from different places
 */
/*
 * Use i386 endianess settings from "VmUnix/h/machine.h" as these are
 * correct for both the device and the simulator
 */
#undef  BIG_ENDIAN
#undef  LITTLE_ENDIAN
#undef  NEED_LONG_ALIGNMENT
#define LITTLE_ENDIAN 1
#define NEED_LONG_ALIGNMENT 0

/*
 * The JAR file containing the Java .class files in stored in flash memory.
 * This flag determines if we access the contents of this JAR
 * directly via standard pointer arithmetic or if the JAR is first
 * transferred into RAM. Setting this to 1 requires an appropriate 'map'
 * directive in the DCryptor build spec file if we are not building the
 * simulator.
 */
#define MMAPPED_JAR 1

/* Compiler supports 64-bit arithmetic */
#define COMPILER_SUPPORTS_LONG	1

typedef long long		        long64;   /*  64-bit signed integer type */
typedef unsigned long long	ulong64;  /*  64-bit unsigned integer type */

#ifndef Calendar_md
unsigned long * Calendar_md(void);
#endif

/*
 * This is the size of the garbage collected heap on the DCryptor. By
 * declaring the size in terms of cells, we get word alignment for free by
 * allocating memory as an array of ints (i.e. word-sized quantities).
 *
 * This size is determined by investigating the log file produced by the
 * DCryptor build tool. The approach taken is to set it to 1, build the KVM
 * image and then replace it with the maximum amount of spare RAM
 * determined by looking at the log file.
 */
#ifndef HEAP_SIZE_IN_CELLS
#define HEAP_SIZE_IN_CELLS (1024 * (200) / CELL)
#endif

/*
 * The classes available to the VM are in a single JAR file that resides as a
 * predetermined address in the flash ROM of the DCryptor device. The ROM on
 * the DCryptor is implemented using high-density Flash ROMs that are divided
 * into 32 blocks of 128KB each (giving 4Mb per ROM). The first block is
 * occupied by the bootstrap code. The KVM application will occupy blocks 1
 * .. n where n is the smallest number of blocks that will store the
 * complete KVM image. The remaining blocks (i.e. (n+1) .. 31 are available
 * for storing the class files (in a JAR).
 */ 
#ifndef FLASH_BLOCK_COUNT
#define FLASH_BLOCK_COUNT   32
#endif
#define FLASH_BLOCK_SIZE   (1024 * 128)
#define FLASH_READ_SIZE     128

#define JAR_FLASH_BLOCK     6  /* Conservative - should be decreased to
                                  minimum for production build */
#define JAR_FLASH_ADDRESS  (FLASH_BASE + (JAR_FLASH_BLOCK * FLASH_BLOCK_SIZE))
#define MAX_JAR_SIZE       (FLASH_BLOCK_COUNT - JAR_FLASH_BLOCK) * FLASH_BLOCK_SIZE 

/*=========================================================================
 * Compilation flags and macros that override values defined in main.h
 *=======================================================================*/

/* Turn class prelinking/preloading (JavaCodeCompact) support on */
#ifndef ROMIZING
#define ROMIZING		1
#endif

/* Turn CLASSPATH support on */
#define USES_CLASSPATH		0

/* Exclude debugging and logging code
#define INCLUDEDEBUGCODE	0
*/

/* Turn runtime bytecode replacement and method inline caching on */
#define ENABLEFASTBYTECODES	1

/* Override the sleep function defined in main.h */
void timer_delay_ms(unsigned int delay);
#define SLEEP_UNTIL(wakeupTime)                                           \
    {  unsigned int delta = (unsigned int)(wakeupTime - CurrentTime_md());\
       timer_delay_ms(delta);                                          \
    }

/*=========================================================================
 * Platform-specific macros and function prototypes
 *=======================================================================*/

/*
 * This function provides 'gets' type of functionality over the UART port.
 */
int uart_getline(char* buffer, int size);

/*
 * These definitions enable redirection of debug input/output (i.e. what
 * fprintf and get_line operate on). This is only required for the
 * dCryptor device until an appropriate debug cable is available.
 */

typedef void (*ConsolePutCFunctionPtr)(char);
typedef char (*ConsoleGetCFunctionPtr)(void);
extern ConsolePutCFunctionPtr ConsolePutCFunction;
extern ConsoleGetCFunctionPtr ConsoleGetCFunction;

/*
 *
 */
void InitializeNativeCode();
void FinalizeNativeCode();

/*
 * No initialization or finalization required.
 */
#define InitializeVM()
#define FinalizeVM()

/*
 * This zeros the heap.
 */
void freeHeap(void* theHeap);

/*
 * There is no window system on the DCryptor
 */
#define InitializeWindowSystem()
#define FinalizeWindowSystem()

/*
 * There is no dynamic memory management on the DCryptor
 */
#define allocateVirtualMemory_md(size) 0
#define freeVirtualMemory_md(address,size)
#define protectVirtualMemory_md(address,size,protection)

/*
 * The UART port requires no initialization or finalization
 */
#define InitializeStandardIO()
#define FinalizeStandardIO()
