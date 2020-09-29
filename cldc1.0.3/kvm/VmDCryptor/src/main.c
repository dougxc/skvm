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
 * SUBSYSTEM: Main program
 * FILE:      main.c
 * OVERVIEW:  Main program for the DCryptor environment
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Dcrypt entry point by K. H. Ong
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <stdlib.h>

#include <flash.h>
#include <fx.h>
#include <uart.h>
#include <uart5.h>

#include <global.h>
/*
 * This must come after <global.h> otherwise the GNU C compiler will try
 * to use its own builtin versions of the string functions which will
 * conflict with the definitions in machine_md.h.
 */
#include <string.h>

/* Timer services */
#include "timer.h"

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

/*
 * Max length of command line to start VM.
 */
#define COMMAND_LINE_BUF_SIZE 1024

#ifdef SIMULATOR
void InitializeSimulator();
void FinalizeSimulator();
#endif

/*
 * Timeout (in milliseconds to wait for user choice before automatically
 * launching AUTO_APP). Set to 0 for no timeout.
 */
#define AUTO_APP_TIMEOUT 5000
#define AUTO_APP         "AudioApp"

/*=========================================================================
 * Functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      printVMHelp
 * TYPE:          startup
 * OVERVIEW:      Display the available VM command line options.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/
void printVMHelp() {
    fprintf(stdout, "Usage: kvm <-options> <classfile>\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -version\n");

#if INCLUDEDEBUGCODE

#define PRINT_TRACE_OPTION(varName, userName) \
    fprintf(stdout, "  " userName "\n");
FOR_EACH_TRACE_FLAG(PRINT_TRACE_OPTION)
    fprintf(stdout, "  -traceall (activates all tracing options above)\n");
#endif /* INCLUDEDEBUGCODE */
}

#ifndef BUILD_VERSION
#define BUILD_VERSION "generic"
#endif

/*=========================================================================
 * FUNCTION:      StartKVM
 * TYPE:          startup
 * OVERVIEW:      This is the command-line(ish) entry point for the KVM. The
 *                only real difference from the standard 'main' function is
 *                that command line arguments start at 0 (i.e. there is
 *                no reference to the program name).
 *
 *                All options that don't have any meaning on the DCryptor
 *                platform have been removed for clarity (e.g. -classpath,
 *                -heapsize as well as debugger options).
 * INTERFACE:
 *   parameters:  argc, argv
 *   returns:     VM exit status
 *=======================================================================*/
int StartKVM (int argc, char* argv[])
{
    int result;
    while (argc > 1) {    
        if (strcmp(argv[0], "-version") == 0) {
            fprintf(stdout, "Version: %s\n", BUILD_VERSION);
            exit(1);           
        } else if (strcmp(argv[0], "-help") == 0) {
            printVMHelp();
            exit(0);
#if INCLUDEDEBUGCODE

#define CHECK_FOR_OPTION_IN_ARGV(varName, userName)  \
        } else if (strcmp(argv[0], userName) == 0) { \
            varName = 1;                             \
            argv++; argc--;                          

        FOR_EACH_TRACE_FLAG(CHECK_FOR_OPTION_IN_ARGV)

        } else if (strcmp(argv[0], "-traceall") == 0) {
#           define TURN_ON_OPTION(varName, userName) varName = 1;
            FOR_EACH_TRACE_FLAG(TURN_ON_OPTION)
            argv++; argc--;

#endif /* INCLUDEDEBUGCODE */

        } else {
            break;
        }
    }

    /* Call the portable KVM startup routine */
    result = StartJVM(argc, argv);

#if INCLUDEDEBUGCODE
#define TURN_OFF_OPTION(varName, userName) varName = 0;
    FOR_EACH_TRACE_FLAG(TURN_OFF_OPTION)
#endif /* INCLUDEDEBUGCODE */

#if ENABLEPROFILING
    /* By default, the VM prints out profiling information */
    /* upon exiting if profiling is turned on. */
    printProfileInfo();
#endif /* ENABLEPROFILING */

    /* If no classfile was provided, print help text */
    if (result == -1) printVMHelp();

    return result;
}

/*=========================================================================
 * FUNCTION:      printDCryptorMenu
 * TYPE:          startup
 * OVERVIEW:      Display the available DCryptor application menu options.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void    printDCryptorMenu(uint timeout)
{
    fprintf(stdout,"KVM Application Menu\n\n");

    fprintf(stdout,"1.\tRe-install KVM (or other app)\n");
    fprintf(stdout,"2.\tRun java application\n");
    fprintf(stdout,"3.\tDownload JAR file (max size = %d bytes)\n",
            MAX_JAR_SIZE - 200);
#ifdef SIMULATOR
    fprintf(stdout,"q.\tQuit simulator\n");
#endif
    if (timeout != 0)
        fprintf(stdout,
            "\n(Default action will occur after %d second timeout)\n",
            timeout);
    fprintf(stdout,"\nChoice : ");
}

/*=========================================================================
 * FUNCTION:      main/start
 * TYPE:          startup
 * OVERVIEW:      This is the DCryptor entry point for the KVM. It is
 *                oriented to a terminal application (e.g. Minicom on Linux
 *                or HyperTerminal on Windows). The name of the function
 *                depends of whether we are building for the DCryptor
 *                device itself or the simulator. Only the latter can use
 *                main as this function name has special meaning to the GNU
 *                C compiler in that it generates extra startup and
 *                shutdown routines that are not supported by the DCryptor.
 * INTERFACE:
 *   parameters:  <ignored as this is a menu-driven application manager>
 *   returns:     VM exit status
 *=======================================================================*/

