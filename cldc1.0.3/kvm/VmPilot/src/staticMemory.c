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
 * SUBSYSTEM: Memory management (for the Palm)
 * FILE:      staticMemory.c
 * OVERVIEW:  Allocate "storage/static" memory on the Palm.
 *            There is plenty of such memory available.
 *
 *            Unfortunately, it is very slow to write anything
 *            to static memory, so the areas allocated from
 *            static memory should only be used for those 
 *            data structures that don't need to be changed 
 *            often.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            (based on code written by Doug Simon)
 *=======================================================================*/

/*=========================================================================
 * For detailed explanation of the memory system, see Garbage.h
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <KVM.h>

#if USESTATIC

/*=========================================================================
 * Variables
 *=======================================================================*/

static CharPtr LastStaticChunk;

/*=========================================================================
 * FUNCTION:      mallocStaticBytes()
 * TYPE:          public global operation
 * OVERVIEW:      Allocate a chunk in static memory of the given size 
 *                (in bytes). This chunk is locked and will be automatically
 *                unlocked and deallocated in FinalizeStaticMemory. Note
 *                that PalmOS may return two-byte aligned addresses,
 *                and hence we manually bump the address upwards by
 *                two bytes if necessary. This is quite tricky since
 *                we also need the original PalmOS addresses in order
 *                to get rid of the chunks when the VM shuts down.
 *                Writing to static objects has to be done using
 *                the PalmOS DmWrite function, with the proper
 *                adjustment for alignment.
 * INTERFACE:
 *   parameters:  size - the size in bytes of the chunk requested
 *   returns:     pointer to newly allocated area, or
 *                NIL if allocation fails.
 *=======================================================================*/

void* 
mallocStaticBytes(int size)
{
    /* We allocate two extra words.  In the first word, we store the location
     * of the previous "static memory."  The pieces of static memory form
     * a linked list.
     * 
     * The Pilot guarantees that allocations are two-byte aligned, but we
     * need the allocations to be 4-byte aligned.  So we allocate four extra
     * bytes (well, only two are needed) to make sure we really will get the
     * space we need.  
     *
     * In either case, we write teh address of the previous value of
     * LastStaticChunk into the first four bytes of the newly allocated
     * memory.  If we need to perform alignment, we write the value "-1" into
     * the last byte just before we are about to write the data.  
     */
    VoidHand newHandle = DmNewHandle(ClassfileDB, size + (2 * CELL));
    if (!newHandle) { 
        fatalError("Out of static memory");
        return NIL;
    } else { 
        char *newChunk      = MemHandleLock(newHandle);
        char *alignedChunk  = (char *)(((long)newChunk + (CELL - 1)) & ~(CELL - 1));
        int  alignmentOffset = alignedChunk - newChunk; /* wasted space */
        /* Save the previous value of LastStatic Chunk */
        DmWrite(newChunk, 0, &LastStaticChunk, CELL);
        if (alignmentOffset != 0) { 
            /* Write the -1 into the final unused byte */
            DmSet(newChunk, alignmentOffset + (CELL - 1), 1, -1);
        }
#if ENABLEPROFILING
        StaticObjectCounter++;
        StaticAllocationCounter += size;
#endif
        LastStaticChunk = (VoidPtr)newChunk;
        return (char *)(alignedChunk + CELL);
    }
}

/*=========================================================================
 * FUNCTION:      modifyStaticMemory()
 * TYPE:          public global operation
 * OVERVIEW:      Modify a chunk of static memory.
 * INTERFACE:
 *   parameters:  staticMemory - a pointer to some piece of memory returned
 *                     by mallocStaticBytes
 *                ptr - A pointer into the interior of the object pointed at
 *                     by staticMemory, indicating the bytes to change
 *                newVal - The new value to place into the memory
 *                size - The number of bytes to change.
 *=======================================================================*/

void
modifyStaticMemory(void *staticMemory, int offset, void *newVal, int size)
{
    int err;     

    /* If the byte immediately before staticMemory is -1, then we have two
     * wasted bytes.  Otherwise, we have no wasted bytes */
    signed char flag =  *((signed char *)staticMemory - 1);
    int alignOffset = (flag & 0x1) << 1; /* 0 or 2 */

    /* Chunk is the actual value returned by DmNewHandle */
    void *chunk = (void *) ((char *)staticMemory - (alignOffset + CELL));
    
    /* Perform the modification */
    err = DmWrite(chunk, offset + alignOffset + CELL, newVal, size);
    ErrFatalDisplayIf(err, "DmWriteCheck failed (make static)");
}

/*=========================================================================
 * FUNCTION:      FinalizeStaticMemory()
 * TYPE:          public global operation
 * OVERVIEW:      Deallocate all static memory chunks allocated by
 *                mallocStaticBytes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void FinalizeStaticMemory()
{
    VoidPtr thisChunk, nextChunk;
    /* We have a linked list of static chunks.  We just run down the chain
     * and deallocate them. */
    for (thisChunk = LastStaticChunk; thisChunk != NIL; thisChunk = nextChunk) {
        VoidHand chunkHandle = MemPtrRecoverHandle(thisChunk);
        nextChunk  = *(VoidPtr**)thisChunk;
        MemHandleUnlock(chunkHandle);
        MemHandleFree(chunkHandle);
    }
    LastStaticChunk = NIL;
}

#endif /* USESTATIC */


