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
 * SYSTEM:    KVM
 * SUBSYSTEM: networking
 * FILE:      commProtocol_md.c
 * OVERVIEW:  This file contains the native functions that implement the
 *            "comm:" protocol for the DCryptor. As the DCryptor only has
 *            one port which is always open, the open and close functions
 *            don't do anything meaningful.
 * AUTHOR:    K H Ong
 *            Doug Simon
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <uart.h>

/*=========================================================================
 * FUNCTION:        freePortError
 * OVERVIEW:        
 * INTERFACE:
 *   parameters:    *pszError: Error message
 *   returns:       <nothing>
 *=======================================================================*/
void	freePortError (char* pszError) 
{
}

/*=========================================================================
 * FUNCTION:        openPortByName
 * OVERVIEW:        Opens a serial port from a specified name ie "/dev/term/a".
 * INTERFACE:
 *   parameters:    **ppszError:    Error message
 *                  *pszDeviceName: Port number
 *                  baudRate:       The speed of the connection.
 *                  options:        Options to be used for the port.
 *                  blocking:       Sets the blocking mode.
 *   returns:       The open port
 *=======================================================================*/
long	openPortByName (char** ppszError, char* pszDeviceName, long baudRate,
			long  options, long blocking) 
{
	return (0) ;
}

/*=========================================================================
 * FUNCTION:        openPortByNumber
 * OVERVIEW:        Opens a serial port from the specified port number.
 * INTERFACE:
 *   parameters:    **ppszError: Error message
 *                  port:        Port number
 *                  baudRate:    The speed of the connection.
 *                  options:     Options to be used for the port.
 *                  blocking:    Sets the blocking mode.
 *   returns:       The open port 
 *=======================================================================*/
long	openPortByNumber (char** ppszError, long port, long baudRate, 
			  long options, long blocking) 
{
	return (openPortByName (ppszError, 0, baudRate, options, blocking)) ;
}

/*=========================================================================
 * FUNCTION:        closePort
 * OVERVIEW:        Closes the open serial port.
 * INTERFACE:
 *   parameters:    hPort:  The open port.
 *   returns:       <nothing>
 *=======================================================================*/
void	closePort (long hPort) 
{
}

/*=========================================================================
 * FUNCTION:        writeToPort
 * OVERVIEW:        Writes to the open port
 * INTERFACE:
 *   parameters:    **ppszError:            Error message
 *                  hPort:                  The name of the port to write to.
 *                  *pBuffer:               The data to be written to the port.
 *                  nNumberOfBytesToWrite:  The number of bytes to write.
 *   returns:       The number of bytes written.
 *=======================================================================*/
long	writeToPort (char** ppszError, long hPort, char* pBuffer,
		     long nNumberOfBytesToWrite) 
{
	int	i ;

	for (i = 0; i < nNumberOfBytesToWrite; i++)
	{
		uart_putc (pBuffer[i]) ;
		if (pBuffer[i] == '\n') uart_putc ('\r') ;
	}

	*ppszError = 0 ;

	return (nNumberOfBytesToWrite) ;
}

/*=========================================================================
 * FUNCTION:        readFromPort
 * OVERVIEW:        Read from the open port
 * INTERFACE:
 *   parameters:    **ppszError:          Error message
 *                  hPort:                The name of the port to read from.
 *                  pBuffer:              The buffer to store the bytes read.
 *                  nNumberOfBytesToRead: The number of bytes to read.
 *   returns:       The number of bytes read.
 *=======================================================================*/
long	readFromPort (char** ppszError, long hPort, char* pBuffer,
		      long nNumberOfBytesToRead) 
{
	int	i ;

	for (i = 0; i < nNumberOfBytesToRead; i++)
		pBuffer[i] = uart_getc() ;

	*ppszError = 0 ;

	return (nNumberOfBytesToRead) ;
}

/*=========================================================================
 * FUNCTION:        peekFromPort
 * OVERVIEW:        Returns the available number of characters on the port
 *                  for reading.
 *                  DCryptor does not support this kind of query on the
 *                  UART port and so this function simply returns 0.
 * INTERFACE:
 *   parameters:    hPort:  The open port.
 *   returns:       <nothing>
 *=======================================================================*/
long	peekFromPort (long hPort) 
{
    return 0;
}


