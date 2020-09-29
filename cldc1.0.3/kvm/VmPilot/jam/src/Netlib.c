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
#include <netmgr.h>
#include <Jam.h>

static Word    AppNetRefnum;
static Long    AppNetTimeout;
static Err     errno;

static char *fetchURL(const char *host, int port, const char *path);
static char *fetchURL_internal(const char *host, int port, 
                               const char *path, int *httpCode);
static NetSocketRef openHTTP(const char *host, int port);
static bool_t CreateSocketAddress(NetSocketAddrINType *, 
                                  const char *host, int port);
static char* getLine(NetSocketRef sock);
static void closeNetlib(void);
static Err netlibReadBytes(void *sock, char *record, long offset, ULong *lengthP);


bool_t 
Netlib_open(NetworkFetcherType* fetcher, NetworkCloserType* closer) { 
    Err err;
    Word ifErr;
    AppNetTimeout = 2 * SysTicksPerSecond();
    err = SysLibFind("Net.lib", &AppNetRefnum);
    if (err) {
        Alert("Cannot find Net.lib: Error %ld", (long)err);
        return FALSE;
    }
    NetLibOpen(AppNetRefnum, &ifErr);
    if (ifErr) {
        Alert("Cannot open Net.lib: Error %ld", (long)ifErr);
        return FALSE;
    }
    *fetcher = fetchURL;
    *closer =  closeNetlib;
    return TRUE;
}


static void
closeNetlib(void) 
{
    NetLibClose(AppNetRefnum, false);
    AppNetRefnum = 0;
}


/* Implement HTTP policy for retransmissions, etc.. */


static char *
fetchURL(const char *host, int port, const char *path) {
    for(;;) {
        int httpCode;
        char *content = fetchURL_internal(host, port, path, &httpCode);
        if (httpCode != 503) {
            return content;
        }
        if (content != NULL) {
            MemPtrFree(content);
        }
        SysTaskDelay(2 * SysTicksPerSecond());
    }
}


/*
 * This function is a real implementation of HTTP GET for downloading
 * Web pages and JAR files into the phone emulator.
 *
 * The current implementation maintains an malloc'ed auto-growing buffer
 * to download all the data that comes from the HTTP server.
 */
static char *
fetchURL_internal(const char *host, int port, const char *path, int *httpCode)
{
    enum { CODE, HEAD, BODY } state;
    NetSocketRef sock = -1;
    char buffer[MAX_URL + 30]; /* we reuse this for sending the data */
    int length;
    char *bufp;
    bool_t readBody = TRUE;
    Err errno;

    sock = openHTTP(host, port);
    if (sock < 0) { 
        return NULL;
    }

    // Send the data
    StrPrintF(buffer, "GET %s HTTP/1.0\n\n", path);
        for (bufp = buffer, length = StrLen(bufp); length > 0; ) { 
        int sent  = NetLibSend(AppNetRefnum, sock, bufp, length, 0, 0, 0, 
                               AppNetTimeout,&errno);
        if (sent < 0) { 
            break;
        }
        length -= sent;
        bufp += sent;
    }
    
    // Receive the data
    for (state = CODE;; ) { 
        switch (state) {
            case CODE:
                bufp = getLine(sock);
                if (bufp == NULL) { 
                    *httpCode = 400;
                    goto error;
                } else {
                    char *p = StrChr(bufp, ' '); /* the first space */
                    do { p++; } while (*p == ' '); /* skip multiple spaces */
                    *httpCode = StrAToI(p);
                    switch(*httpCode) {
                        case 200: /* OK */
                            break;
                        default:
                        case 404: /* Resource not available - permanent */
                            Alert("URL not found");
                        /* Fall through */
                        case 503:/* Resource not available - temporary */
                            readBody = FALSE;
                            break;
                     }
                }
                state = HEAD;
                MemPtrFree(bufp);
                break;

            case HEAD:
                if ((bufp = getLine(sock)) == NULL) { 
                    goto error;
                } else { 
                    state = (bufp[0] == '\n') ? BODY : HEAD;
                    MemPtrFree(bufp);
                } 
                break;

            case BODY: { 
                char *content = readBody ? readCompletely((void *)sock, netlibReadBytes)
                                 : NULL;
                NetLibSocketClose(AppNetRefnum, sock, AppNetTimeout, &errno);
                return content;
            }
        }
    }

error:
    if (sock >= 0) {
        NetLibSocketClose(AppNetRefnum, sock, AppNetTimeout, &errno);
    }
    return NULL;
}

