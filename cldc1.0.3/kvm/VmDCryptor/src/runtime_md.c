/*
 * Copyright 2000-2001 D'Crypt Pte Ltd
 * All rights reserved.
 *
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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: DCryptor-specific functions needed by the virtual machine
 * FILE:      runtime_md.c
 * AUTHOR:    K. H. Ong
 *            Doug Simon - completed port and added simulation functions.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*
 * Defines: vsnprintf, fprintf, putchar, stdout, stderr
 * NOTE: The last 4 symbols are redefined in <machine_md.h> and
 * as such they need to be undefined before including <machine_md.h> (via
 * <global.h>) to prevent conflicts.
 */
#include <stdio.h>
#undef stdout
#undef stderr
#undef fprintf
#undef putchar

/*
 * This is for our own implementation of fprintf.
 */
#include <stdarg.h>

/*
 * The DCryptor libraries.
 */
#include <rtc.h>
#include <uart.h>
#include <timer.h>
#include <noisesrc.h>
#include <flash.h>
#include <fx.h>

/*
 * KVM wide definitions.
 */
#include <global.h>

/*=========================================================================
 * Helper variables and methods
 *=======================================================================*/

#define MAXCALENDARFLDS 15

#define YEAR 1
#define MONTH 2
#define DAY_OF_MONTH 5
#define HOUR 10
#define MINUTE 12
#define SECOND 13
#define MILLISECOND 14

static unsigned long date[MAXCALENDARFLDS];

/* Size of the buffer used to construct the string for fprintf. */
#define FPRINTF_BUFFER_SIZE   512

#ifdef SIMULATOR
/*
 * Timer functions are currently implemented in terms of processor boot up
 * time. This has to be simulated by a function call.
 */
ulong64 getTimeSinceStartup(); 
#endif /* SIMULATOR */


/*=========================================================================
 * FUNCTION:      AlertUser()
 * TYPE:          error handling operation
 * OVERVIEW:      Show an alert dialog to the user.
 * INTERFACE:
 *   parameters:  message string
 *   returns:     <nothing>
 *=======================================================================*/

void AlertUser(const char* message)
{
    fprintf(0, "ALERT: %s\n", message);
}

/*=========================================================================
 * FUNCTION:      allocateHeap()
 * TYPE:          allocates memory 
 * OVERVIEW:      Allocate a chunk of memory for the GC heap. Size is given
 *                in bytes.
 * INTERFACE:
 *   parameters:  *sizeptr:  INPUT:   Pointer to size of heap to allocate
 *                           OUTPUT:  Pointer to actually size of heap
 *                *realresultptr: Returns pointer to actual pointer than
 *                           was allocated, before any adjustments for
 *                           memory alignment.  This is the value that
 *                           must be passed to "free()"
 * 
 *  returns:      pointer to aligned memory.
 *
 *  Note that "sizeptr" reflects the size of the "aligned" memory, and
 *  note the actual size of the memory that has been allocated.
 *
 *  DCryptor Note: the input values of the parameters are ignored as the
 *  heap is simply a static array on this platform.
 *=======================================================================*/

/*
 * A static array is used for the heap.
 */
static const cell TheHeap[HEAP_SIZE_IN_CELLS];
cell *allocateHeap(long *sizeptr, void **realresultptr) { 
    void *space = (void*)TheHeap;
    *realresultptr = space;
    /*
     * No need for alignment - heap was automatically allocated on a word
     * boundary.
     */
    *sizeptr = HEAP_SIZE_IN_CELLS * 4;
    return space;
}

/*=========================================================================
 * FUNCTION:      freeHeap()
 * TYPE:          memory management
 * OVERVIEW:      Zeroes the static array assigned as the heap.
 * INTERFACE:     theHeap - must be the static array return by
 *                allocateMemory
 *  returns:      <nothing>
 *
 *  Note that "sizeptr" reflects the size of the "aligned" memory, and
 *  note the actual size of the memory that has been allocated.
 *=======================================================================*/

void freeHeap(void* theHeap)
{
    if (theHeap != TheHeap)
        AlertUser("Heap was corrupted!");
    /*
     * Zeroize the heap.
     */
    memset(theHeap,0,HEAP_SIZE_IN_CELLS * 4);
}

/*=========================================================================
 * FUNCTION:      CurrentTime_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Returns the current time. On the DCryptor this will be
 *                the number of millisseconds since the processor was
 *                started.
 * INTERFACE:
 *   parameters:  none
 *   returns:     current time, in milliseconds since startup
 *=======================================================================*/
ulong64
CurrentTime_md(void)
{
#ifdef SIMULATOR
    ulong64 time_ms = getTimeSinceStartup();
#else
    ulong64 time_ms = ostimer.oscr / TICKS_PER_MS;
#endif /* SIMULATOR */
//fprintf(0,"CurrentTime_md: %lld\n",time_ms);
	  return time_ms ;
}

