/*
 * Copyright (c) 2000 Sun Microsystems, Inc. All Rights Reserved.
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

#include <CWCompatibility.h>
#include <inetmgr.h>
#include <ctp.h>
#include <Jam.h>

static Word    AppINetRefnum;
static Handle  AppINetHandle;

static Err INetStart(void);
static char *fetchURL(const char *host, int port, const char *path) ;
static void closeLib(void);
static bool_t smallEventLoop();
static Err inetlibReadBytes(void *sock, char *record, long offset, ULong *lengthP);

Boolean INetLibWiCmd2(Word refNum, Word cmd, Int arg1, Int arg2)
    SYS_TRAP(inetLibTrapWiCmd);

NetworkFetcherType netlibFetcher;
NetworkCloserType  netlibCloser;


#define MAXIMUM_DOWNLOAD_SIZE 65000

bool_t
INetlib_exists() 
{ 
    Err    error = 0;
    UInt32 value = 0; // Value of the feature requested.

    error = FtrGet(inetLibFtrCreator, inetFtrNumVersion, &value);
    return (error == 0 && value != 0);
}



bool_t 
INetlib_open(NetworkFetcherType* fetcher, NetworkCloserType* closer) { 

    if (INetStart() == 0) { 
        *fetcher = fetchURL;
        *closer = closeLib;
        return TRUE;
    } else { 
        return FALSE;
    }
}


static Err INetStart(void)
{
    Err    error = 0;
    UInt32 value = 0;      // Value of the feature requested.

    INetConfigNameType cfgName  = { inetCfgNameCTPDefault }; 
    UInt16             cfgIndex = 0;                          

    error = FtrGet(inetLibFtrCreator, inetFtrNumVersion, &value);
    if (error || value == 0) { 
        // No need for status.  It's just not there.
        return error;
    }

    
    error = SysLibFind("INet.lib", &AppINetRefnum);
    if (error) { 
        Alert("Could not find INet.lib");
        return error;
    }

    error = INetLibConfigIndexFromName(AppINetRefnum, &cfgName, &cfgIndex);
    if (error) { 
        Alert("Could not find configuration");
        return error;
    }

    error = INetLibOpen(AppINetRefnum, cfgIndex, 0, NULL, 0, &AppINetHandle);
    if (error) { 
        if (error == inetErrTooManyClients) { 
            Alert("Too many INetLib clients");
        } else { 
            Alert("INetLib open failed");
        }
        return error;
    }

    INetLibWiCmd2 (AppINetRefnum, 0, 140, 1);
    INetLibWiCmd2 (AppINetRefnum, 5, 76, 1);
    INetLibWiCmd2 (AppINetRefnum, 2, 1, 1);

    
    // Set the max download size
    value = MAXIMUM_DOWNLOAD_SIZE;
    INetLibSettingSet(AppINetRefnum, AppINetHandle, 
              inetSettingMaxRspSize, (BytePtr)&value, sizeof(value));

    // Set the default conversion algorithm
    value = ctpConvNone;
    INetLibSettingSet(AppINetRefnum, AppINetHandle,
              inetSettingConvAlgorithm,
              (BytePtr)&value, sizeof(value));

    return 0;
}

static char *
fetchURL(const char *host, int port, const char *path) 
{
    Handle sockHandle = NULL;
    char *result = NULL;
    int length = 0;
    Err errno;

    char url[MAX_URL + 1];
    StrPrintF(url, "http://%s:%ld%s", host, (long)port, path);

#if 1
    errno = INetLibURLOpen(AppINetRefnum, AppINetHandle, (unsigned char *)url, 
               NULL, &sockHandle, (SDWord)-1, 0);
    if (errno) {
        Alert("INetLibURLOpen returned %ld", (long)errno);
        goto done;
    }

#else

    errno = INetLibSockOpen(AppINetRefnum, AppINetHandle, 
                inetSchemeHTTP, &sockHandle);

    if (errno) { 
        Alert("INetLibSockOpen returned %ld", (long)errno);
        goto done;
    }

    errno = INetLibSockHTTPReqCreate(AppINetRefnum, sockHandle, 
                     (unsigned char *)"GET", 
                     (unsigned char *)url, NULL);
    if (errno) { 
        Alert("INetLibHTTPReqCreate returned %ld", (long)errno);
        goto done;
    } 

    errno = INetLibSockHTTPReqSend(AppINetRefnum, sockHandle, NULL, 0, -1);
    if (errno) { 
        Alert("INetLibSockHTTPReqSend returned 0x%x", errno);
        goto done;
    }
#endif

    if (!smallEventLoop()) { 
        goto done;
    }

    result = readCompletely(sockHandle, inetlibReadBytes);

done:
    if (sockHandle != NULL) { 
        INetLibSockClose(AppINetRefnum, sockHandle);
    }
    return result;
}

static void
closeLib(void) 
{
    INetLibClose(AppINetRefnum, AppINetHandle);
    //  netlibCloser();
} 

static Err 
inetlibReadBytes(void *sock, char *record, long offset, ULong *lengthP) {
    char buffer[256];
    ULong readLength = *lengthP;
    Err result;
    if (readLength > sizeof(buffer)) { 
        readLength = sizeof(buffer);
    }
    result = INetLibSockRead(AppINetRefnum, sock, 
                             buffer, readLength, lengthP, -1);
    if (result == 0 && *lengthP > 0) { 
        DmWrite(record, offset, buffer, *lengthP);
    }
    return result;
}





#define firstINetLibEvent 4096

static bool_t
smallEventLoop(void) 
{ 
    int i;
    INetEventType event;
    FormPtr form = FrmGetActiveForm();
    const char * oldTitle = FrmGetTitle(form);
    for (i = 0;  ; i++) { 
        INetLibGetEvent(AppINetRefnum, AppINetHandle, 
                (INetEventType*)&event, SysTicksPerSecond() * 2);

        if (! SysHandleEvent((EventType*)&event)) { 
            switch(event.eType) { 
            case inetSockStatusChangeEvent:
                switch (event.data.inetSockStatusChange.status) {

                case inetStatusNew:
                    break;
                case inetStatusResolvingName:
                    setTitle("Resolving name...");
                    break;
                case inetStatusNameResolved:
                    break;
                case inetStatusConnecting:
                    setTitle("Connecting...");
                    break;
                case inetStatusConnected:
                    break;
                case inetStatusSendingRequest:
                    setTitle("Sending...");
                    break;
                case inetStatusWaitingForResponse:
                    setTitle("Waiting...");
                    break;
                case inetStatusReceivingResponse:
                    setTitle("Receiving...");
                    break;
                case inetStatusResponseReceived:
                    break;
                case inetStatusClosingConnection:
                    setTitle("Closing...");
                    break;
                case inetStatusClosed:
                    break;
                case inetStatusAcquiringNetwork:
                    setTitle("Acquiring...");
                    break;

                default:
                    break;
                }
                break;

            case inetSockReadyEvent:
                setTitle(oldTitle);
                return TRUE;

            case appStopEvent:
                setTitle(oldTitle);
                // Someone else needs to do the right thing.
                EvtAddEventToQueue((EventType*)&event);
                return FALSE;

            default:
                break;
            }
        }
    }
 }
