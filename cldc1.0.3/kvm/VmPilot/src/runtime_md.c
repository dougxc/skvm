/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: Platform-specific runtime functions. 
 * FILE:      runtime_md.c
 * OVERVIEW:  Palm-specific runtime functions needed by Java Runtime.
 * AUTHOR:    Frank Yellin.
 *=====================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <global.h>
#include <KVM.h>

/*======================================================================
 * Helper operations and macros
 *====================================================================*/

#define MAXCALENDARFLDS 15

static ulong64 javaBaseMillis;   /* Java time base in millis, since 1/1/1970 */
static UInt32  palmBaseSecs;     /* Palm time base in secs,   since 1/1/1904 */
static UInt32  palmBaseTicks;    /* Palm time base in ticks, since reset */
static void initializeTime(void);
static unsigned long date[MAXCALENDARFLDS];

/* constant: number of seconds between Palm time base and Java time base */
#define JAVA_PALM_OFFSET ((unsigned long)(24107U * 24U * 60U * 60U))

#define YEAR 1
#define MONTH 2
#define DAY_OF_MONTH 5
#define HOUR 10
#define MINUTE 12
#define SECOND 13
#define MILLISECOND 14

/*=========================================================================
 * FUNCTION:      alertUser()
 * TYPE:          error handling operation
 * OVERVIEW:      Show an alert dialog to the user and wait for 
 *                confirmation before continuing execution.
 * INTERFACE:
 *   parameters:  message string
 *   returns:     <nothing>
 *=======================================================================*/

void AlertUser(const char* message)
{
    extern bool_t calledFromJam;
    if (calledFromJam) { 
        fprintf(stderr, "%s", message);
    } else { 
        FrmCustomAlert(CustomAlert, (char *)message, NULL, NULL);
    }
}

/*=========================================================================
 * FUNCTION:      allocateHeap()
 * TYPE:          allocates memory 
 * OVERVIEW:      Show an alert dialog to the user and wait for 
 *                confirmation before continuing execution.
 * INTERFACE:
 *   parameters:  *sizeptr:  INPUT:   Pointer to size of heap to allocate
 *                           OUTPUT:  Pointer to actually size of heap
 *                *realresultptr: Returns pointer to actual pointer than
 *                           was allocated, before any adjustments for
 *                           memory alignment.  This is the value that
 *                           must be passed to "MemPtrFree()" 
 *  returns:      pointer to aligned memory.
 *
 *  Note that "sizeptr" reflects the size of the "aligned" memory, and
 *  note the actual size of the memory that has been allocated.
 *=======================================================================*/

typedef struct MemoryChainStruct { 
    struct MemoryChainStruct *next;
} *MEMORY_CHUNK;

extern CHUNK FirstFreeChunk;    /* Not all GC algorithms use this. */

#define MINIMUM_CHUNK_SIZE 512
#define MAXIMUM_MEMPTRNEW_REQUEST 65500