/*=========================================================================
 * FUNCTION:      Calendar_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Initializes the calendar fields, which represent the 
 *                Calendar related attributes of a date. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

unsigned long *
Calendar_md(void)
{
    /* initialize the date array */
    memset(&date, 0, MAXCALENDARFLDS);

    /* returns the time */ 
    rtc_get_time();

    /* initialize the calendar fields */
    date[YEAR] = rtc_time_shared.yr;
    date[MONTH] = rtc_time_shared.mth;
    date[DAY_OF_MONTH] = rtc_time_shared.date;
    date[HOUR] = rtc_time_shared.hrs;
    date[MINUTE] = rtc_time_shared.mins;
    date[SECOND] = rtc_time_shared.secs;
    /*
     * Adjust for AM/PM if necessary
     */
    if (rtc_time_shared.mode_12hr && rtc_time_shared.mode_pm)
        date[HOUR] += 12;
    /* We cannot accurately determine this value,
     * so set it to 0 for now
     */
    date[MILLISECOND] = 0;
    return date;
}

/*=========================================================================
 * FUNCTION:      RandomNumber_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Returns a random number.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

long RandomNumber_md() {
    long result = 0;
    int i;
    for (i = 0; i != sizeof(result); ++i)
        result |= ((ns_get_byte() & 0xFF) << (i*8));
    return result;
}
    
/*=========================================================================
 * FUNCTION:      The stub of GetAndStoreNextKVMEvent
 * TYPE:          event handler
 * OVERVIEW:      Wait for an external event.
 *  returns:      void
 *=======================================================================*/

void GetAndStoreNextKVMEvent(bool_t forever, ulong64 waitUntil) {
    /*
     * No events defined yet for the DCryptor.
     */
}

/*=========================================================================
 * FUNCTION:      InitializeNativeCode
 * TYPE:          initialization
 * OVERVIEW:      called at start up to perform machine-specific initialization. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void InitializeNativeCode() {
    /*
     * Initialize noise source for random number generation.
     * Currently crashes the card!!!
     */
    ns_init();
    /*
     * Initialize the Real Time Clock
     */
    rtc_init();
    /*
     * Start time-keeping.
     */
    rtc_start_oscr();
}

/*=========================================================================
 * FUNCTION:      nativeFinalization
 * TYPE:          initialization
 * OVERVIEW:      called at shut down to perform machine-specific
 *                initialization. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/
void FinalizeNativeCode() { 
    rtc_stop_oscr();
}

/*======================================================================
 * FUNCTION:    putchar
 * DESCRIPTION: Implements the standard putchar functionality except
 *              all output is written to the UART port.
 * PARAMETERS:  c - character to output
 * RETURNS:     <nothing>
 *====================================================================*/

ConsolePutCFunctionPtr ConsolePutCFunction = uart_putc;
ConsoleGetCFunctionPtr ConsoleGetCFunction = uart_getc;

void __dcrypt_putchar(int c) {
#ifndef SIMULATOR
    if (c == '\n')
        (*ConsolePutCFunction)('\r');
#endif /* SIMULATOR */
    (*ConsolePutCFunction)((char)c);
}

/*======================================================================
 * FUNCTION:    fprintf
 * DESCRIPTION: Implements the standard fprintf functionality except
 *              all output is written to the UART port.
 * PARAMETERS:  printf parameters
 * RETURNS:     number of characters output
 *====================================================================*/

int __dcrypt_fprintf(LOGFILEPTR ignore, const char *format, ... )
{
    va_list  args;
    char     tmpBuffer[FPRINTF_BUFFER_SIZE];
    int      strLen, i;
    
    /*  Use the variable argument macros and format the printf string */
    va_start(args, format);
    strLen = vsnprintf(tmpBuffer,FPRINTF_BUFFER_SIZE,format,args);
    va_end(args);

    /*
     * According to the man page, vsnprintf will return -1 if the given buffer
     * would have overflowed. In this case, only exactly the size of the
     * buffer characters were written.
     */
    if (strLen == -1)
        strLen = FPRINTF_BUFFER_SIZE;
    
    /*
     * Now put it out the UART port using putchar so that LF (i.e. '\r's
     * are inserted as necessary.
     */
    for (i = 0; i != strLen; ++i)
        __dcrypt_putchar(*(tmpBuffer+i));
    
    return strLen;
}

/*======================================================================
 * FUNCTION:    uart_getline
 * DESCRIPTION: Reads in at most one less than size characters
 *              from the UART port and stores them into the buffer
 *              pointed to  by buf. Reading stops after a newline.
 *              The newline is not put into the buffer. A '\0'  is
 *              stored after the last character in the buffer.
 *
 *              This is very similiar to the functionality of fgets except
 *              for the handling of newlines.
 * PARAMETERS:  buf - the buffer to write into
 *              size - the maximum number of characters to read
 * RETURNS:     number of characters in buf (excluding terminating 0).
 *====================================================================*/
