/* 
   Netdrive Samba client plugin
   general utility functions
   Copyright (C) netlabs.org 2003-2008

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
*/

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "smbwrp.h"

// map errno errors to API errors
int maperror(int rc)
{
	switch (rc)
	{
		case 0 : return NO_ERROR ; /* NO_ERROR */
		case 1 : return ERROR_ACCESS_DENIED ; /* EPERM -  Operation not permitted               */
		case 2 : return ERROR_FILE_NOT_FOUND ; /* ENOENT -  No such file or directory             */
		case 3 : return ERROR_PID_MISMATCH ; /* ESRCH -  No such process                       */
		case 4 : return ERROR_INTERRUPT ; /* EINTR -  Interrupted system call               */
		case 5 : return ERROR_READ_FAULT ; /* EIO -  I/O error                             */
		case 6 : return ERROR_BAD_UNIT ; /* ENXIO -  No such device or address             */
		case 7 : return ERROR_INVALID_DATA ; /* E2BIG -  Arguments or environment too big      */
		case 8 : return ERROR_BAD_EXE_FORMAT ; /* ENOEXEC -  Invalid executable file format        */
		case 9 : return ERROR_INVALID_HANDLE ; /* EBADF -  Bad file number                       */
		case 10 : return ERROR_NO_CHILD_PROCESS ; /* ECHILD -  No child processes                    */
		case 11 : return ERROR_BUSY ; /* EAGAIN -  Resource temporarily unavailable      */
		case 12 : return ERROR_NOT_ENOUGH_MEMORY ; /* ENOMEM -  Not enough memory                     */
		case 13 : return ERROR_ACCESS_DENIED ; /* EACCES -  Permission denied                     */
		case 14 : return ERROR_INVALID_ADDRESS ; /* EFAULT -  Bad address                           */
		case 15 : return ERROR_NOT_LOCKED ; /* ENOLCK -  No locks available                    */
		case 16 : return ERROR_BUSY ; /* EBUSY -  Resource busy                         */
		case 17 : return ERROR_FILE_EXISTS ; /* EEXIST -  File exists                           */
		case 18 : return ERROR_NOT_SAME_DEVICE ; /* EXDEV -  Cross-device link                     */
		case 19 : return ERROR_REM_NOT_LIST ; /* ENODEV -  No such device                        */
		case 20 : return ERROR_PATH_NOT_FOUND ; /* ENOTDIR -  Not a directory                       */
		case 21 : return ERROR_DIRECTORY ; /* EISDIR -  Is a directory                        */
		case 22 : return ERROR_INVALID_PARAMETER ; /* EINVAL -  Invalid argument                      */
		case 23 : return ERROR_TOO_MANY_OPEN_FILES ; /* ENFILE -  Too many open files in system         */
		case 24 : return ERROR_TOO_MANY_OPENS ; /* EMFILE -  Too many open files                   */
		case 25 : return ERROR_MOD_NOT_FOUND ; /* ENOTTY -  Inappropriate ioctl                   */
		case 26 : return ERROR_LOCK_VIOLATION ; /* EDEADLK -  Resource deadlock avoided             */
		case 27 : return ERROR_TRANSFER_TOO_LONG ; /* EFBIG -  File too large                        */
		case 28 : return ERROR_DISK_FULL ; /* ENOSPC -  Disk full                             */
		case 29 : return ERROR_SEEK ; /* ESPIPE -  Invalid seek                          */
		case 30 : return ERROR_WRITE_PROTECT ; /* EROFS -  Read-only file system                 */
		case 31 : return ERROR_TOO_MANY_OPEN_FILES ; /* EMLINK -  Too many links                        */
		case 32 : return ERROR_BROKEN_PIPE ; /* EPIPE -  Broken pipe                           */
		case 33 : return ERROR_INVALID_LEVEL ; /* EDOM -  Domain error                          */
		case 34 : return ERROR_FILENAME_EXCED_RANGE ; /* ERANGE -  Result too large                      */
		case 35 : return ERROR_DIR_NOT_EMPTY ; /* ENOTEMPTY -  Directory not empty                   */
		case 36 : return ERROR_BUSY_DRIVE ; /* EINPROGRESS -  Operation now in progress             */
		case 37 : return ERROR_INVALID_FUNCTION ; /* ENOSYS -  Function not implemented              */
		case 38 : return ERROR_FILENAME_EXCED_RANGE ; /* ENAMETOOLONG -  File name too long                    */
		case 39 : return ERROR_KBD_FOCUS_REQUIRED ; /* EDESTADDRREQ -  Destination address required          */
		case 40 : return ERROR_TRANSFER_TOO_LONG ; /* EMSGSIZE -  Message too long                      */
		case 48 : return ERROR_NETWORK_BUSY ; /* EADDRINUSE -  Address already in use                */
		case 49 : return ERROR_INFO_NOT_AVAIL ; /* EADDRNOTAVAIL -  Can't assigned requested address      */
		case 50 : return ERROR_NETWORK_ACCESS_DENIED ; /* ENETDOWN -  Network is down                       */
		case 51 : return ERROR_NETWORK_ACCESS_DENIED ; /* ENETUNREACH -  Network is unreachable                */
		case 52 : return ERROR_NETWORK_ACCESS_DENIED ; /* ENETRESET -  Network dropped connection on reset   */
		case 53 : return ERROR_NETWORK_ACCESS_DENIED ; /* ECONNABORTED -  Software caused connection abort      */
		case 54 : return ERROR_NETWORK_ACCESS_DENIED ; /* ECONNRESET -  Connection reset by peer              */
		case 55 : return ERROR_BUFFER_OVERFLOW ; /* ENOBUFS -  No buffer space available             */
		case 56 : return ERROR_PIPE_BUSY ; /* EISCONN -  Socket is already connected           */
		case 57 : return ERROR_REM_NOT_LIST ; /* ENOTCONN -  Socket is not connected               */
		case 58 : return ERROR_ALREADY_SHUTDOWN ; /* ESHUTDOWN -  Can't send after socket shutdown      */
		case 60 : return ERROR_TIMEOUT ; /* ETIMEDOUT -  Connection timed out                  */
		case 61 : return ERROR_NETWORK_ACCESS_DENIED ; /* ECONNREFUSED -  Connection refused                    */
		case 63 : return ERROR_INVALID_BLOCK ; /* ENOTSOCK -  Socket operation on non-socket        */
		case 64 : return ERROR_BAD_FORMAT ; /* EHOSTDOWN -  Host is down                          */
		case 65 : return ERROR_BAD_NETPATH ; /* EHOSTUNREACH -  No route to host                      */
		case 66 : return ERROR_BUSY_DRIVE ; /* EALREADY -  Operation already in progress         */
	}
//	debug_printf( "Unhandled return code %d\n");
	return rc + 40000;
}

char * getlastslash(char * path)
{
	char * p;
	if (!path)
	{
		return NULL;
	}
	for (p = path + strlen(path) - 1; p >= path; p--)
	{
		if (*p == '\\' || *p == '/')
		{
			return p;
		}
	}
	return NULL;
}

