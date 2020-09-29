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
 * FILE:      jcov.c
 * OVERVIEW:  Java coverage tool.  Used for analyzing the execution
 *            of the Java programs that are run in the KVM.
 * AUTHOR:    Alex Kuzmin
 *=======================================================================*/

/*=========================================================================
 * Local include files
 *=======================================================================*/

#include <global.h>
#include <execute.h>
#include <jcov.h>

/*=========================================================================
 * Variables
 *=======================================================================*/

   jCLASS jcovtable = NULL;
   char* jcovfilename = NULL;
   char* defaultjfilename = "jcov.dat";
   jcovFile *jcovfile = NULL;

   static INSTANCE_CLASS className;
   static METHOD methodName;
   unsigned long jcovBCcounter=0;

   char tmpcname[256];
   char tmpsname[256];
   
   static unsigned long classId, methodId;
   static long offset=-1;
   
   static int saveLast=0;

/*=========================================================================
 * Functions
 *=======================================================================*/

   void saveLastTracePoint() {
       jCLASS tmpc ,prevc;
       jMETHOD tmpm ,prevm;
       jOffset tmpo ,tmpn, prevo;

       tmpc = jcovtable;
       prevc=NULL;
           while (tmpc!=NULL) {                       /* Scan classes */
               if (tmpc->classID==classId) {                                  
               
                   tmpm=tmpc->methodlist;
                   prevm=NULL;
                   while (tmpm!=NULL) {               /* Scan methods */
                       if (tmpm->methodID == methodId) {

                           tmpo=tmpm->offsetlist;
                           prevo=NULL;                /* Scan offsets */
                           while (tmpo!=NULL && (tmpo->offset<=offset)) {
                           
                               if (tmpo->offset==offset) {
                                   tmpo->count++;           
                                   return;
                               }
                               prevo=tmpo;
                               tmpo=tmpo->next;
                           }
                           
                           if (tmpo==NULL) {
                               /* new offset executed in given method*/
                               tmpo=(jOffset)malloc(jOSize);
                               tmpo->next=NULL;
                               tmpo->offset=offset;
                               tmpo->count=1;
                               prevo->next=tmpo;
                               jcovBCcounter++;
                               return; 
                           } else {
                               tmpn=(jOffset)malloc(jOSize);
                               tmpn->next=tmpo;
                               tmpn->offset=offset;
                               tmpn->count=1;
                               if (prevo!=NULL) {
                                   prevo->next=tmpn;
                               } else {
                                   tmpm->offsetlist=tmpn;
                                   tmpn->next=tmpo;
                               }
                               jcovBCcounter++;
                               return;                           
                           }
                           
                       } /* Found the method*/
                       prevm=tmpm;
                       tmpm=tmpm->next;              
                   }
                   /*new method found*/
                   tmpm=(jMETHOD)malloc(jMSize);

                   tmpm->methodName=methodName;

                   tmpm->next=NULL;
                   tmpm->methodID = methodId;
                   
                   tmpm->offsetlist=(jOffset)malloc(jOSize);
                   tmpm->offsetlist->next=NULL;                   
                   tmpm->offsetlist->offset=offset;
                   tmpm->offsetlist->count=1;
                   prevm->next=tmpm;                                      
                   return;                      
               } /* Found the class */
               prevc=tmpc;
               tmpc=tmpc->next;          
           }
           /* New class found */
           tmpc = (jCLASS)malloc(jCSize);
           tmpc->next=NULL;
           
           tmpc->className=(CLASS)className;

           tmpc->classID = classId;
           
           tmpc->methodlist=(jMETHOD)malloc(jMSize);
           tmpc->methodlist->methodID=methodId;
           tmpc->methodlist->next=NULL;           

           tmpc->methodlist->methodName=methodName;
           
           tmpc->methodlist->offsetlist=(jOffset)malloc(jOSize);
           tmpc->methodlist->offsetlist->next=NULL;           
           tmpc->methodlist->offsetlist->offset=offset;
           tmpc->methodlist->offsetlist->count=1;
           
           if (jcovtable==NULL) {
               jcovtable=tmpc;
           } else {
               prevc->next=tmpc;
           }                     
           return;                    
       }

   
       void addTracePoint(METHOD *tm, long newoffset) {
          METHOD thisMethod = *tm;

          if (saveLast) {
              saveLastTracePoint();    
          }
        
          START_TEMPORARY_ROOTS
          ASSERTING_NO_ALLOCATION 

          offset=newoffset;
                    
          methodName=thisMethod;
          className=thisMethod->ofClass;
          classId=(unsigned long)(&thisMethod->ofClass->clazz);
          methodId=(unsigned long)(thisMethod - \
              thisMethod->ofClass->methodTable->methods);

          END_ASSERTING_NO_ALLOCATION 
          END_TEMPORARY_ROOTS
                         
          saveLast=1;
      }

      
      void initJcov(const char * filename) {
          if (filename == NULL) {
              filename=getJcovFileName();
              if (filename == NULL) { 
                  filename=defaultjfilename;
              }
          }

          jcovfile = jcovFopen(filename, "ab+"); 

          if (jcovfile==NULL) {
              fprintf(stderr,"Unable to write to %s\n",filename);
#ifndef PILOT
              exit(13);       
#endif
          }  
      }


      /* Prints the jcov table and frees its memory*/
      void saveJcov() {
          jCLASS tmpc;
          jMETHOD tmpm;
          jOffset tmpo;
          void * tmpp;
            
          saveLastTracePoint();        

          tmpc = jcovtable;
          while (tmpc!=NULL) {           /* Scan classes */
              tmpm=tmpc->methodlist;
              while (tmpm!=NULL) {       /* Scan methods */
                  tmpo=tmpm->offsetlist;
                  while (tmpo!=NULL) {   /* Scan offsets */

          START_TEMPORARY_ROOTS
          ASSERTING_NO_ALLOCATION 

          getClassName_inBuffer((CLASS)tmpc->className, tmpcname);
          change_Key_to_MethodSignature_inBuffer(\
              tmpm->methodName->nameTypeKey.nt.typeKey,tmpsname);
                          
                      jcovFprintf(jcovfile, \
                          "%s %s %s %ld %ld\n", tmpcname,  \
                          methodName(tmpm->methodName), tmpsname,\
                          tmpo->offset,\
                          tmpo->count);
                      tmpp=tmpo;
                      tmpo=tmpo->next;
                      free(tmpp);
          END_ASSERTING_NO_ALLOCATION 
          END_TEMPORARY_ROOTS

                  }
                  tmpp=tmpm;                                   
                  tmpm=tmpm->next;
                  free(tmpp);              
              }
              tmpp=tmpc;
              tmpc=tmpc->next;          
              free(tmpp);
          }                               
          jcovtable=NULL;
          jcovBCcounter=0;
          jcovFclose(jcovfile);    
      }


    void forgetLastTracePoint() {
        saveLast=0;
    }

#ifndef PILOT
    char * getEnvJcovFileName() {

          char * filename = getenv("jcovfile");

          if (filename == NULL) {
              filename = getenv("JCOVFILE");
           }
         return filename;
    }
#endif