#ifdef SIMULATOR
#define    RETURN_CHAR '\n'
#else
#define    RETURN_CHAR '\r'
#endif /* SIMULATOR */
int uart_getline(char* buf, int size) {
    char *ch = buf;
    while ((ch - buf) < size &&
           ((*ch) = (*ConsoleGetCFunction)()) != RETURN_CHAR) {
        /*
         * Echo the character.
         */
        putchar(*ch);
        ch++;
    }
    /*
     * Echo newline.
     */
    putchar('\n');
    /*
     * Replace the newline (or last character in case of buffer fill) with
     * terminating 0.
     */
    *ch = 0;
    return ch - buf;
}


#ifdef SIMULATOR
/*=========================================================================
 * Simulator functions. These are the functions that simulate the
 * functionality of the DCryptor device.
 *=======================================================================*/

#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#undef mprotect
#define mprotect(a,b,c) 0

#include <limits.h>    /* for PAGESIZE */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

/*
 * Given that we are emulating a device with unbuffered IO, we use
 * ncurses to implement the same functionality over a (normally buffered)
 * UNIX terminal.
 */
#include <ncurses.h>

/*
 * DCrypt libraries required by other files.
 */
#include <audio.h>
#include <melp.h>
#include <uart1.h>
#include <uart5.h>

/*=========================================================================
 * Simulator definitions
 *=======================================================================*/

/*
 * This is required to simulate the Flash on the device.
 */
#define TOTAL_FLASH_SIZE  (FLASH_BLOCK_SIZE * FLASH_BLOCK_COUNT)

/*
 * The flash is emulated by a word-aligned dynamically allocated array of
 * the appropriate size.
 */
static void* RealFlash = 0;
static unsigned char* Flash = 0;
unsigned char* JarFlashBlock = 0; /* declared extern in loader_md.c */

/*
 * All output can also be logged to the file defined below. Comment out
 * this define if you don't want this extra logging.
 */
#define SIMULATION_LOG_FILE "execution.log"
#ifdef  SIMULATION_LOG_FILE
static FILE* SimulatorLogFile = 0;
#endif /* SIMULATION_LOG_FILE */

/*
 * For the purpose of CurrentTime_md, we need to keep track of how much
 * time has elapsed between since this process started.
 */
static struct timeval ProcessStart;

/*
 * The shared memory structure through which communication with the Real
 * Time Clock module is performed.
 */
RTC_TIME rtc_time_shared;

/*
 * This supports the timer functions in timer.h.
 */
static ulong64 TimerArray[NUM_TIMER_IDS];


/*========================================================================
 * Miscellaneous helper functions.
 *======================================================================*/

/*
 * Helper for processing command line arguments that take a parameter.
 */
static char* getParam(char** argv, int index, int argc) {
    if (index >= argc) {
        printf("Command line option %s requires an argument.\n",
            argv[index-1]);
        exit(1);
    }
    return argv[index];
}

/*
 * Helper for getting a file name and opening the corresponding file.
 */
#define FILENAME_SIZE 1024
static FILE* promptForFile(const char* mode, char** name, int* size) {
    FILE* fp;
    static char fileName[FILENAME_SIZE];
    struct stat sbuf;
    
    fprintf(0,"Enter name of file: ");
    uart_getline(fileName,FILENAME_SIZE);
        
    if (stat(fileName,&sbuf) != 0) {
        printf("Couldn't stat file '%s': %s\n",
                         fileName,strerror(errno));
        return NULL;
    }
    if ((fp = fopen(fileName,mode)) == NULL) {
        printf("Couldn't open file: %s\n",strerror(errno));
        return NULL;
    }
    if (name != NULL)
        strcpy(*name,fileName);
    if (size != NULL)
        *size = sbuf.st_size;
    return fp;
}

/*========================================================================
 * The UART ports on a DCryptor device can be emulated in different ways.
 * Those that will typically be connected to a terminal-type interface
 * (e.g. HyperTerminal) can be emulated with ncurses terminals. Others
 * that are data pipes can be emulated with files. The structures and
 * functions below implement the state and functionality of these types of
 * emulation.
 *======================================================================*/

/*========================================================================
 * NCurses Emulation.
 *======================================================================*/

enum {
    UART_Terminal,
    UART_File
};

struct uartStruct {
    int     type;
    bool_t  initialised;
    bool_t  enabled;
};
    
struct uartTerminalStruct {
    struct uartStruct base;

    WINDOW* window;
    SCREEN* screen;
};

struct uartFileStruct {
    struct uartStruct base;
    
    FILE* out;
    struct {
        FILE*  fp;
        char   name[FILENAME_SIZE];
        int    size;
        time_t modtime;
    } in;
};

typedef struct uartStruct*         UART;
typedef struct uartTerminalStruct* UART_TERMINAL;
typedef struct uartFileStruct*     UART_FILE;

void UART_redirect(UART* from, UART to) {
    to->initialised = (*from)->initialised;
    to->enabled = (*from)->enabled;
    *from = to;
}