cell *allocateHeap(long *sizeptr, void **realresultptr) { 
    MEMORY_CHUNK memoryChain, ptr;
    ULong spaceNeeded = KVMprefs.heapSize << 10;
    UInt chunkCount, i;
    CHUNK previousFreeChunk;
    cell *startOfHeap;
    ULong totalSize;

    /* Compact the dynamic C heap so that we can allocate as much as
     * possible */
    MemHeapCompact(0);

    chunkCount = 0;
    memoryChain = NULL;
    while (spaceNeeded >= MINIMUM_CHUNK_SIZE) {
        /* Allocate as large a chunk as possible, within the
         * constraints of the memory system, and the amount of space we
         * want */
        ULong thisRequest;
        MEMORY_CHUNK thisResult;
        ULong freeSpace, largestChunk;

        MemHeapFreeBytes(0, &freeSpace, &largestChunk);

        if (freeSpace < OSRESERVEDSIZE + MINIMUM_CHUNK_SIZE 
            || largestChunk < MINIMUM_CHUNK_SIZE) { 
            /* Don't bother */
            break;
        } 
        freeSpace -= OSRESERVEDSIZE; /* Subtract OS reserved space */

        thisRequest = spaceNeeded;
        if (thisRequest > largestChunk) { 
            thisRequest = largestChunk;
        }
        if (thisRequest > freeSpace) { 
            thisRequest = freeSpace;
        } 

        /* Try to get the memory.  Some platforms don't allow requests
         * larger than 64K, so if we fail and it was a large request, 
         * try again with a smaller one */
        thisResult = MemPtrNew(thisRequest);
        if (thisResult == NULL && thisRequest > MAXIMUM_MEMPTRNEW_REQUEST) { 
            thisRequest = MAXIMUM_MEMPTRNEW_REQUEST;
            thisResult = MemPtrNew(thisRequest); 
            if (thisResult == 0) { 
                /* We couldn't get any memory. */
                break;
            }
        }
        chunkCount++;

        /* Put this chunk of memory onto the linked list of memory we
         * have allocated.  This list is kept sorted. */
        if (memoryChain == NULL) { 
            memoryChain = thisResult;
            thisResult->next = NULL;
        } else if (memoryChain > thisResult) {
            thisResult->next = memoryChain;
            memoryChain = thisResult;
        } else { 
            for (ptr = memoryChain; ; ptr = ptr->next) { 
                if (ptr->next == NULL || ptr->next > thisResult) { 
                    /* We're at the last item in the chain, or this is the
                     * place to insert the new item */
                    thisResult->next = ptr->next;
                    ptr->next = thisResult;
                    break;
                }
            }
        }
        /* Go back and make more requests */
        spaceNeeded -= thisRequest;
    }

    /* Now the clunky part.  We have to go through this mess and make it
     * look like it's a contiguous chain of memory, in which the
     * usable pieces are marked as free space, and the unusable spaces
     * are marked as being pieces of a GCT_NOPOINTERS.
     *
     * In addition, we have to make sure that the pieces marked GCT_NOPOINTERS
     * are all pointed at by a global root, so that they do not become
     * garbage collected.
     */
    for (i = 0, ptr = memoryChain, previousFreeChunk = NULL, totalSize = 0;
        ; 
        i++, ptr = ptr->next) { 

        ULong size = MemPtrSize(ptr);
        MEMORY_CHUNK next = ptr->next;

        /* We get the first and last "usable" memory location in each chunk.
         * At the beginning, we have to round up to a multiple of 4, and skip
         * over the header at the beginning.
         * At the end, we simply have to round down to a multiple of 4. 
         */
        void **usableStart = 
            (void *)(((long)ptr + 3 + sizeof(*ptr)) & ~0x3);
        void **usableEnd = (void **)(((long)ptr + size) & ~0x3);

        /* For all except the last block, we need to reserve the last word
         * for creating a fake GC header.  So this is the beginning and
         * end of the free space chunk 
         */
        void **freeStart = usableStart;
        void **freeEnd = (next == NULL) ? usableEnd : usableEnd - 1;
        CHUNK freeChunk;

        totalSize += size;


        if (ptr == memoryChain) { 
            startOfHeap = (cell *)freeStart;
        }

        /* Create the free chunk out of the free space */
        freeChunk = (CHUNK)freeStart;
        freeChunk->size = (freeEnd - freeStart - 1) << TYPEBITS;
        freeChunk->next = NULL;
        if (previousFreeChunk != NULL) {
            previousFreeChunk->next = freeChunk;
        } else {
            FirstFreeChunk = freeChunk;
        }
        previousFreeChunk = freeChunk;

        if (next != NULL) { 
            /* If this isn't the last item, then create the fake header in
             * the last word, that causes all the empty space to be ignored. */
            void **nextUsableStart = (void **)
                (((long)next + 3 + sizeof(*ptr)) & ~0x3);
            ((long *)usableEnd)[-1] = (GCT_NOPOINTERS << 2) 
                + ((nextUsableStart - usableEnd) << TYPEBITS)
                + STATICBIT;
        } else { 
            /* We've reached the end.  Return info to the user:
             * The "size" is distance from the start of the heap to the
             * last usable word of this last chain */
            *sizeptr = (long)freeEnd - (long)startOfHeap;
            break;
        }
    }
    if (KVMprefs.showHeapStats) {
        StrPrintF(str_buffer, "KVM.allocateHeap()\n"
                  "Requested %ld bytes\n"
                  "Allocated %ld bytes\n"
                  "in %ld chunks\n",
                  KVMprefs.heapSize<<10, totalSize, chunkCount);
        AlertUser(str_buffer);
    }

    *realresultptr = memoryChain;
    return startOfHeap;
}