#define OUTPUT_HACK 0
#if OUTPUT_HACK
void uart5_putc_block(char ch) {
    uart_putc(ch);
    while(pcmcia_online() == 0 || uart5_putc_rdy() == 0);
    uart5_putc(ch);
}
char uart5_getc_block(void) {
    char ch;
    while(pcmcia_online() == 0 || uart5_getc_rdy() == 0);
    ch = (char)uart5_getc();
    uart_putc(ch);
    return ch;
}
#endif

#ifdef SIMULATOR
int main(int argc, char** argv)
#else
int start(void)
#endif    
{
    char    ch, commandLine[COMMAND_LINE_BUF_SIZE];
    char   *commandLinePtr;
    char   *KVM_argv[COMMAND_LINE_BUF_SIZE / 2], *arg;
    int     KVM_argc, timeout;

#ifdef SIMULATOR
    /*
     * Initializes the simulator.
     */
    InitializeSimulator(argc,argv);
#endif
    
    fprintf(stdout,"\n\n\n");
    fprintf(stdout,"\t*************** KVM/SVM ***************\n\n");
    fprintf(stdout,"GC Heap Size: %d bytes\n\n",HEAP_SIZE_IN_CELLS * CELL);
    /*
     * Setup the timeout app.
     */
    strcpy (commandLine, AUTO_APP) ;
    KVM_argv[0] = NULL;
    /*
     * Main processing loop with an interactive menu
     */
    while (1)
    {
        timeout = AUTO_APP_TIMEOUT;
        ch = '2' ;
        printDCryptorMenu(AUTO_APP_TIMEOUT/1000) ;

        /*
         * Launch a program automatically if the user doesn't reply within
         * a certain timeframe.
         */
        timer_init (TIMEOUT_ID_1) ;

        if (AUTO_APP_TIMEOUT == 0) {
            ch = uart_getc();
            fprintf(stdout,"%c\n\n",ch);
        }
        else {
            while (timer_timeout (TIMEOUT_ID_1, AUTO_APP_TIMEOUT) == ERROR) {
                if (uart_stat() & 0x1) {
                    ch = uart_getc();
                    timeout = 0 ;
                    fprintf(stdout,"%c\n\n",ch);
                    break ;
                }
            }
        }

        if (ch == '1')
        {
            /* Flash in new version of KVM */

            fprintf(stdout,"Use XModem to transfer application ...\n") ;
            fx_load (1) ;
        }
        else if (ch == '2')
        {   
#if OUTPUT_HACK
char* testStr = "testing console\r\n\r\n";            
            ConsoleGetCFunction = uart5_getc_block;
            ConsolePutCFunction = uart5_putc_block;
while (*testStr != 0) (*ConsolePutCFunction)(*(testStr++));
            uart5_init();
            uart5_enable();

            if (TRUE) {
#else            
            if (timeout == 0) {
#endif
                fprintf(stdout,"Enter VM args: ");
                uart_getline(commandLine,COMMAND_LINE_BUF_SIZE);
            }
            else {
                fprintf(stdout,"<timed out>\nAuto running VM with args '%s'\n",
                       commandLine);
            } 
         
            /*
             * Tokenize command line to create argv and argc
             */
            KVM_argc = 0;
            arg = strtok_r(commandLine," 	",&commandLinePtr);
            while (arg != NULL) {
                KVM_argv[KVM_argc++] = arg;
                arg = strtok_r(NULL," 	",&commandLinePtr);
            }
            StartKVM (KVM_argc,KVM_argv) ;
            /*
             * Revert command line.
             */
            KVM_argc--;
            while (KVM_argc > 0) {
                (KVM_argv[KVM_argc - 1])[strlen(KVM_argv[KVM_argc - 1])] = ' ';
                KVM_argc--;
            }
#if OUTPUT_HACK
            ConsoleGetCFunction = uart_getc;
            ConsolePutCFunction = uart_putc;
#endif
        }
        else if (ch == '3')
        {
            /* Flash java application */

            fprintf(stdout,"Use XModem to transfer JAR file ...\n") ;
            fx_load (JAR_FLASH_BLOCK) ;
        }
#ifdef SIMULATOR
        /*
         * Give the simulator user a clean way to exit.
         */
        else if (ch == 'q') {
            break;
        }
#endif
        else {
            fprintf(stdout,"Invalid choice...\n\n");
        }
       
        fprintf(stdout,"\n") ;
    }

#ifdef SIMULATOR
    FinalizeSimulator();
#endif
    
    return(0) ;
}