static void UART_initialise(UART uart, int type) {
    uart->type = type;
    uart->initialised = FALSE;
    uart->enabled = FALSE;
}

static void UART_init(UART uart) {
    uart->initialised = TRUE;
}

static void UART_enable(UART uart) {
    if (!uart->initialised) {
        printf("Alert: UART_enable called when UART is uninitialized.\n");
    }
    uart->enabled = TRUE;
}

static void UART_disable(UART uart) {
    if (!uart->initialised) {
        printf("Alert: UART_disable called when UART is uninitialized.\n");
    }
    uart->enabled = FALSE;
}

/* Re-open the input file if it has changed. */
static void UART_sync(UART uart) {
    if (uart->type == UART_File) {
        UART_FILE f = (UART_FILE)uart;
        struct stat sbuf;
    
        if (stat(f->in.name,&sbuf) != 0) {
            printf("Unable to stat UART input file %s: %s\n.",
                    f->in.name,strerror(errno));
            exit(1);
        }

        if (f->in.modtime < sbuf.st_mtime) {
            f->in.fp = fopen(f->in.name,"r");
            if (f->in.fp == NULL) {
                printf("Unable to refresh UART input: %s\n.",strerror(errno));
                exit(1);
            }
            f->in.modtime = sbuf.st_mtime;
            f->in.size = sbuf.st_size;
            printf("Refreshed UART input file %s\n",f->in.name);
        }
    }
}

static void UART_putc(UART uart, char ch) {
    if (!uart->enabled) {
        printf("Alert: UART_putc called while UART is disabled.\n");
    }
    if (uart->type == UART_Terminal) {
        UART_TERMINAL t = (UART_TERMINAL)uart;
#ifdef SIMULATION_LOG_FILE
        fputc(ch,SimulatorLogFile);
#endif /* SIMULATION_LOG_FILE */
        if (ch != '\r') {
            set_term(t->screen);
            waddch(t->window,ch);
        }
    }
    else {
        UART_FILE f = (UART_FILE)uart;
        if (f->out != NULL)
            fputc(ch,f->out);
    }
}

static int UART_putc_rdy(UART uart) {
    if (!uart->enabled)
        return FALSE;
    return TRUE;
}

static void UART_puts(UART uart, char* str) {
    if (!uart->enabled) {
        printf("Alert: UART_puts called while UART is disabled.\n");
    }
    if (uart->type == UART_Terminal) {
        UART_TERMINAL t = (UART_TERMINAL)uart;
        char* ch = str;
        set_term(t->screen);
        while(*ch != 0) {
#ifdef SIMULATION_LOG_FILE
            fputc(*ch,SimulatorLogFile);
#endif /* SIMULATION_LOG_FILE */
            waddch(t->window,*ch);
            ch++;
        }
    }
    else {
        UART_FILE f = (UART_FILE)uart;
        if (f->out != NULL)
            fputs(str,f->out);
    }
}

static int UART_getc(UART uart) {
    UART_sync(uart);
    if (!uart->enabled) {
        printf("Alert: UART_getc called while UART is disabled.\n");
    }
    if (uart->type == UART_Terminal) {
        UART_TERMINAL t = (UART_TERMINAL)uart;
        set_term(t->screen);
        return wgetch(t->window);
    }
    else {
        UART_FILE f = (UART_FILE)uart;
        if (f->in.fp != NULL && (ftell(f->in.fp) < f->in.size)) {
            int ch = fgetc(f->in.fp);
            return(ch);
        }
        else
            return 0;
    }
}

int    UART_getc_rdy(UART uart) {
    UART_sync(uart);
    if (!uart->enabled) {
        printf("Alert: UART_getc called while UART is disabled.\n");
    }
    if (uart->type == UART_Terminal) {
        UART_TERMINAL t = (UART_TERMINAL)uart;
        int ch;
        set_term(t->screen);
        /*
         * Put terminal into non-blocking mode temporarily.
         */
        nodelay(t->window,TRUE);
        ch = wgetch(t->window);
        if (ch != ERR)
          ungetch(ch);
        nodelay(t->window,FALSE);
        return (ch != ERR);
    }
    else {
        UART_FILE f = (UART_FILE)uart;
        return (f->in.fp != NULL &&  (ftell(f->in.fp) < f->in.size));
    }
}

/*
 * These are the emulated UART ports.
 */
struct uartTerminalStruct    UART3Struct;
struct uartTerminalStruct    UART5Struct;
static struct uartFileStruct UART1Struct;
UART UART3 = (UART)&UART3Struct;
UART UART5 = (UART)&UART5Struct;
UART UART1 = (UART)&UART1Struct;

/*=========================================================================
 * FUNCTION:      InitializeUARTFile
 * TYPE:          initialization
 * OVERVIEW:      Open the input and output files that emulate a UART port.
 * INTERFACE:
 *   parameters:  uart -
 *                baseName - base file name. Actual files opened will be 
 *                baseName+".in" and baseName+".out" respectively.
 *   returns:     <nothing>
 *=======================================================================*/

