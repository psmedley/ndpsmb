/* 
   Netdrive Samba client plugin
   logging functions
   Copyright (C) netlabs.org 2003-2009

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

#define INCL_DOSERRORS
#define INCL_DOS
#include <os2emx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "nversion.h"

const char * smbwrp_getVersion();
int debuglevel = 9; // we set it to 9, so we get all messages
char logfile[_MAX_PATH +1];
char debugfile[_MAX_PATH +1];
char logfilename[] = "log.ndpsmb";
BOOL do_logging;
BOOL firstLogLine;
HMTX logMutex;
char nameMutex[] = "\\SEM32\\NDPSMB";

int debuglvl(int level)
{
	return (level <= debuglevel) ? 1 : 0;
}

BOOL writeLog()
{
        return do_logging;
}

void debugInit()
{
        *logfile = '\0';
        *debugfile = '\0';
        do_logging = FALSE;
        firstLogLine = TRUE;
        logMutex = NULLHANDLE;
        struct stat filestat;
        APIRET rc = NO_ERROR;

        // create the debugfile name
        strncat(debugfile, getenv("ETC"), 2);
        strncat(debugfile, "\\", sizeof(debugfile) - strlen(debugfile) -1);
        strncat(debugfile, "ndpsmb.dbg", sizeof(debugfile) - strlen(debugfile) -1);

        // is the file around? if not we have no debug
        if (stat(debugfile, &filestat) !=0)
           return;

        //create the logfile variable
        char *env = getenv("LOGFILES");
        if (env != NULL)
        {
           strncat(logfile, env, sizeof(logfile) -1);
           strncat(logfile, "\\", sizeof(logfile) - strlen(logfile) -1);
           strncat(logfile, logfilename, sizeof(logfile) - strlen(logfile) -1);  
        }
        else
        {
           strncat(logfile, logfilename, sizeof(logfile) -1);
        }
        // set the samba logging stuff
        do_logging = TRUE;

        // now we create a sem, so that logging from different threads works
        rc = DosCreateMutexSem(nameMutex, &logMutex, 0, FALSE);
        if (rc == ERROR_DUPLICATE_NAME)
        {
        rc = DosOpenMutexSem(nameMutex, &logMutex);
        }

        return;
}

void debugDelete()
{
        DosCloseMutexSem(logMutex);
        return;
}

void debuglocal(int level, const char * fmt, ...)
{
        FILE *f=NULL;

        // do we have to log at all
	if (!do_logging)
	{
		return;
	}

        // if the sem is created we request it
        if (logMutex)
        {
           DosRequestMutexSem(logMutex, (ULONG) SEM_INDEFINITE_WAIT);
        }

	do
	{
		struct timeval tv;
		char buf[80] = {0};
		va_list args;
		if (logfile[0])
		{
			f = fopen(logfile, "a");
			if (!f)
			{
				break;
			}
		}
		else
		{
			f = stdout;
		}

                // in the first log line we write our version of the client
                if (firstLogLine)
                {
                   fprintf(f, "Samba client %s build %s based on %s\n", VERSION, BUILD, smbwrp_getVersion());
                   fprintf(f, "This build is maintained by %s\n", VENDOR);
                   firstLogLine = FALSE;
                }

		gettimeofday(&tv, NULL);
		strftime(buf,sizeof(buf)-1,"%Y/%m/%d %H:%M:%S", localtime((time_t *)&tv.tv_sec));
		fprintf(f, "%s.%02d: %d %d: ", buf, tv.tv_usec / 10000, level, (long)_gettid());;
		va_start(args, fmt);
		vfprintf(f, fmt, args);
		va_end(args);
		if (logfile)
		{
//			fflush(f);
                        fclose(f);
		}
	}
	while (0);

        // if the sem is created we release it
        if (logMutex)
        {
           DosReleaseMutexSem(logMutex);
        }

        return;
}