static NetSocketRef 
openHTTP(const char *host, int port) {
    NetSocketAddrINType sockaddr;
    NetSocketRef sock = -1;
    Err errno;

    if (!CreateSocketAddress(&sockaddr, host, port)) {
        goto error;
    }
    sock = NetLibSocketOpen(AppNetRefnum, netSocketAddrINET, 
                netSocketTypeStream, 0,AppNetTimeout,&errno);
    if (sock < 0) { 
        Alert("Cannot open socket");
        goto error;
    }

    if (NetLibSocketConnect(AppNetRefnum, sock, 
            (NetSocketAddrType*)&sockaddr, sizeof(sockaddr),
            AppNetTimeout,&errno) < 0) {
        Alert("Cannot connect to host");
        goto error;
    }
    return sock;

error:
    if (sock >= 0) { 
        NetLibSocketClose(AppNetRefnum, sock, AppNetTimeout, &errno);
    }
    return -1;
}

static bool_t
CreateSocketAddress(NetSocketAddrINType *sockaddrPtr, const char *host, int port) {
    NetHostInfoBufType appHostInfo; /* Buf to store results */
    NetHostInfoPtr hostent;         /* Host database entry */
    Err errno;
    
    MemSet(sockaddrPtr, sizeof(NetSocketAddrINType), 0);
    sockaddrPtr->family = netSocketAddrINET;
    sockaddrPtr->port = port;

    if (host == NULL) {
        sockaddrPtr->addr = netIPAddrLocal;
    } else {
        sockaddrPtr->addr = NetLibAddrAToIN(AppNetRefnum, (char *)host);
        if (sockaddrPtr->addr == -1) {
            hostent = NetLibGetHostByName(AppNetRefnum, (char *)host, 
                      &appHostInfo,AppNetTimeout,&errno);
            if (hostent != NULL) {
                MemMove(&sockaddrPtr->addr, hostent->addrListP[0], 
                        hostent->addrLen);
            } else {
                Alert("Cannot find host.  Errno = %lx", (long)errno);
                return FALSE;    /* error */
            }
        }
    }
    return TRUE;    /* Success. */
}

static char*
getLine(NetSocketRef sock)
{
    Handle handle;
    char *buffer;
    int length = 0;
    Err errno; 
    handle = MemHandleNew(100);
    buffer = MemHandleLock(handle);

    for(;;) {
        char c;
        int n = NetLibReceive(AppNetRefnum, 
                  sock, &c, 1, 0,0,0,AppNetTimeout,&errno);
    if (n < 0) { 
        Alert("NetLibReceive returned error %ld", (long)errno);
        goto error;
    } else if (n == 0) { 
            buffer[length] = 0;
            return buffer;
        } else if (c == '\r') {
            // ignore
            continue;
        } else { 
            if (length + 2 >= MemPtrSize(buffer)) {
                long newSize = MemPtrSize(handle) + 50;
                MemHandleUnlock(handle);
                errno = MemHandleResize(handle, newSize);
                if (errno) { 
                    Alert("Unable to increase size of handle to %ld", newSize);
                    goto error;
                }
                buffer = MemHandleLock(handle);
            }
            buffer[length++] = c;
            if (c == '\n') {
                buffer[length] = '\0';
                return buffer;
            }
        }
    }
 error:
    MemPtrFree(buffer);
    return NULL;
}

static Err 
netlibReadBytes(void *sock, char *record, long offset, ULong *lengthP) {
    ULong maxRead = *lengthP;
    ULong actualRead;
    Err errno;
    actualRead = NetLibDmReceive(AppNetRefnum, (NetSocketRef)sock, record,
                                 offset, maxRead, 0,0,0,
                                 AppNetTimeout, &errno);
    *lengthP = actualRead;
    return errno;
}
    