static void InitializeUARTFile(UART_FILE uart, char* inputName,
                               char* outputName)
{
    UART_initialise((UART)uart,UART_File);
    if ((uart->out = fopen(outputName,"w")) == NULL) {
        printf("Unable to open %s for output: %s\n",outputName,
               strerror(errno));
        exit(1);
    }

    strcpy(uart->in.name,inputName);
    uart->in.modtime = 0;
    uart->in.size = 0;
    uart->in.fp = NULL;
    UART_sync((UART)uart);
}

/*=========================================================================
 * FUNCTION:      FinalizeUARTFile
 * TYPE:          finalization
 * OVERVIEW:      Close the input and output files that emulate a UART port.
 * INTERFACE:
 *   parameters:  uart -
 *   returns:     <nothing>
 *=======================================================================*/

static void FinalizeUARTFile(UART_FILE uart) {
    if (uart->out != NULL)
        fclose(uart->out);
    if (uart->in.fp != NULL)
        fclose(uart->in.fp);
    uart->in.modtime = 0;
    uart->in.size = 0;
}
 
/*=========================================================================
 * FUNCTION:      InitializeUARTTerminal
 * TYPE:          initialization
 * OVERVIEW:      Initialise an ncurses terminal to emulate a UART port.
 * INTERFACE:
 *   parameters:  uart -
 *                device - e.g "/dev/pts/4"
 *   returns:     <nothing>
 *=======================================================================*/
static void InitializeUARTTerminal(UART_TERMINAL uart, const char* device)
{
    int   outId, inId;
    FILE  *outFp, *inFp;
    char* TERM = getenv("TERM");
    
    UART_initialise((UART)uart,UART_Terminal);
    if (device == NULL) {
        outId = STDOUT_FILENO;
        inId  = STDIN_FILENO;
    }
    else {
        outId = open(device,O_WRONLY);
        inId  = open(device,O_RDONLY);
    }
    inFp = fdopen(inId,"r");
    if (inFp == NULL) {
        printf("Unable to open %s for input: %s\n",
            (device == NULL ? "<stdin>" : device),strerror(errno));
        exit(1);
    }
    outFp = fdopen(outId,"w");
    if (outFp == NULL) {
        printf("Unable to open %s for output: %s\n",
            (device == NULL ? "<stdout>" : device),strerror(errno));
        exit(1);
    }
    uart->screen = newterm(TERM,outFp,inFp);
    if (uart->screen == NULL) {
        printf("Unable to create terminal.\n");
        exit(1);
    }
    set_term(uart->screen);
    uart->window = stdscr;
    def_prog_mode();
    /*
     * Make input unbuffered.
     */
    cbreak();
    /*
     * We don't want local character echoing.
     */
    noecho();
    /*
     * Screen can scroll if end of line reached.
     */
    scrollok(uart->window,TRUE);
    /*
     * Terminal output is not buffered.
     */
    immedok(uart->window,TRUE);
}

/*=========================================================================
 * FUNCTION:      FinalizeUARTTerminal
 * TYPE:          finalization
 * OVERVIEW:      Close the terminal that emulates a UART port.
 * INTERFACE:
 *   parameters:  uart -
 *   returns:     <nothing>
 *=======================================================================*/

static void FinalizeUARTTerminal(UART_TERMINAL uart) {
    set_term(uart->screen);
    endwin();
}

/*=========================================================================
 * FUNCTION:      InitializeSimulator
 * TYPE:          initialization
 * OVERVIEW:      Initializes the simulator. This starts up the ncurses
 *                based console which simulates user interaction via a
 *                terminal emulator (e.g. minicom or hyperterminal). It
 *                also allocates the simulated Flash memory and opens the
 *                log file.
 * INTERFACE:
 *   parameters:  argc, argv
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeSimulator(int argc, char** argv) {
    int i;
    char* uart1InputName = NULL;
    char* uart1OutputName = NULL;
    char* uart3Device = NULL;
    char* uart5Device = NULL;
    
    for (i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg,"-uart1.in") == 0)
            uart1InputName = getParam(argv,++i,argc);
        else
        if (strcmp(arg,"-uart1.out") == 0)
            uart1OutputName = getParam(argv,++i,argc);
        else
        if (strcmp(arg,"-uart3") == 0)
            uart3Device = getParam(argv,++i,argc);
        else
        if (strcmp(arg,"-uart5") == 0)
            uart5Device = getParam(argv,++i,argc);
        else {
            printf("Unknown arg: %s\n",arg);
            exit(1);
        }
    }
    
    /*
     * Initialize the UART emulators.
     */
    InitializeUARTTerminal((UART_TERMINAL)UART3,uart3Device);
    InitializeUARTTerminal((UART_TERMINAL)UART5,uart5Device);
    if (uart1InputName == NULL)
        uart1InputName = "UART1.in";
    if (uart1OutputName == NULL)
        uart1OutputName = "UART1.out";
    InitializeUARTFile((UART_FILE)UART1,uart1InputName,uart1OutputName);
    /*
     * The debug port (i.e. UART3) is initialised and enabled by default.
     */
    UART_init(UART3);
    UART_enable(UART3);
    /*
     * Allocate the simulated flash.
     */
    RealFlash = malloc(TOTAL_FLASH_SIZE + PAGESIZE - 1);
    if (!RealFlash) {
        perror("Couldn't malloc flash");
        exit(errno);
    }
    /*
     * Align to multiple of PAGESIZE for the purpose of using mprotect.
     * Note that PAGESIZE is assumed to be a power of two.
     */
    Flash = (unsigned char*)(((int)RealFlash + PAGESIZE - 1) & ~(PAGESIZE-1));
    JarFlashBlock = Flash + (JAR_FLASH_BLOCK * FLASH_BLOCK_SIZE);
        
    /*
     * Make the simulated flash read-only.
     */
    if (mprotect(Flash,TOTAL_FLASH_SIZE,PROT_READ) != 0)
        printf("Couldn't mprotect flash: %s\n",
                         sys_errlist[errno]);
    /*
     * Open the log file.
     */
