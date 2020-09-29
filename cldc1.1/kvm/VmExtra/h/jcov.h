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
 * SUBSYSTEM: JCOV
 * FILE:      jcov.h
 * OVERVIEW:  Java Coverage tool.  Used for analyzing the execution
 *            of the Java programs that are run in the KVM.
 * AUTHOR:    Alex Kuzmin
 *=======================================================================*/

#ifndef _JCOV_
#define _JCOV_


#ifdef PILOT
#define jcovFprintf HostFPrintF
#define jcovFopen   HostFOpen
#define jcovFclose  HostFClose
#define jcovFile    HostFILE
#define getJcovFileName  JCOV_GET_OUTPUT_FILENAME
#define JCOVLIMIT   JCOV_GET_LIMIT   
#else
#define jcovFprintf fprintf
#define jcovFopen   fopen
#define jcovFclose  fclose 
#define jcovFile    FILE
#define getJcovFileName getEnvJcovFileName
#define JCOVLIMIT   300000   
#endif

#define jCSize sizeof(struct jcovClassStruct)
#define jMSize sizeof(struct jcovMethodStruct)
#define jOSize sizeof(struct jcovOffsetStruct)

   typedef struct jcovClassStruct*  jCLASS;
   typedef struct jcovMethodStruct* jMETHOD;
   typedef struct jcovOffsetStruct* jOffset;

   extern jCLASS jcovtable;
   extern char* jcovfilename;
   extern unsigned long jcovBCcounter;
   
   struct jcovClassStruct {
       unsigned long classID;
       CLASS className;
       jMETHOD methodlist;
       jCLASS next;
   };
   
   struct jcovMethodStruct {
       unsigned long methodID;
       METHOD methodName;
       
       jOffset offsetlist;
       jMETHOD next;
   };  

   struct jcovOffsetStruct {
       unsigned long count;
       long offset;
       jOffset next;
   };

   void addTracePoint(METHOD *tm, long offset);
   void forgetLastTracePoint();
   void initJcov(const char * filename);
   void saveJcov();

#ifndef PILOT
    char * getEnvJcovFileName();
#endif

#endif /* _JCOV_ */