void freeHeap(void *theHeap) { 
    MEMORY_CHUNK memoryChain = (MEMORY_CHUNK)theHeap;

    while (memoryChain != NULL) { 
        MEMORY_CHUNK next = memoryChain->next;
        MemPtrFree(memoryChain);
        memoryChain = next;
    }
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
    extern bool_t PendingBeamInitialized;
    
    PendingBeamInitialized = FALSE;
    initializeTime();
}

/*=========================================================================
 * FUNCTION:      FinalizeNativeCode
 * TYPE:          finalization
 * OVERVIEW:      called at shut down to perform machine-specific initialization. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void FinalizeNativeCode() {
}

/*=========================================================================
 * FUNCTION:      CurrentTime_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Returns the current time. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     current time, in centiseconds since startup
 *=======================================================================*/

static void
initializeTime() { 
    UInt32 timeBaseSecs;  /* time base in seconds */
    
    palmBaseSecs  = TimGetSeconds();
    palmBaseTicks = TimGetTicks();
    
    /* subtract tick-seconds from current time to get Palm timeBaseSecs */
    timeBaseSecs = palmBaseSecs - (palmBaseTicks / SysTicksPerSecond());
    
    /* subtract constant to get Java timeBaseSecs */
    timeBaseSecs -= JAVA_PALM_OFFSET;
    
    /* store Java timeBase in millis */
    javaBaseMillis = (ulong64)timeBaseSecs * 1000;
}

ulong64
CurrentTime_md(void)
{
    ulong64 elapsedMillis;   /* millis since timeBaseMillis */
    
    /* get current Palm time and tick values */
    UInt32 time  = TimGetSeconds();
    UInt32 ticks = TimGetTicks();
    
    /* compute elapsed seconds for both time and ticks */
    UInt32 elapsedTime  = time  - palmBaseSecs;
    UInt32 elapsedTicks = ticks - palmBaseTicks;
    UInt32 elapsedSecs  = elapsedTicks / SysTicksPerSecond();

    /* This is the tricky part: If Palm's time is more than 2 seconds ahead
     * of its ticker, we assume the Palm was powered off for a while, and
     * we re-initialize the time bases.
     */
    if (elapsedTime > (elapsedSecs+2)) {
       initializeTime();
       ticks = palmBaseTicks;  /* this is current Palm tick value */
    }
    
    /* compute elapsed millis from ticks, and return current time in millis */
    elapsedMillis = (ulong64)ticks * (1000 / SysTicksPerSecond());
    return elapsedMillis + javaBaseMillis;
}

/*=========================================================================
 * FUNCTION:      Calendar_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Initializes the calendar fields, which represent the 
 *                Calendar related attributes of a date. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     none 
 * AUTHOR:    Tasneem Sayeed
 *=======================================================================*/

unsigned long *
Calendar_md(void)
{
    unsigned long time;

    DateTimePtr dateTimeP = NULL;
    
    /* return seconds since Jan 1, 1904 */
    time = TimGetSeconds();
    
    /* initialize */
    memset(&date, 0, MAXCALENDARFLDS);

    /* get the DateTimeType Time Manager structure */
    TimSecondsToDateTime (time, dateTimeP);
    
    /* initialize Calendar fields */
    date[YEAR] = dateTimeP->year;
    date[MONTH] = dateTimeP->month; 
    date[DAY_OF_MONTH] = dateTimeP->day;
    date[HOUR] = dateTimeP->hour;
    date[MINUTE] = dateTimeP->minute;
    date[SECOND] = dateTimeP->second;
    /* On the Palm, we cannot accurately determine this value
     * Set it to zero for now
         */
    date[MILLISECOND] = 0;  
    
    return date;
}