#ifdef SIMULATION_LOG_FILE
    if ((SimulatorLogFile = fopen(SIMULATION_LOG_FILE,"w")) == NULL) {
        perror("Couldn't open simulation.log");
        exit(errno);
    }
#endif /* SIMULATION_LOG_FILE */

    /*
     * Capture the startup time.
     */
    gettimeofday(&ProcessStart,NULL);

    /*
     * Initialise all slots in TimerArray to 0.
     */
    memset(&(TimerArray[0]),0,sizeof(ulong64)*NUM_TIMER_IDS);
}

/*=========================================================================
 * FUNCTION:      FinalizeSimulator
 * TYPE:          finalization
 * OVERVIEW:      Finalizes the simulator. This closes the ncurses
 *                based console, deallocate the simulated Flash and closes
 *                the log file.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void FinalizeSimulator() {
    /*
     * Finalize UART emulators.
     */
    FinalizeUARTFile((UART_FILE)UART1);
    FinalizeUARTTerminal((UART_TERMINAL)UART3);
    FinalizeUARTTerminal((UART_TERMINAL)UART5);
    /*
     * Free emaulated Flash.
     */
    if (RealFlash != 0)
        free(RealFlash);
#ifdef SIMULATION_LOG_FILE
    if (SimulatorLogFile != NULL)
        fclose(SimulatorLogFile);
#endif /* SIMULATION_LOG_FILE */
}

/*=========================================================================
 * FUNCTION:      flash_read
 *                flash_write      - not yet implemented
 *                flash_unlock     - not yet implemented
 *                flash_erase      - not yet implemented
 *                flash_config     - not yet implemented
 *                flash_status     - not yet implemented
 *                flash_clear      - not yet implemented
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the flash.o module.
 *=======================================================================*/

int flash_read(int addr, unsigned char* buf) {
    memcpy(buf,Flash + (addr - FLASH_BASE),FLASH_READ_SIZE);
    return 0;
}

/*=========================================================================
 * FUNCTION:      fx_load
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the fx.o module.
 *=======================================================================*/

void fx_load(int blockId) {
    unsigned char* block = Flash + (blockId * FLASH_BLOCK_SIZE);
    int size, bytesRead;
    int tmpFlash[(FLASH_BLOCK_SIZE * FLASH_BLOCK_COUNT) / 4];
    FILE* fp = promptForFile("rb",NULL,&size);
    
    if (fp == NULL)
        return;
    /*
     * This implementation does not actually restart the process as that
     * effectively undoes the effect of loading to flash. It simply succeeds or
     * fails, reports this to the user and returns.
     */
    if (size > MAX_JAR_SIZE) {
        printf("File too big - load cancelled.\n");
        return;
    }
    bytesRead = fread(tmpFlash,1,size,fp);
    if (bytesRead != size) {
        printf("Read failed - load cancelled.\n");
        return;
    }
    /*
     * Make flash write-only temporarily.
     */
    if (mprotect(Flash,TOTAL_FLASH_SIZE,PROT_WRITE) != 0)
        printf("Couldn't mprotect flash: %s\n",
                         sys_errlist[errno]);
    memcpy(block,tmpFlash,bytesRead);
    /*
     * Now make flash read-only again.
     */
    if (mprotect(Flash,TOTAL_FLASH_SIZE,PROT_READ) != 0)
        printf("Couldn't mprotect flash: %s\n",
                         sys_errlist[errno]);
    printf("Load successful.\n");
    fclose(fp);
}

/*=========================================================================
 * FUNCTION:      uart_putc
 *                uart_putv    - not yet implemented
 *                uart_puts
 *                uart_getc
 *                uart_getv    - not yet implemented
 *                uart_stat
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the uart.o module.
 *=======================================================================*/

void	uart_putc(char ch) {
    UART_putc(UART3,ch);
}
void	uart_puts(char* str) {
    UART_puts(UART3,str);
}

char		uart_getc(void) {
    return UART_getc(UART3);
}

int     uart_stat(void) {
    return UART_getc_rdy(UART3);
}

/*=========================================================================
 * FUNCTION:      rtc_init
 *                rtc_get_time
 *                rtc_set_time     - not yet implemented
 *                rtc_start_oscr
 *                rtc_stop_oscr
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the rtc.o module.
 *=======================================================================*/

void rtc_init() {
}
void rtc_start_oscr() {
}
void rtc_stop_oscr() {
}

void	rtc_get_time(void) {
    time_t clk;
    struct tm *tmP = NULL;

    /* returns the time */ 
    time(&clk);  

    /* returns pointer to tm struct */
    tmP = localtime(&clk); 
    
    /* initialize the calendar fields */
    rtc_time_shared.yr = tmP->tm_year;
    rtc_time_shared.mth = tmP->tm_mon;
    rtc_time_shared.date = tmP->tm_mday;
    rtc_time_shared.hrs = tmP->tm_hour;
    rtc_time_shared.mins = tmP->tm_min;
    rtc_time_shared.secs = tmP->tm_sec;
    rtc_time_shared.mode_12hr = 0;
}

/*=========================================================================
 * FUNCTION:      timer_start     - not yet implemented
 *                timer_end       - not yet implemented
 *                timer_init
 *                timer_timeout   - not yet implemented
 *                timer_delay_us  - not yet implemented
 *                timer_delay_ms
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the timer.o module.
 *=======================================================================*/

RESULT timer_init (TIMER_ID id) {
    if (id < TIMEOUT_ID_1 || id >= NUM_TIMER_IDS)
        return ERROR;
    TimerArray[id] = getTimeSinceStartup();
    return OK;
}

/*
 * <ncurses.h> redefines OK to be 0!
 */
#define DCRYPT_ERROR  0
#define DCRYPT_OK     1
RESULT timer_timeout(TIMER_ID id, uint timeout) {
    if (id < TIMEOUT_ID_1 || id >= NUM_TIMER_IDS || TimerArray[id] == 0 ||
        ((getTimeSinceStartup() - TimerArray[id]) < timeout))
        return DCRYPT_ERROR;
    else
        return DCRYPT_OK;
}

void timer_delay_ms(uint duration) {
    struct timeval tv;
    tv.tv_sec = duration /1000;
    tv.tv_usec = (duration % 1000) * 1000;
    select(0,NULL,NULL,NULL,&tv);
}

/*
 * This function is provided to simulate reading directly from a variable
 * that is mapped to a hardware register - ostimer.
 */
ulong64 getTimeSinceStartup() {
    struct timeval now;
    ulong64 result;
    gettimeofday(&now,NULL);
    /*
     * Do carry from seconds to microseconds manually so we don't have to
     * rely on the compiler generated arithmetic to 'do the right thing'.
     */
    if (now.tv_usec < ProcessStart.tv_usec) {
        now.tv_usec += 1000000;
        now.tv_sec -= 1;
    }
    result = (ulong64)
            (((now.tv_sec  - ProcessStart.tv_sec)*1000) + 
             ((now.tv_usec - ProcessStart.tv_usec)/1000));
    return result;
}

/*=========================================================================
 * FUNCTION:      ns_init
 *                ns_get_bit       - not yet implemented
 *                ns_get_byte
 *                ns_fips_tests    - not yet implemented
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the noisesrc.o module.
 *=======================================================================*/

void ns_init() {
    srand(time(0));
}

int  ns_get_byte() {
    return rand();
}

/*=========================================================================
 * FUNCTION:      uart1_put_dtr       - not yet implemented
 *                uart1_put_rts       - not yet implemented
 *                uart1_get_cts       - not yet implemented
 *                uart1_get_dcd       - not yet implemented
 *                uart1_get_dsr       - not yet implemented
 *                uart1_get_ri        - not yet implemented
 *                uart1_init          - not yet implemented
 *                uart1_enable        - not yet implemented
 *                uart1_disable       - not yet implemented
 *                uart1_putc          - not yet implemented
 *                uart1_getc          - not yet implemented
 *                uart1_putc_rdy      - not yet implemented
 *                uart1_getc_rdy      - not yet implemented
 *                uart1_putc_avail    - not yet implemented
 *                uart1_getc_avail    - not yet implemented
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the uart.o module.
 *=======================================================================*/

void uart1_init(int flow, uart_t baud) {
    UART_init(UART1);
}

void uart1_enable(void) {
    UART_enable(UART1);
}

void uart1_disable(void) {
    UART_disable(UART1);
}

char uart1_getc(void) {
    return UART_getc(UART1);
}
int uart1_getc_rdy(void) {
    return UART_getc_rdy(UART1);
}
int uart1_getc_avail(void) {
    return UART_getc_rdy(UART1);
}

void uart1_put_dtr(int onOff) {
    printf("stub: uart1_put_dtr(onOff=%d)\n",onOff);
}

int uart1_get_cts(void) {
    printf("stub: uart1_get_cts()\n");
    return 0;
}
int uart1_get_dcd(void) {
    printf("stub: uart1_get_dcd()\n");
    return 0;
}
int uart1_get_dsr(void) {
    printf("stub: uart1_get_dsr()\n");
    return 0;
}
int uart1_get_ri(void) {
    printf("stub: uart1_get_ri()\n");
    return 0;
}
void uart1_putc(char c) {
    UART_putc(UART1,c);
}
int uart1_putc_rdy(void) {
    return UART_putc_rdy(UART1);
}
int uart1_putc_avail(void) {
    return UART_putc_rdy(UART1);
}
void uart1_put_rts(int onOff) {
    printf("stub: uart1_put_rts(onOff=%d)\n",onOff);
}

/*=========================================================================
 * FUNCTION:      uart5_get_dtr       - stub
 *                uart5_get_rts       - stub
 *                uart5_put_cts       - stub
 *                uart5_put_dcd       - stub
 *                uart5_put_dsr       - stub
 *                uart5_put_ri        - stub
 *                uart5_init
 *                uart5_enable
 *                uart5_disable
 *                uart5_putc
 *                uart5_getc
 *                uart5_putc_rdy
 *                uart5_getc_rdy
 *                pcmcia_online
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the uart5.o module.
 *=======================================================================*/

void uart5_init(void) {
    UART_init(UART5);
}

void uart5_enable(void) {
    UART_enable(UART5);
}

void uart5_disable(void) {
    UART_disable(UART5);
}

unsigned uart5_getc(void) {
    return UART_getc(UART5);
}

int uart5_getc_rdy(void) {
    return UART_getc_rdy(UART5);
}

int uart5_get_dtr(void) {
    printf("stub: uart5_get_dtr()\n");
    return 0;
}
int uart5_get_rts(void) {
    printf("stub: uart5_get_rts()\n");
    return 0;
}

void uart5_putc(unsigned char ch) {
    UART_putc(UART5,ch);
}
int uart5_putc_rdy(void) {
    return TRUE;
}
void uart5_put_cts(int onOff) {
    printf("stub: uart5_put_cts(onOff=%d)\n",onOff);
}
void uart5_put_dcd(int onOff) {
    printf("stub: uart5_put_dcd(onOff=%d)\n",onOff);
}
void uart5_put_dsr(int onOff) {
    printf("stub: uart5_put_dsr(onOff=%d)\n",onOff);
}
void uart5_put_ri(int onOff) {
    printf("stub: uart5_put_ri(onOff=%d)\n",onOff);
}

int	pcmcia_online(void)  {
    return TRUE;
}

/*=========================================================================
 * FUNCTION:      audio_recv         - stub
 *                audio_xmit         - stub
 *                audio_init         - stub
 *                audio_enable       - stub
 *                audio_diable       - stub
 *                audio_set_spk_gain - stub
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the audio.o module.
 *=======================================================================*/

/*
 * Model the shared memory regions.
 */
AUDIO_FRAME	audio_shared;
AUDIO_SAMPLES	audio_shared_samples;

void	audio_disable(void)  {
    printf("stub: audio_disable()\n");
}
void	audio_enable(void)  {
    printf("stub: audio_enable()\n");
}
void	audio_init (uint sample_rate)  {
    printf("stub: audio_init(sample_rate=%u)\n",sample_rate);
}

RESULT	audio_recv(void)  {
    printf("stub: audio_recv()\n");
    return ERROR;
}

void	audio_set_spk_gain (uint spk_gain)  {
    printf("stub: audio_set_spk_gain(spk_gain=%u)\n",spk_gain);
}

RESULT	audio_xmit(void)  {
    printf("stub: audio_xmit()\n");
    return ERROR;
}

/*=========================================================================
 * FUNCTION:      melp_ana_init       - stub
 *                melp_ana_mod        - stub
 *                melp_syn_init       - stub
 *                melp_syn_mod        - stub
 * TYPE:          DCryptor stubs
 * OVERVIEW:      Stubs for the functions in the melp.o module.
 *=======================================================================*/

void melp_ana_init(uint rate) {
    printf("stub: melp_ana_init(rate=%u)\n",rate);
}
void melp_ana_mod(short sp_in[], unsigned char buf[])  {
    printf("stub: melp_ana_mod(sp_in=0x%x,buf=0x%x)\n",(unsigned)sp_in,(unsigned)buf);
}
void melp_syn_init(uint rate) {
    printf("stub: melp_syn_init(rate=%u)\n",rate);
}
void melp_syn_mod(unsigned char buf[],  short sp_out[])  {
    printf("stub: melp_syn_mod(buf=0x%x,sp_out=0x%x)\n",(unsigned)buf,(unsigned)sp_out);
}


#endif /* SIMULATOR */
