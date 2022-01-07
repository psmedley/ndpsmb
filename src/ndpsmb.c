/* 
   Netdrive Samba client plugin
   plugin API
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define NDPL_LARGEFILES
#define INCL_LONGLONG
#include <ndextpl2.h>
#include "smbwrp.h"
#include "util.h"

void debugInit();
void debugDelete();
#define debug_printf(...) debuglocal(9, __VA_ARGS__)

// -------------------------------------------------------------

/* time conversion functions: SMB protocol sends timestamps in GMT time,
*  os2 api uses localtime,
*  emx/klibc uses timezone and daylight saving to convert GMT timestamps,
*  so only the timezone must be counted in conversion.
*/
void fsphUnixTimeToDosDate( time_t time, FDATE* fdate, FTIME *ftime)
{
	struct tm* gmt = localtime( &time);
#if 0 // as localtime() already does dst we don't need to add something
	if (gmt->tm_isdst>0) {
	debug_printf( "daylight saving in effect %d, timezone %d\n",gmt->tm_isdst, timezone); 
	time -= 3600;
	gmt = localtime( &time);
	}
#endif
	fdate->day  = gmt->tm_mday;
	fdate->month  = gmt->tm_mon+1;
	fdate->year = gmt->tm_year + 1900 - 1980;
	ftime->twosecs = gmt->tm_sec/2;
	ftime->minutes = gmt->tm_min;
	ftime->hours = gmt->tm_hour;
}

void fsphDosDateToUnixTime( FDATE fdate, FTIME ftime, ULONG* time)
{
	struct tm gmtime = { 0 };

	debug_printf( "fsphDosDateToUnixTime time %02d:%02d:%02d\n", ftime.hours, ftime.minutes, ftime.twosecs*2);
	gmtime.tm_mday = fdate.day;
	gmtime.tm_mon = fdate.month-1;
	gmtime.tm_year = fdate.year + 1980 - 1900;
	gmtime.tm_sec = ftime.twosecs*2;
	gmtime.tm_min = ftime.minutes;
	gmtime.tm_hour = ftime.hours;
	gmtime.tm_isdst = -1; // force libc to check dst saving

	*time = mktime( &gmtime);
	debug_printf( "fsphDosDateToUnixTime time1 %d %s", *time, ctime( (time_t*)time));
#if 0 // as mktime() already does dst we don't need to add something
	struct tm* gmt;
	gmt = localtime( (time_t*) time);
	if (gmt->tm_isdst>0) {
	debug_printf( "fsphDosDateToUnixTime daylight saving in effect %d, timezone %d\n",gmt->tm_isdst, timezone); 
	*time += 3600;
	}
	debug_printf( "fsphDosDateToUnixTime time2 %d %s", *time, ctime( (time_t*)time));
#endif
}
	
// -------------------------------------------------------------

/* uppercased type of resource */
const char *NdpTypes[] =
{
	"SMBFS",
	NULL
}
;

/* Properties of supported resource types */

/* Properties of resource */
static const NDPROPERTYINFO smbProperties[] =
{
	{ND_PROP_STRING, 0, "WORKGROUP", ""},
	{ND_PROP_STRING, 0, "SERVER", ""},
	{ND_PROP_STRING, 0, "SHARE", ""},
	{ND_PROP_STRING, 0, "USER", "guest"},
	{ND_PROP_STRING, 0, "PASSWORD", ""},
	{ND_PROP_STRING, 0, "SPASSWORD",  ""},
	{ND_PROP_STRING, 0, "MASTER", "WORKGROUP"},
	{ND_PROP_ULONG, 0, "MASTERTYPE", "1"},
	{ND_PROP_ULONG, 0, "CTO", "10"},
	{ND_PROP_ULONG, 0, "CLD", "32"},
	{ND_PROP_ULONG, 0, "EASUPPORT", "1"},
	{ND_PROP_ULONG, 0, "KRB5SUPPORT", "0"},
	{ND_PROP_ULONG, 0, "NTLMv1SUPPORT", "0"},
	{ND_PROP_ULONG, 0, "ENCRYPTIONSUPPORT", "0"},
	{ND_PROP_STRING, 0, NULL, NULL}
};


/* Exported array of properties */
const NDPROPERTYINFO *NdpPropertiesInfo[] =
{
	smbProperties
};


PLUGINHELPERTABLE2L *ph;
static int ifL;

/* A mutex to serialize plugin calls because libsmb may not be thread safe. */
static NDMUTEX mutex;

static int lockInit (void)
{
	return ph->fsphCreateMutex (&mutex);
}

static void lockClose (void)
{
	ph->fsphCloseMutex (mutex);
}

static int lockRequest (void)
{
	return ph->fsphRequestMutex (mutex, SEM_INDEFINITE_WAIT);
}

static void lockRelease (void)
{
	ph->fsphReleaseMutex (mutex);
}

#if LIBSMB_THREAD_SAFE==0

#define ENTER() do {			\
	int rcLock = lockRequest();	\
	if (rcLock != NO_ERROR)		\
		return rcLock;		\
} while (0)

#define LEAVE() do {	\
	lockRelease();	\
} while (0)

#else
#define ENTER() do { /* nothing */ } while (0)
#define LEAVE() do { /* nothing */ } while (0)
#endif

int helperEASet (cli_state *cli, FEALIST *pFEAList, char *path)
{
	int rc = 0;

	if (!path || !pFEAList || pFEAList->cbList <= sizeof(long))
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	ENTER();

	do {
		// got FEA there
		FEA * pfea;
		unsigned long done = sizeof(long);
		pfea = pFEAList->list;
		while (done < pFEAList->cbList)
		{
			rc = smbwrp_setea(cli, path, (char*)(pfea + 1), pfea->cbValue ? (char *)(pfea + 1) + pfea->cbName + 1: NULL, pfea->cbValue);
			if (rc)
			{
				break;
			}
			pfea = (FEA *)((char *)(pfea + 1) + pfea->cbName + 1 + pfea->cbValue);
			done += sizeof(FEA) + pfea->cbName + 1 + pfea->cbValue;
		}
	} while (0);
	LEAVE();
	return rc;
}

int APIENTRY NdpPluginLoad (PLUGINHELPERTABLE2L *pPHT)
{
	int rc;
	HPIPE pipe;
	unsigned long action;
	ph = pPHT;
	ifL = 0;
/*
	if (ph->cb < sizeof (PLUGINHELPERTABLE2))
	{
		return ERROR_INVALID_FUNCTION;
	}
*/
	if (ph->cb >= sizeof (PLUGINHELPERTABLE2L))
	{
		ifL = 1;
	}
	lockInit();
	debugInit();
	debuglocal(9,"Working with %s bit fileio NDFS\n", ifL ? "64" : "32");
	return NO_ERROR;
}


int APIENTRY NdpPluginFree (void)
{
	debugDelete();
	lockClose();
	return NO_ERROR;
}


void getfindinfo(Connection * pConn, FILEFINDBUF3 * stat, smbwrp_fileinfo * finfo)
{
	char * name = ph->fsphStrRChr(finfo->fname, '\\');
	if (name)
	{
		name++;
	}
	else
	{
		name = finfo->fname;
	}
	if (!*name)
	{
		name = pConn->pRes->srv.share_name;
	}
	strncpy(stat->achName, name, CCHMAXPATHCOMP - 1);
	stat->cbFile = finfo->size;
	stat->cbFileAlloc = stat->cbFile;
	stat->oNextEntryOffset = 0ul;
	stat->cchName = strlen(stat->achName);
	stat->attrFile = (finfo->attr & 0x37);

	fsphUnixTimeToDosDate(finfo->mtime, &stat->fdateLastWrite, &stat->ftimeLastWrite);
	fsphUnixTimeToDosDate(finfo->btime, &stat->fdateCreation, &stat->ftimeCreation);
	fsphUnixTimeToDosDate(finfo->atime, &stat->fdateLastAccess, &stat->ftimeLastAccess);
}

int getfindinfoL(Connection * pConn, void * plist, smbwrp_fileinfo * finfo, ULONG ulAttribute, char * mask)
{
	FILESTATUS3L stat = {0};
	char * name = ph->fsphStrRChr(finfo->fname, '\\');
	if (name)
	{
		name++;
	}
	else
	{
		name = finfo->fname;
	}
	if (!*name)
	{
		name = pConn->pRes->srv.share_name;
	}
	if (mask && (!ph->fsphAttrMatch(ulAttribute, finfo->attr & 0x37) || !ph->fsphWildMatch(mask, name, ND_IGNORE_CASE)))
	{
		return 0;
	}

	stat.cbFile = finfo->size;
	stat.cbFileAlloc = stat.cbFile;
	stat.attrFile = (finfo->attr & 0x37);

	fsphUnixTimeToDosDate(finfo->mtime, &stat.fdateLastWrite, &stat.ftimeLastWrite);
	fsphUnixTimeToDosDate(finfo->btime, &stat.fdateCreation, &stat.ftimeCreation);
	fsphUnixTimeToDosDate(finfo->atime, &stat.fdateLastAccess, &stat.ftimeLastAccess);
	//debug_printf( "fname %s\n", finfo->fname);
	//debug_printf( "btime %d %s", finfo->btime, ctime( (time_t*)&finfo->btime));
	//debug_printf( "atime %d %s", finfo->atime, ctime( (time_t*)&finfo->atime));
	//debug_printf( "mtime %d %s", finfo->mtime, ctime( (time_t*)&finfo->mtime));
	//debug_printf( "ctime %d %s", finfo->ctime, ctime( (time_t*)&finfo->ctime));
	//debug_printf( "ftimeLastAccess %02d:%02d:%02d\n", stat.ftimeLastWrite.hours, stat.ftimeLastWrite.minutes, stat.ftimeLastWrite.twosecs*2);

	ph->fsphAddFile32L(plist, &stat, name, strlen(name), finfo, sizeof(*finfo), 0);
	return 1;
}

static unsigned char fromhex (char c)
{
	if ('0' <= c && c <= '9')
	{
		return c - '0';
	}

	if ('A' <= c && c <= 'F')
	{
		return c - 'A' + 0xA;
	}

	if ('a' <= c && c <= 'f')
	{
		return c - 'a' + 0xA;
	}

	return 0;
}

static char tohex (unsigned char b)
{
	b &= 0xF;

	if (b <= 9)
	{
		return b + '0';
	}

	return 'A' + (b - 0xA);
}

static void decryptPassword (const char *pszCrypt, char *pszPlain)
{
	/* A simple "decryption", character from the hex value. */
	const char *s = pszCrypt;
	char *d = pszPlain;

	while (*s)
	{
		*d++ = (char)((fromhex (*s++) << 4) + fromhex (*s++));
	}

	*d++ = 0;
}

static void encryptPassword (const char *pszPlain, char *pszCrypt)
{
	/* A simple "encryption" encode each character as hex value. */
	const char *s = pszPlain;
	char *d = pszCrypt;

	while (*s)
	{
		*d++ = tohex ((*s) >> 4);
		*d++ = tohex (*s);
		s++;
	}

	*d++ = 0;
}

/* accept parameters in form
 * [filename][;name=filename]
 */
int initResource (Resource *pRes, NDPROPERTYHANDLE properties)
{
	int rc = NO_ERROR;
	unsigned long t;
	const CHAR * q = NULL;
	int defaultPassword = 1;

	pRes->rootlevel = 0;
	pRes->easupport = 1;
	pRes->krb5support = 0;
	pRes->ntlmv1support = 0;
	pRes->encryptionsupport = 0;
	pRes->pdc = NULL;

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "WORKGROUP", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.workgroup, q, sizeof(pRes->srv.workgroup) - 1);
		pRes->rootlevel = 1;
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "SERVER", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.server_name, q, sizeof(pRes->srv.server_name) - 1);
		pRes->rootlevel = 2;
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "SHARE", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.share_name, q, sizeof(pRes->srv.share_name) - 1);
		pRes->rootlevel = 3;
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "USER", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.username, q, sizeof(pRes->srv.username) - 1);
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "PASSWORD", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.password, q, sizeof(pRes->srv.password) - 1);
		defaultPassword = 0;
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "SPASSWORD", &q, &t);
	if (   rc == NO_ERROR && *q != '\0' && defaultPassword)
	{
		char p[1024];
		p[0] = 0;

		decryptPassword (q, p);

		if (*p)
		{
			strncpy(pRes->srv.password, p, sizeof(pRes->srv.password) - 1);

			/* clear the plain password */
			ph->fsphSetProperty (properties, "PASSWORD", "");
		}
	}
	else
	{
		char c[1024];
		encryptPassword (pRes->srv.password, c);

		ph->fsphSetProperty (properties, "SPASSWORD", c);

		// clear the plain password
		ph->fsphSetProperty (properties, "PASSWORD", "");
	}

	t = 0, q = NULL;
	rc = ph->fsphQueryStringProperty (properties, "MASTER", &q, &t);
	if (!rc && t && *q)
	{
		strncpy(pRes->srv.master, q, sizeof(pRes->srv.master) - 1);
	}

	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "MASTERTYPE", &t);
	if (!rc)
	{
		if (t > 1)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->srv.ifmastergroup = t;
		}
	}

	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "EASUPPORT", &t);
	if (!rc)
	{
		if (t > 1)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->easupport = t;
		}
	}

	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "KRB5SUPPORT", &t);
	if (!rc)
	{
		if (t > 1)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->krb5support = t;
		}
	}
	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "NTLMv1SUPPORT", &t);
	if (!rc)
	{
		if (t > 1)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->ntlmv1support = t;
		}
	}
	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "ENCRYPTIONSUPPORT", &t);
	if (!rc)
	{
		if (t > 1)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->encryptionsupport = t;
		}
	}
	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "CTO", &t);
	if (!rc)
	{
		if (t > 600)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->cachetimeout = t;
		}
	}

	t = 0;
	rc = ph->fsphQueryUlongProperty (properties, "CLD", &t);
	if (!rc)
	{
		if (t > 96)
		{
			rc = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pRes->cachedepth = t;
		}
	}

	/*
	 * Create a directory cache with expiration time and cache listings
	 * the above values come from the gui. default: timeout 10; listings: 32
	 */
	dircache_create(&pRes->pdc, pRes->cachetimeout, pRes->cachedepth);

	return rc;
}

int iftestpath(char * path)
{
	char * p = path;
	if (!path)
	{
		return 0;
	}
	while ((p = ph->fsphStrChr(p, 'A')) != NULL)
	{
		if (ph->fsphStrNCmp(p, "A.+,;=[].B", 10) == 0)
		{
			return 1;
		}
		p++;
	}
	return 0;
}

int pathparser(Resource *pRes, Connection * pConn, char * path, char * result)
{
	int rootlevel;
	int rc = NO_ERROR;
	if (!pRes || !path || !result)
	{
		return ERROR_INVALID_PARAMETER;
	}
	// handle special case when someone wants to test support of LFN or smth similar
	if (iftestpath(path))
	{
		strcpy(result, "\\A.+,;=[].B");
		return NO_ERROR;
	}

	rootlevel = pRes->rootlevel;
	if (*path == '\\') path++;

	if (rootlevel < 3)
	{
		char * p;
		// flag: 1 parameters changed, reconnection required, 0 do nothing
		int newlevel = 0;
		// use a temporary resource to test disconnection/reconnection
		Resource tmpRes;
		// copy exising data
		memcpy( &tmpRes, pRes, sizeof( tmpRes));
		// pointer to new connection fields
		smbwrp_server * tmp = &tmpRes.srv;
		if (rootlevel == 0)
		{
			p = ph->fsphStrChr(path, '\\');
			if (!p)
			{
				p = path + strlen(path);
			}
			if (strlen(tmp->workgroup) != p - path
				|| (p == path || ph->fsphStrNICmp(path, tmp->workgroup, p - path)))
			{
				strncpy(tmp->workgroup, path, p - path);
				tmp->workgroup[p - path] = 0;
				newlevel = 1;
			}
			path = *p == '\\' ? p + 1 : p;
			rootlevel = 1;
		}
		if (rootlevel == 1) // root path starts from server name
		{
			p = ph->fsphStrChr(path, '\\');
			if (!p)
			{
				p = path + strlen(path);
			}
			if (strlen(tmp->server_name) != p - path
				|| (p == path || ph->fsphStrNICmp(path, tmp->server_name, p - path)))
			{
				strncpy(tmp->server_name, path, p - path);
				tmp->server_name[p - path] = 0;
				newlevel = 1;
			}
			path = *p == '\\' ? p + 1 : p;
			rootlevel = 2;
		}
		if (rootlevel == 2) // root path starts from share name
		{
			p = ph->fsphStrChr(path, '\\');
			if (!p)
			{
				p = path + strlen(path);
			}
			if (strlen(tmp->share_name) != (p - path)
				|| (p == path || ph->fsphStrNICmp(path, tmp->share_name, p - path)))
			{
				strncpy(tmp->share_name, path, p - path);
				tmp->share_name[p - path] = 0;
				newlevel = 1;
			}
			path = *p == '\\' ? p + 1 : p;
		}
		if (newlevel)
		{
			// reconnect to server here, first test new connection
			cli_state* tmp_cli = NULL;
			rc = smbwrp_connect( &tmpRes, &tmp_cli);
			if (!rc) 
			{
				// new connection is ok, disconnect old one
				cli_state* cli = pConn->cli;
				smbwrp_disconnect( pRes, cli);
				// save tmp data structure
				memcpy( pRes, &tmpRes, sizeof( tmpRes));
				// save new connection handle
				pConn->cli = tmp_cli;
			}
		}
	}

	strcpy(result, "\\");
	strncat(result, path, CCHMAXPATH);

	return rc;
}


// -------------------------------------------------------------

/* check if the requested resource is available */
static int checkMountResource( Resource* pRes)
{
	int rc;
	unsigned long action;
	cli_state* cli = NULL;

	debug_printf("checkMountResource in tid#%d\n", _gettid());
	rc = smbwrp_connect( pRes, &cli);
/* changed to real error codes SCS 
	if (rc)
		rc = (rc == 7 ? ERROR_BAD_DEV_TYPE : ERROR_ACCESS_DENIED); */
	switch (rc) {
	case 0:
		rc = NO_ERROR;
		break;
	case 1:
	case 10:
	case 11:
		rc = ERROR_BAD_NET_NAME;
		break;
	case 2:
		rc = ERROR_INIT_ROUTINE_FAILED;
		break;
	case 3:
		rc = ERROR_BAD_NET_RESP;
		break;
	case 4:
		rc = ERROR_NETWORK_BUSY;
		break;
	case 6:
		rc = ERROR_NETWORK_ACCESS_DENIED;
		break;
	case 7:
		rc = ERROR_BAD_NETPATH;
		break;
	default:
		rc = ERROR_UNEXP_NET_ERR;
	  break;
	} /* endswitch */ 

	smbwrp_disconnect( pRes, cli);

	return rc;
}

int APIENTRY NdpMountResource (HRESOURCE *presource, int type, NDPROPERTYHANDLE properties)
{
	int rc = NO_ERROR;
	unsigned long objany = OBJ_ANY;
	Resource *pRes = NULL;

	ENTER();

	debuglocal(9,"NdpMountResource in\n");

	// init code
	smbwrp_init();

	/* since samba plugin support only 1 type of resources we do not need */
	/* to check what the found type really is */
	pRes = malloc( sizeof(Resource));
	if (pRes == NULL)
	{
		rc = ERROR_NOT_ENOUGH_MEMORY;
	} 
	else
	{
		memset(pRes, 0, sizeof(Resource));
		//pRes->objany = objany;
		// parse init string
		rc = initResource (pRes, properties);
		// try to connect to resource (check type) only if thread!=1, so ndctl startup
		// is not slowed down by network connections.
		// ndctl does mounting on main thread (#1)
		// nd/ndpm do not use main thread
		if (!rc && _gettid()!=1)
			rc = checkMountResource( pRes);
		if (!rc)
		{
			// store resource data
			*presource = (HRESOURCE)pRes;
		}
		else
		{
			free(pRes);
		}
	}
	debuglocal(9,"NdpMountResource rc=%d\n", rc);
	LEAVE();
	return rc;
}

// -------------------------------------------------------------

int APIENTRY NdpFreeResource (HRESOURCE resource)
{
	Resource *pRes = (Resource *)resource;
	ENTER();
	dircache_delete(pRes->pdc);
	memset(&pRes->srv, 0, sizeof(pRes->srv));
	free(pRes);
	debuglocal(9,"NdpFreeResource %d\n", NO_ERROR);
	LEAVE();
	return NO_ERROR;
}

// -------------------------------------------------------------

int APIENTRY NdpRsrcCompare (HRESOURCE resource, HRESOURCE resource2)
{
	Resource *pRes = (Resource *)resource;
	Resource *pRes2 = (Resource *)resource2;
	int rc = ND_RSRC_DIFFERENT;

	debuglocal(9,"NdpRsrcCompare in\n");
	if (ph->fsphStrICmp(pRes->srv.server_name, pRes2->srv.server_name) == 0
		&& ph->fsphStrICmp(pRes->srv.share_name, pRes2->srv.share_name) == 0
		&& ph->fsphStrICmp(pRes->srv.username, pRes2->srv.username) == 0
		&& ph->fsphStrICmp(pRes->srv.workgroup, pRes2->srv.workgroup) == 0)
	{
		// resources are equal
		rc = ND_RSRC_EQUAL;
	}

	debuglocal(9,"NdpRsrcCompare %d\n", rc);

	return rc;
}

int APIENTRY NdpRsrcUpdate (HRESOURCE resource, HRESOURCE resource2)
{
	// do nothing
	debuglocal(9,"NdpRsrcUpdate %d\n", NO_ERROR);
	return NO_ERROR;
}

int APIENTRY NdpRsrcQueryInfo (HRESOURCE resource, ULONG *pulFlags, void *pdata, ULONG insize, ULONG *poutlen)
{
	Resource *pRes = (Resource *)resource;
	int rc = NO_ERROR;
	char s[4096];

	debuglocal(9,"NdpRsrcQueryInfo in\n");

	switch (pRes->rootlevel)
	{
		case 0:
		{
			ph->fsph_snprintf(s, sizeof(s) - 1, "SMBFS%s \\\\@%s", ifL ? "64" : "32", pRes->srv.username);
		} break;
		case 1:
		{
			ph->fsph_snprintf(s, sizeof(s) - 1, "SMBFS%s %s: \\\\@%s", ifL ? "64" : "32", pRes->srv.workgroup, pRes->srv.username);
		} break;
		case 2:
		{
			ph->fsph_snprintf(s, sizeof(s) - 1, "SMBFS%s \\\\%s%s%s@%s", ifL ? "64" : "32", *pRes->srv.workgroup ? pRes->srv.workgroup : "", *pRes->srv.workgroup ? ":" : "", pRes->srv.server_name, pRes->srv.username);
		} break;
		default:
		{
			ph->fsph_snprintf(s, sizeof(s) - 1, "SMBFS%s \\\\%s%s%s\\%s@%s", ifL ? "64" : "32", *pRes->srv.workgroup ? pRes->srv.workgroup : "", *pRes->srv.workgroup ? ":" : "", pRes->srv.server_name, pRes->srv.share_name, pRes->srv.username);
		} break;
	}
	*poutlen = strlen(s) + 1;
	if (*poutlen > insize)
	{
		rc = ERROR_BUFFER_OVERFLOW;
	}
	else
	{
		memcpy(pdata, s, *poutlen);
	}

	debuglocal(9,"NdpRsrcQueryInfo %d\n", rc);

	return rc;
}

int APIENTRY NdpRsrcQueryFSAttach (HRESOURCE resource, void *pdata, ULONG insize, ULONG *poutlen)
{
	ULONG ulDummy = 0;
	/* just return the resource info string */
	return NdpRsrcQueryInfo (resource, &ulDummy, pdata, insize, poutlen);
}

int APIENTRY NdpRsrcQueryFSAllocate (HRESOURCE resource, NDFSALLOCATE *pfsa)
{
	Resource *pRes = (Resource *)resource;
	int rc = NO_ERROR, rc1;
	unsigned long action = 0;
	cli_state* cli = NULL;
	FSALLOCATE fsa;

	ENTER();
	debuglocal(9,"NdpRsrcQueryFSAllocate %08x\n", pfsa);

	if (!pfsa)
	{
		LEAVE();
		return NO_ERROR;
	}

	rc = smbwrp_connect( pRes, &cli);
	if (rc) 
	{
		debuglocal(9,"smbwrp_connect failed rc=%d\n", rc);	
		pfsa->cSectorUnit = 1;
		pfsa->cUnit = 123456;
		pfsa->cUnitAvail = 123456;
		pfsa->cbSector = 2048;
		rc = (rc == 7 ? ERROR_BAD_DEV_TYPE : ERROR_ACCESS_DENIED);
		LEAVE();
		return rc;
	}

	rc = smbwrp_dskattr( cli, &fsa);
	if (rc)
	{
		pfsa->cSectorUnit = 1;
		pfsa->cUnit = 123456;
		pfsa->cUnitAvail = 123456;
		pfsa->cbSector = 2048;
		//rc = rc ? rc : (resp.rc ? resp.rc : ERROR_INVALID_PARAMETER);
	}
	else
	{
		pfsa->cSectorUnit = fsa.cSectorUnit;
		pfsa->cUnit = fsa.cUnit;
		pfsa->cUnitAvail = fsa.cUnitAvail;
		pfsa->cbSector = fsa.cbSector;
	}

	smbwrp_disconnect( pRes, cli);

	debuglocal(9,"NdpRsrcQueryFSAllocate %d/%d (cUnit = %lu/cUnitAvail = %lu/cbSector = %lu)\n", rc, rc1, pfsa->cUnit, pfsa->cUnitAvail, pfsa->cbSector);
	LEAVE();
	return rc;
}

// -------------------------------------------------------------

int APIENTRY NdpCreateConnection (HRESOURCE resource, HCONNECTION *pconn)
{
	int rc = 0;
	Resource * pRes = (Resource *)resource;
	unsigned long action;
	Connection *pConn = NULL;

	ENTER();

	debuglocal(9,"NdpCreateConnection in\n");

	pConn = malloc( sizeof(Connection));
	if (pConn == NULL)
	{
		rc =  ERROR_NOT_ENOUGH_MEMORY;
	}
	if (rc)
	{
		debuglocal(9,"NdpCreateConnection ERROR_NOT_ENOUGH_MEMORY %d\n", rc);
		LEAVE();
		return rc;
	}
	memset(pConn, 0, sizeof(Connection));
	pConn->pRes = pRes;
	pConn->file.fd = -1;

	debuglocal(9,"NdpCreateConnection send CONNECT\n");
	rc = smbwrp_connect( pRes, &pConn->cli);
	if (rc)
	{
		free(pConn);
		pConn = NULL;
		rc = (rc == 7 ? ERROR_BAD_DEV_TYPE : ERROR_INVALID_PARAMETER);
	}

	*pconn = (HCONNECTION)pConn;
	debuglocal(9,"NdpCreateConnection [%p] %d\n", pConn, rc);
	LEAVE();
	return rc;
}

// -------------------------------------------------------------

int APIENTRY NdpFreeConnection (HCONNECTION conn)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc;

	ENTER();

	debuglocal(9,"NdpFreeConnection in [%p]\n", pConn);
	if (pConn->file.fd >= 0)
	{
		rc = smbwrp_close( pConn->cli, &pConn->file);
		pConn->file.fd = -1;
	}

	smbwrp_disconnect( pRes, pConn->cli);

	free(pConn);
	debuglocal(9,"NdpFreeConnection %d\n", NO_ERROR);
	LEAVE();
	return NO_ERROR;
}

// -------------------------------------------------------------

/* 
 * NdpQueryPathInfo is the most important  function :) netdrive always calls
 * the function before every operation to find out the path status:
 * does it exist, is it a file, does a parent directory exist, etc.
 * Plugin must return one of the following error codes:
 * NO_ERROR - path exists and the path information have been successfully
 * 	retrieved.
 * ERROR_FILE_NOT_FOUND - all but the last component of the path exist and the 
 * 	path without the last component is a directory. dir1_ok\dir2_ok\does_not_exist. 
 * 	the wildcard can not exist, so the plugin returns FILE_NOT_FOUND, if the parent 
 * 	directory exist.
 * ERROR_PATH_NOT_FOUND - any of not last path components does not exist, or all
 * 	but the last component exist and is a file: \dir_ok\dir2_ok\file_ok\non_existing.
 * ERROR_REM_NOT_LIST - resource is temporarily unavailable for some reasons.
 * Any other error codes means an internal plugin error, not related to the status 
 * of the path queried.
 */
int APIENTRY NdpQueryPathInfo (HCONNECTION conn, void *plist, char *szPath)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	smbwrp_fileinfo finfo;
	int rc = 0;
	int rcCon = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

		debuglocal(9,"NdpQueryPathInfo in [%p] <%s>\n", pConn, szPath);
	
		// is wildcard is specified, we suppose parent dir exist, so exit immediately
		if (ph->fsphStrChr(szPath, '*') || ph->fsphStrChr(szPath, '?'))
		{
			LEAVE();
			return ERROR_FILE_NOT_FOUND;
		}


		do {
			/* First check if there is information in the directory cache. */
			unsigned long ulAge = 0;
			if (dircache_find_path(pRes->pdc, szPath, &finfo, &ulAge))
			{
				if (ulAge <= 15) /* @todo configurable. */
				{
					rc = NO_ERROR;
					finfo.easize = -1;
					getfindinfoL(pConn, plist, &finfo, 0, NULL);
					break;
				}
			}

			rc = pathparser(pRes, pConn, szPath, path);
			debuglocal(9,"NdpQueryPathInfo pathparser for <%s> rc=%d\n", path, rc);
			switch (rc)
			{
				case NO_ERROR :
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND:
				case ERROR_ACCESS_DENIED:
				case ERROR_INVALID_PARAMETER:
				{
					break;
				}
				default :
				{	
					rc = ERROR_PATH_NOT_FOUND;
				}
			}
			if (rc)
			{
				break;
			}
			strncpy(finfo.fname, path, sizeof(finfo.fname) - 1);
			debuglocal(9,"NdpQueryPathInfo smbwrp_getattr for <%s>\n", path);
			rc = smbwrp_echo(pConn->cli);
			if (rc) 
			{
				rcCon = smbwrp_connect( pRes, &pConn->cli);
				if (rcCon)
					return ERROR_PATH_NOT_FOUND;
			}
			rc = smbwrp_getattr( &pRes->srv, pConn->cli, &finfo);
			if (rc)
			{
				// remote server not available for first time?
				if (rc == ERROR_REM_NOT_LIST)
				{
					// free current cli resources
					smbwrp_disconnect( pRes, pConn->cli);
					// reconnect
					rcCon = smbwrp_connect( pRes, &pConn->cli);
					if (rcCon != NO_ERROR)
						debuglocal(9,"NdpQueryPathInfo smbwrp_connect rc = %d\n", rcCon);

					// try file list again if reconnecting worked
					if (rcCon == NO_ERROR)
						rc = smbwrp_getattr( &pRes->srv, pConn->cli, &finfo);
				}
				debuglocal(9,"NdpQueryPathInfo smbwrp_getattr, rc = %d\n", rc);
				switch (rc)
				{
					case NO_ERROR :
					case ERROR_FILE_NOT_FOUND:
					case ERROR_PATH_NOT_FOUND:
					case ERROR_ACCESS_DENIED:
					case ERROR_INVALID_PARAMETER:
					case ERROR_REM_NOT_LIST:
					break;
					default :
					{
						rc = ERROR_PATH_NOT_FOUND;
					}
				}
			}
			
			if (rc == NO_ERROR)			{
				finfo.easize = -1;
				getfindinfoL(pConn, plist, &finfo, 0, NULL);
			}
			else if (rc == ERROR_FILE_NOT_FOUND)
			{
				// now try the upper path
				char * p = ph->fsphStrRChr(finfo.fname, '\\');
				if (p && p > finfo.fname)
				{
					*p = 0;
					rc = smbwrp_getattr( &pRes->srv, pConn->cli, &finfo);
					debuglocal(9,"NdpQueryPathInfo upper path in <%s>, rc = %d\n",  finfo.fname, rc);
					if (rc == NO_ERROR)
					{	
						rc = (finfo.attr & FILE_DIRECTORY) !=0 ?
						ERROR_FILE_NOT_FOUND:ERROR_PATH_NOT_FOUND;
					}
					else if (rc != ERROR_REM_NOT_LIST)
					{
						rc = ERROR_PATH_NOT_FOUND;
					}
				}
			}
		} while (0);
		debuglocal(9,"NdpQueryPathInfo <%s> (%s) %d\n", szPath, path, rc);

	LEAVE();
	return rc;
}

// -------------------------------------------------------------

int APIENTRY NdpFindStart (HCONNECTION conn, void *plist, NDFILEINFOL *pfiparent, char *szPath, ULONG ulAttribute)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = NO_ERROR, count = 0;
	unsigned long action;
	char *mask = "*";
	char dir[CCHMAXPATH+1] = {0};
	char path[CCHMAXPATH+1] = {0};
	smbwrp_fileinfo * data;
	NDPATHELEMENT *pel = ph->fsphNameElem(0);
	filelist_state state;
	char * p;

	ENTER();

	debug_printf("NdpFindStart in [%p]\n", pConn);
//	debug_printf("NdpFindStart Debug szPath = %s, dir = %s, path = %s\n", szPath, dir, path);
		strncpy(dir, szPath, sizeof(dir) - 1);
		if (pel)
		{
			mask = pel->name;
			dir[strlen(szPath) - pel->length] = 0;
		}
		action = strlen(dir) - 1;
		if (dir[action] == '\\')
		{
			dir[action] = 0;
		}
//	debug_printf("NdpFindStart Debug2 szPath = %s, dir = %s, path = %s\n", szPath, dir, path);
		rc = pathparser(pRes, pConn, dir, path);
		if (rc)
		{
			return rc;
		}
//	debug_printf("NdpFindStart Debug3 szPath = %s, dir = %s, path = %s\n", szPath, dir, path);
		action = strlen(path) - 1;
		if (path[action] != '\\')
		{
			strncat(path, "\\", sizeof(path) - 1);
		}
//	debug_printf("NdpFindStart Debug4 szPath = %s, dir = %s, path = %s\n", szPath, dir, path);
		strcpy(dir, path);
		strncat(path, mask, sizeof(path) - 1);
//	debug_printf("NdpFindStart Debug5 szPath = %s, dir = %s, path = %s\n", szPath, dir, path);
		// this structure will be used by libsmb callbacks, so we store here all we need 
		// to fill netdrive structures 
		state.pConn = pConn;
		state.plist = plist;
		state.ulAttribute = ulAttribute;
		strcpy( state.dir, dir);
		strcpy( state.dir_mask, mask);
		strcpy( state.mask, path);
		state.fullpath = szPath;
		/* This plugin always reads a complete directory listing and filters results
		 * using actual mask (state.dir_mask) in getfindinfoL.
		 * May be this was a workaround for some server bug.
		 * If this will be changed, then directory listing cache must be changed too,
		 * and must remember the mask, which was used to obtain a listing.
		 * Now the directory cache saves complete directory listings and then uses them to find
		 * information about single files.
		 * However, with a directory cache, it is probably faster to get a full listing and
		 * then use it to obtain info about separate files than to perform a network
		 * list query operation using actual wild cards for each file. Some application,
		 * for example OpenOffice, do this.
		 */
		p = getlastslash(state.mask);
		if (p)
		{
			*(p + 1) = '*';
			*(p + 2) = 0;
		} 
		else
		{
			strcpy(state.mask, "\\*");
		}
		debuglocal(9,"NdpFindStart: dir [%s], dir_mask [%s], mask [%s], szPath [%s]\n",
			state.dir, state.dir_mask, state.mask, state.fullpath);

		/* use cli_echo to check the connection is alive - if not, reconnect */
		rc = smbwrp_echo(pConn->cli);
		if (rc) 
		{
		debuglocal(4,"NdpFindStart, smbwrp_echo rc = %d, reconnecting.....\n",rc);
			int rcCon = smbwrp_connect( pRes, &pConn->cli);
			if (rcCon)
				return ERROR_PATH_NOT_FOUND;
		}
		rc = smbwrp_filelist( &pRes->srv, pConn->cli, &state);
		// we need to handle reconnection also here, because NdpQueryPathInfo 
		// could be called with '*' and exit then immediately (without calling libsmb) 
		if (rc == ERROR_REM_NOT_LIST)
		{
			// free current cli resources
			smbwrp_disconnect( pRes, pConn->cli);
			// reconnect
			smbwrp_connect( pRes, &pConn->cli);
			// try file list again next loop
			rc = smbwrp_filelist( &pRes->srv, pConn->cli, &state);
			debuglocal(9,"NdpFindStart remote connection lost, rc = %d\n", rc);
		}
	debuglocal(9,"NdpFindStart <%s> (%s) cnt %d %d\n", szPath, path, count, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpDeletePathInfo (HRESOURCE resource, NDFILEINFOL *pfi)
{
//	debuglocal(9,"NdpDeletePathInfo %d\n", 0);
	return NO_ERROR;
}

int APIENTRY NdpRefresh (HCONNECTION conn, char *path, int tree)
{
	debuglocal(9,"NdpRefresh <%s> %d\n", path, 0);
	return NO_ERROR;
}

int APIENTRY NdpDiscardResourceData (HRESOURCE resource, NDDATABUF *pdatabuf)
{
	// The plugin do not have to deallocate anything
	// because resource data did not contain any pointers
	// to plugins data.
	// Data stored by fsphSetResourceData will be
	// deallocated by NetDrive.

	debuglocal(9,"NdpDicardresourceData %d\n", 0);
	return NO_ERROR;
}

int APIENTRY NdpSetPathInfo (HCONNECTION conn, NDFILEINFOL *pfi, char *szPathName)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};
	smbwrp_fileinfo finfo;

	ENTER();

	debug_printf("NdpSetPathInfo in [%p]\n", pConn);

	// delete the dir cache
	dircache_invalidate(szPathName, pRes->pdc, 1);

	do {
		rc = pathparser(pRes, pConn, szPathName, path);
		if (rc)
		{
			break;
		}

		memset(&finfo, 0, sizeof(finfo));

		strncpy(finfo.fname, path, sizeof(finfo.fname) - 1);
		fsphDosDateToUnixTime(pfi->stat.fdateLastWrite, pfi->stat.ftimeLastWrite, &(finfo.mtime));
		finfo.attr = pfi->stat.attrFile & 0x37;
		rc = smbwrp_setattr(pConn->cli, &finfo);
	} while (0);
	debuglocal(9,"NdpSetPathInfo <%s> (%s) %d\n", szPathName, path, rc);
	LEAVE();
	return rc;
}

int buildFEALIST(FEALIST *pFEASrc, GEALIST *pGEAList, FEALIST *pFEAList)
{
	int rc = 0;
	FEA * pfea;
	FEA * pfeadest;
	unsigned long size, done = sizeof(pFEAList->cbList), dsize, ddone = sizeof(pFEAList->cbList);

	size = pFEASrc->cbList;
	pfea = pFEASrc->list;
	pfeadest = pFEAList->list;
	dsize = pFEAList->cbList;
//debuglocal(9,"buildFEALIST in destsize %d srcsize %d pGEAList=%08x pGEAList->cbList=%d\n", dsize, ddone, size, pGEAList, pGEAList ? pGEAList->cbList : 0);
	while (done < size)
	{
		char * name = (char *)(pfea + 1);
		int insert = 1;
		if (pGEAList && pGEAList->cbList > sizeof(pGEAList->cbList))
		{
			GEA * pgea = pGEAList->list;
			unsigned long size = pGEAList->cbList - sizeof(pGEAList->cbList), done = 0;
			insert = 0;
			while (done < size)
			{
//debuglocal(9,"comp <%s> <%s>\n", name, pgea->szName);
				if (!ph->fsphStrNCmp(name, pgea->szName, pgea->cbName))
				{
					insert = 1;
					break;
				}
				done += pgea->cbName + 2;
				pgea = (GEA *)((char *)pgea + pgea->cbName + 2);
			}
		}
		if (insert)
		{
			ddone += sizeof(FEA) + pfea->cbName + 1 + pfea->cbValue;
			if (ddone <= dsize)
			{
				pfeadest->cbName = pfea->cbName;
				pfeadest->cbValue = pfea->cbValue;
				pfeadest->fEA = 0;
				strcpy((char *)(pfeadest + 1), name);
				memcpy((char *)(pfeadest + 1) + pfea->cbName + 1, (char *)(pfea + 1) + pfea->cbName + 1, pfea->cbValue);
				pfeadest = (FEA *)((char *)pFEAList + ddone);
			}
		}
		done += sizeof(FEA) + pfea->cbName + 1 + pfea->cbValue;
//debuglocal(9,"buuildfea <%s> insert=%d pfea->cbName=%d pfea->cbValue=%d srcdone=%d destdone=%d pfeadest=%08x pfea=%08x\n", name, insert, pfea->cbName, pfea->cbValue, done, ddone, pfeadest, pfea);
		pfea = (FEA *)((char *)pFEASrc + done);
	}
	pFEAList->cbList = ddone;
	if (ddone > dsize && dsize > sizeof(pFEAList->cbList))
	{
		rc = ERROR_BUFFER_OVERFLOW;
	}
	debuglocal(9,"buildFEALIST rc=%d destsize=%d destdone=%d srcsize=%d pGEAList=%08x\n", rc, dsize, ddone, size, pGEAList);
	return rc;
}

int APIENTRY NdpEAQuery (HCONNECTION conn, GEALIST *pGEAList, NDFILEINFOL *pfi, FEALIST *pFEAList)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char * path = NULL;
	FEALIST * pFEASrc;
	NDDATABUF fdata = {0};
	smbwrp_fileinfo *finfo;
	const int cbBuffer = 64*1024;

	if (!pfi || !pfi->pszName || !pFEAList)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	rc = ph->fsphGetFileInfoData(pfi, &fdata, 0);
	if (rc || !fdata.ulSize || !fdata.pData)
	{
		debuglocal(9,"NdpEAQuery: ph->fsphGetFileInfoData = %d/%d %08x\n", rc, fdata.ulSize, fdata.pData);
		return ERROR_EAS_NOT_SUPPORTED;
	}

	ENTER();

	finfo = (smbwrp_fileinfo *)fdata.pData;
	path = finfo->fname;

	debuglocal(9,"NdpEAQuery in [%p] <%s> %08x %d\n", pConn, path, pGEAList, pGEAList ? pGEAList->cbList : 0);

	char *pchBuffer = (char *)malloc(cbBuffer);
	if (!pchBuffer)
	{
		LEAVE();
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	do {
		rc = smbwrp_listea( pConn->cli, path, pchBuffer, cbBuffer);
		pFEASrc = (FEALIST*) pchBuffer;
		if (rc)
		{
			//rc = pConn->rc ? pConn->rc : (resp.rc ? resp.rc : ERROR_INVALID_PARAMETER);
			switch (rc)
			{
				case ERROR_FILE_NOT_FOUND :
				case ERROR_PATH_NOT_FOUND :
				{
					pFEAList->cbList = sizeof(pFEAList->cbList);
					rc = NO_ERROR;
				} break;
				case ERROR_BUFFER_OVERFLOW :
				{
					pFEAList->cbList = pFEASrc->cbList;
				} break;
				default :
				{
					rc = ERROR_EAS_NOT_SUPPORTED;
				}
			}
		}
		else
		{
			rc = buildFEALIST((FEALIST *)pFEASrc, pGEAList, pFEAList);
		}
	} while (0);
	free(pchBuffer);
	debuglocal(9,"NdpEAQuery <%s> %d %d %d\n", pfi->pszName, rc, pFEASrc->cbList, pFEAList->cbList);
	LEAVE();
	if (rc)
		return ERROR_EAS_NOT_SUPPORTED;
	else
		return rc;
}

int APIENTRY NdpEASet (HCONNECTION conn, FEALIST *pFEAList, NDFILEINFOL *pfi)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	char * path;
	unsigned long action;
	NDDATABUF fdata = {0};
	smbwrp_fileinfo *finfo;

	debuglocal(9,"NdpEASet in [%p]\n", pConn);

	if (!pfi || !pfi->pszName)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	rc = ph->fsphGetFileInfoData(pfi, &fdata, 0);
	if (rc || !fdata.ulSize || !fdata.pData)
	{
		debuglocal(9,"NdpEASet: ph->fsphGetFileInfoData = %d/%d/%08x\n", rc, fdata.ulSize, fdata.pData);
		return ERROR_EAS_NOT_SUPPORTED;
	}

	finfo = (smbwrp_fileinfo *)fdata.pData;
	path = finfo->fname;

	rc = helperEASet(pConn->cli, pFEAList, path);
	debuglocal(9,"NdpEASet %d\n", rc);
	if (rc)
		return ERROR_EAS_NOT_SUPPORTED;
	else
		return rc;
}

int APIENTRY NdpEASize (HCONNECTION conn, NDFILEINFOL *pfi, ULONG *pulEASize)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char * path = NULL;
	FEALIST * pfealist;
	NDDATABUF fdata = {0};
	smbwrp_fileinfo *finfo;
	const int cbBuffer = 64*1024;
	int easize;

	if (!pfi || !pulEASize)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	rc = ph->fsphGetFileInfoData(pfi, &fdata, 0);
	if (rc || !fdata.ulSize || !fdata.pData)
	{
		debuglocal(9,"NdpEASize: ph->fsphGetFileInfoData = %d/%d/%08x\n", rc, fdata.ulSize, fdata.pData);
		return ERROR_EAS_NOT_SUPPORTED;
	}

	ENTER();

	finfo = (smbwrp_fileinfo *)fdata.pData;
	easize = finfo->easize;
	finfo->easize = -1;
	path = finfo->fname;
	if (easize >= 0)
	{
		*pulEASize = easize;
		debuglocal(9,"NdpEASize <%s> cached %d\n", path, easize);
		LEAVE();
		return NO_ERROR;
	}

	debuglocal(9,"NdpEASize in [%p] <%s> \n", pConn, path);

	char *pchBuffer = (char *)malloc(cbBuffer);
	if (!pchBuffer)
	{
		LEAVE();
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	do {
		rc = smbwrp_listea(pConn->cli, path, pchBuffer, cbBuffer);
		pfealist = (FEALIST*)pchBuffer;
		if (rc)
		{
			//rc = pConn->rc ? pConn->rc : (resp.rc ? resp.rc : ERROR_INVALID_PARAMETER);
			switch (rc)
			{
				case ERROR_FILE_NOT_FOUND :
				case ERROR_PATH_NOT_FOUND :
				{
					pfealist->cbList = sizeof(pfealist->cbList);
				} /* Fall through */
				case ERROR_BUFFER_OVERFLOW :
				{
					rc = NO_ERROR;
				} break;
				default :
				{
					rc = ERROR_EAS_NOT_SUPPORTED;
				}
			}
		}
		*pulEASize = pfealist->cbList;
	} while (0);
	free(pchBuffer);
	debuglocal(9,"NdpEASize <%s> %d %d\n", pfi->pszName, *pulEASize, rc);
	LEAVE();
	if (rc)
		return ERROR_EAS_NOT_SUPPORTED;
	else
		return rc;
}

int APIENTRY NdpSetCurrentDir (HCONNECTION conn, NDFILEINFOL *pfi, char *szPath)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};
	
	debuglocal(9,"NdpSetCurrentDir in [%p]\n", pConn);

	ENTER();

	do {
		rc = pathparser(pRes, pConn, szPath, path);
		if (rc)
		{
			break;
		}

		rc = smbwrp_chdir(&pRes->srv, pConn->cli, path);
	} while (0);
	debuglocal(9,"NdpSetCurrentDir <%s> (%s) %d\n", szPath, path, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpCopy (HCONNECTION conn, NDFILEINFOL *pfiDst, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc, ULONG ulOption)
{
	debuglocal(9,"NdpCopy <%s> -> <%s> %d\n", szSrc, szDst, ERROR_CANNOT_COPY);
	return ERROR_CANNOT_COPY;
}

int APIENTRY NdpCopy2 (HCONNECTION conn, HRESOURCE resDst, NDFILEINFOL *pfiDst, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc, ULONG ulOption)
{
	debuglocal(9,"NdpCopy2 <%s> -> <%s> %d\n", szSrc, szDst, ERROR_CANNOT_COPY);
	return ERROR_CANNOT_COPY;
}

int APIENTRY NdpForceDelete (HCONNECTION conn, NDFILEINFOL *pfi, char *szFile)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

	debuglocal(9,"NdpForceDelete in [%p]\n", pConn);

	dircache_invalidate(szFile, pRes->pdc, 1);

	do {
		rc = pathparser(pRes, pConn, szFile, path);
		if (rc)
		{
			break;
		}

		rc = smbwrp_unlink(pConn->cli, path);
	} while (0);
	debuglocal(9,"NdpForceDelete <%s> (%s) %d\n", szFile, path, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpCreateDir (HCONNECTION conn, NDFILEINFOL *pfiparent, char *szDirName, FEALIST *pFEAList)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	int rcEASet = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

	debuglocal(9,"NdpCreateDir in [%p]\n", pConn);

	dircache_invalidate(szDirName, pRes->pdc, 1);

	do {
		rc = pathparser(pRes, pConn, szDirName, path);
		if (rc)
		{
			break;
		}

		rc = smbwrp_mkdir(pConn->cli, path);
	} while (0);
	LEAVE();

	if (path && pRes->easupport)
	{
		rcEASet = helperEASet(pConn->cli, pFEAList, path);
	}
	debuglocal(9,"NdpCreateDir <%s> (%s) rc=%d, EASupport=%s rc=%d\n", szDirName, path, rc, pRes->easupport?"yes":"no", rcEASet);

	if (rc)
		return ERROR_NETWORK_ACCESS_DENIED;
	else
		return rc;
}

int APIENTRY NdpDeleteDir (HCONNECTION conn, NDFILEINFOL *pfi, char *szDir)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

	debuglocal(9,"NdpDeleteDir in [%p]\n", pConn);

	dircache_invalidate(szDir, pRes->pdc, 1);

	do {
		rc = pathparser(pRes, pConn, szDir, path);
		if (rc)
		{
			break;
		}

		rc = smbwrp_rmdir(pConn->cli, path);
	} while (0);
	debuglocal(9,"NdpDeleteDir <%s> (%s) %d\n", szDir, path, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpMove (HCONNECTION conn, NDFILEINFOL *pfiDst, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char src[CCHMAXPATH+1] = {0};
	int l1, l2;
	char * p = szDst;

	ENTER();

	debuglocal(9,"NdpMove in [%p] from <%s> to <%s>\n", pConn, szSrc, szDst);

	dircache_invalidate(szSrc, pRes->pdc, 1);
	dircache_invalidate(szDst, pRes->pdc, 1);

	do
	{
		rc = pathparser(pRes, pConn, szSrc, src);
		if (rc)
		{
			break;
		}
		l1 = strlen(szSrc);
		l2 = strlen(src);
		if (l1 > l2)
		{
			if (ph->fsphStrNICmp(szSrc, szDst, l1 - l2))
			{
				// the file moved accross different shares or servers or workgroups
				rc = ERROR_WRITE_PROTECT;
				break;
			}
			p = szDst + l1 - l2 + 1;
		}
		//pConn->mem[CCHMAXPATH + 1] = '\\';
		rc = smbwrp_rename(pConn->cli, src, p);
	} while (0);
	debuglocal(9,"NdpMove <%s> -> <%s> (%s) %d\n", szSrc, szDst, src, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpMove2 (HCONNECTION conn, HRESOURCE resDst, NDFILEINFOL *pfiDst, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc)
{
	debuglocal(9,"NdpMove2 <%s> -> <%s> %d\n", szSrc, szDst, ERROR_WRITE_PROTECT);
	return ERROR_WRITE_PROTECT;
}


int APIENTRY NdpChangeCase (HCONNECTION conn, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc, char *szNewName, ULONG ulNameLen)
{
	return NdpMove (conn, pfiSrc, szDst, pfiSrc, szSrc);
}

int APIENTRY NdpRename (HCONNECTION conn, char *szDst, NDFILEINFOL *pfiSrc, char *szSrc, char *szNewName, ULONG ulNameLen)
{
	return NdpMove (conn, pfiSrc, szDst, pfiSrc, szSrc);
}

int smbopen(Connection *pConn, char *szFileName, int flags, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList)
{
	Resource *pRes = pConn->pRes;
	unsigned long action;
	int rc = 0;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

	debuglocal(9,"smbopen in [%p] %d\n", pConn, pConn->file.fd);

	if (flags & O_CREAT)
	{
		dircache_invalidate(szFileName, pRes->pdc, 1);
	}
	do {
		if (pConn->file.fd > 0)
		{
			rc = ERROR_TOO_MANY_OPEN_FILES;
			break;
		}

		rc = pathparser(pRes, pConn, szFileName, path);
		if (rc)
		{
			break;
		}

		strncpy(pConn->file.fullname, szFileName, sizeof(pConn->file.fullname) - 1);
		strncpy(pConn->file.fname, path, sizeof(pConn->file.fname) - 1);
		flags |= O_BINARY;
		switch (ulOpenMode & 3)
		{
			case OPEN_ACCESS_READONLY : flags |= O_RDONLY; break;
#if 0
			case OPEN_ACCESS_WRITEONLY : flags |= O_WRONLY; break;
#else
			/*  There is a behavioral change in SMB2 compared to SMB1 
			    When SMB2 opens a file in write only mode SMB2 GetInfo
			    requests fail with NT_STATUS_ACCESS_DENIED
			    Ideally we should use O_WRITEONLY for SMB1 and O_RDWR
			    only for SMB2+
			*/
			case OPEN_ACCESS_WRITEONLY : flags |= O_RDWR; break;
#endif
			case OPEN_ACCESS_READWRITE : flags |= O_RDWR; break;
			default : flags |= O_RDWR;
		}
		pConn->file.openmode = flags;
		pConn->file.openattr = ulAttribute & 0x37;
		pConn->file.denymode = (ulOpenMode & 0x70) >> 4;
		rc = smbwrp_open(pConn->cli, &pConn->file);
	} while (0);
	debuglocal(9,"smbopen <%s> (%s) %08x %08x %08x %d. file = %d\n", szFileName, path, flags, ulOpenMode, ulAttribute, rc, pConn->file.fd);
	if (!rc && pFEAList)
	{
		int rc1 = NdpFileEASet((HCONNECTION)pConn, (NDFILEHANDLE)0, pFEAList);
		debuglocal(9,"smbopen NdpFileEASet %d. pFEAList->cbList %d\n", rc1, pFEAList->cbList);
	}
	LEAVE();
	return rc;
}

int APIENTRY NdpOpenReplace (HCONNECTION conn, NDFILEINFOL *pfi, NDFILEHANDLE *phandle, char *szFileName, ULONG ulSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList)
{
	return smbopen((Connection *)conn, szFileName, O_TRUNC, ulOpenMode, ulAttribute, pFEAList);
}

int APIENTRY NdpOpenReplaceL(HCONNECTION conn, NDFILEINFO *pfi, NDFILEHANDLE *phandle, char *szFileName, LONGLONG llSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList)
{
	return smbopen((Connection *)conn, szFileName, O_TRUNC, ulOpenMode, ulAttribute, pFEAList);
}

int APIENTRY NdpOpenCreate (HCONNECTION conn, NDFILEINFOL *pfiparent, NDFILEHANDLE *phandle, char *szFileName, ULONG ulSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList)
{
//	return smbopen((Connection *)conn, szFileName, O_CREAT, ulOpenMode, ulAttribute);
	return smbopen((Connection *)conn, szFileName, O_CREAT | O_EXCL, ulOpenMode, ulAttribute, pFEAList);
}

int APIENTRY NdpOpenCreateL(HCONNECTION conn, NDFILEINFO *pfiparent, NDFILEHANDLE *phandle, char *szFileName, LONGLONG llSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList)
{
	return smbopen((Connection *)conn, szFileName, O_CREAT | O_EXCL, ulOpenMode, ulAttribute, pFEAList);
}

int APIENTRY NdpOpenExisting (HCONNECTION conn, NDFILEINFOL *pfi, NDFILEHANDLE *phandle, char *szFileName, ULONG ulOpenMode, USHORT *pfNeedEA)
{
	if (pfNeedEA) *pfNeedEA = 0; // wtf is this ?
	return smbopen((Connection *)conn, szFileName, 0, ulOpenMode, 0, NULL);
}

int APIENTRY NdpSetFileAttribute (HCONNECTION conn, NDFILEINFOL *pfi, char *szFileName, USHORT usAttr)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;

	smbwrp_fileinfo finfo;
	char path[CCHMAXPATH+1] = {0};

	ENTER();

	debuglocal(9,"NdpSetFileAttribute in [%p]\n", pConn);
	do {
		rc = pathparser(pRes, pConn, szFileName, path);
		if (rc)
		{
			break;
		}

		memset(&finfo, 0, sizeof(finfo));
		strncpy(finfo.fname, path, sizeof(finfo.fname) - 1);
		finfo.attr = usAttr & 0x37;
		rc = smbwrp_setattr(pConn->cli, &finfo);
	} while (0);
	debuglocal(9,"NdpSetFileAttribute <%s> (%s) %04x %d\n", szFileName, path, usAttr, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFlush (HRESOURCE resource)
{
	debuglocal(9,"NdpFlush %d\n", ERROR_NOT_SUPPORTED);
	return ERROR_NOT_SUPPORTED;
}

// Called when a new thread will call the plugin. Allows to do per thread init, like disable signals.
#define ND_PL_INIT_THREAD 0xFFFF0000

int APIENTRY NdpIOCTL (int type, HRESOURCE resource, char *path, int function, void *in, ULONG insize, PULONG poutlen)
{
	if (function == ND_PL_INIT_THREAD)
	{
		smbwrp_initthread();
		debuglocal(9, "NdpIOCTL init thread\n");
		return NO_ERROR;
	}

	debuglocal(9,"NdpIOCTL <%s> %d\n", path, function);

	if (in && insize > 4096)
	{
		char out[4096];
		sprintf (out, "SAMBA IOCTL function = %d, parms [%s] insize = %d, *poutlen = %d", function, in, insize, *poutlen);
		*poutlen = strlen(out);
		strcpy (in, out);
		return NO_ERROR;
	}

	return ERROR_NOT_SUPPORTED;
}

int APIENTRY NdpFileQueryInfo (HCONNECTION conn, NDFILEHANDLE handle, void *plist)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	smbwrp_fileinfo finfo;

	ENTER();

	debug_printf("NdpFileQueryInfo in [%p]\n", pConn);
	do {
		if (pConn->file.fd < 0 || !*pConn->file.fname)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}
		strncpy(finfo.fname, pConn->file.fname, sizeof(finfo.fname) - 1);
		rc = smbwrp_fgetattr(pConn->cli, &pConn->file, &finfo);
		if (!rc)
		{
			finfo.easize = -1;
			getfindinfoL(pConn, plist, &finfo, 0, NULL);
		}
	} while (0);
	debuglocal(9,"NdpFileQueryInfo <%s> %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileEAQuery (HCONNECTION conn, NDFILEHANDLE handle, GEALIST *pGEAList, FEALIST *pFEAList)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	const int cbBuffer = 64*1024;
	FEALIST * pFEASrc;

	if (!pFEAList)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	debuglocal(9,"NdpFileEAQuery in [%p] <%s>/%d pGEAList=%08x\n", pConn, pConn->file.fname, pConn->file.fd, pGEAList);

	char *pchBuffer = (char *)malloc(cbBuffer);
	if (!pchBuffer)
		return ERROR_NOT_ENOUGH_MEMORY;

	ENTER();

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}
		rc = smbwrp_flistea(pConn->cli, &pConn->file, pchBuffer, cbBuffer);
		pFEASrc = (FEALIST *)pchBuffer;
		if (rc)
		{
			//rc = pConn->rc ? pConn->rc : (resp.rc ? resp.rc : ERROR_INVALID_PARAMETER);
			switch (rc)
			{
				case ERROR_FILE_NOT_FOUND :
				case ERROR_PATH_NOT_FOUND :
				{
					pFEAList->cbList = sizeof(pFEAList->cbList);
					rc = NO_ERROR;
				} break;
				case ERROR_BUFFER_OVERFLOW :
				{
					pFEAList->cbList = pFEASrc->cbList;
				} break;
				default :
				{
					rc = ERROR_EAS_NOT_SUPPORTED;
				}
			}
		}
		else
		{
			rc = buildFEALIST(pFEASrc, pGEAList, pFEAList);
		}
	} while (0);
	free(pchBuffer);
	debuglocal(9,"NdpFileEAQuery out <%s>/%d pFEASrc->cbList=%d pFEAList->cbList=%d rc=%d\n", pConn->file.fname, pConn->file.fd, pFEASrc->cbList, pFEAList->cbList, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileEASet (HCONNECTION conn, NDFILEHANDLE handle, FEALIST *pFEAList)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;

	debuglocal(9,"NdpFileEASet in [%p]\n", pConn);

	if (!pFEAList || pFEAList->cbList <= sizeof(long))
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	ENTER();

	do {
		// got FEA there
		FEA * pfea;
		unsigned long done = sizeof(long);
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}

		pfea = pFEAList->list;
		while (done < pFEAList->cbList)
		{
			rc = smbwrp_fsetea(pConn->cli, &pConn->file, (char *)(pfea + 1), pfea->cbValue ? (char *)(pfea + 1) + pfea->cbName + 1: NULL, pfea->cbValue);
			if (rc)
			{
				break;
			}
			pfea = (FEA *)((char *)(pfea + 1) + pfea->cbName + 1 + pfea->cbValue);
			done += sizeof(FEA) + pfea->cbName + 1 + pfea->cbValue;
		}

	} while (0);
	debuglocal(9,"NdpFileEASet %d\n", rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileEASize (HCONNECTION conn, NDFILEHANDLE handle, ULONG *pulEASize)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;
	char path[CCHMAXPATH+1] = {0};
	FEALIST * pFEAList;
	const int cbBuffer = 64*1024;

	if (!pulEASize)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}
	if (!pRes->easupport)
	{
		return ERROR_EAS_NOT_SUPPORTED;
	}

	debuglocal(9,"NdpFileEASize in [%p] <%s>/%d \n", pConn, pConn->file.fname, pConn->file.fd);

	char *pchBuffer = (char *)malloc(cbBuffer);
	if (!pchBuffer)
		return ERROR_NOT_ENOUGH_MEMORY;

	ENTER();

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}
		rc = smbwrp_flistea(pConn->cli, &pConn->file, pchBuffer, cbBuffer);
		pFEAList = (FEALIST*)pchBuffer;
		if (rc)
		{
			//rc = pConn->rc ? pConn->rc : (resp.rc ? resp.rc : ERROR_INVALID_PARAMETER);
			switch (rc)
			{
				case ERROR_FILE_NOT_FOUND :
				case ERROR_PATH_NOT_FOUND :
				{
					pFEAList->cbList = sizeof(pFEAList->cbList);
				} /* Fall through */
				case ERROR_BUFFER_OVERFLOW :
				{
					rc = NO_ERROR;
				} break;
				default :
				{
					rc = ERROR_EAS_NOT_SUPPORTED;
				}
			}
		}
		*pulEASize = pFEAList->cbList;
	} while (0);
	free(pchBuffer);
	debuglocal(9,"NdpFileEASize %d %d\n", *pulEASize, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileSetInfo (HCONNECTION conn, NDFILEHANDLE handle, NDFILEINFOL *pfi)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action, attrFile;

	ENTER();

	debug_printf("NdpFileSetInfo in [%p]\n", pConn);

	// delete the dir cache
	dircache_invalidate(pConn->file.fullname, pRes->pdc, 1);

	do {
		if (pConn->file.fd < 0 || !*pConn->file.fname)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}
		attrFile = pfi->stat.attrFile;
		// deferred setinfo - on closing the file
		pConn->file.openattr = attrFile;
		fsphDosDateToUnixTime(pfi->stat.fdateLastWrite, pfi->stat.ftimeLastWrite, &(pConn->file.mtime));
//		fsphDosDateToUnixTime(pfi->stat.fdateCreation, pfi->stat.ftimeCreation, &(pConn->file.btime));
		pConn->file.updatetime = 2;
		debug_printf("NdpFileSetInfo mtime %d\n", pConn->file.mtime);
	} while (0);
	debuglocal(9,"NdpFileSetInfo <%s> %08x %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, attrFile, rc);
	LEAVE();
	return NO_ERROR;
}

int APIENTRY NdpFileSetFilePtrL(HCONNECTION conn, NDFILEHANDLE handle, LONGLONG llOffset, ULONG ulMethod, LONGLONG *pllActual)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;

	ENTER();

	debuglocal(9,"NdpFileSetFilePtrL in [%p]\n", pConn);

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}

		rc = smbwrp_lseek(pConn->cli, &pConn->file, ulMethod, llOffset);
		if (!rc)
			*pllActual = pConn->file.offset;

	} while (0);
	debuglocal(9,"NdpFileSetFilePtrL <%s> %lld %lu %lld %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, llOffset, ulMethod, *pllActual, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileSetFilePtr (HCONNECTION conn, NDFILEHANDLE handle, LONG lOffset, ULONG ulMethod, ULONG *pulActual)
{
	LONGLONG llActual;
	int rc = NdpFileSetFilePtrL(conn, handle, lOffset, ulMethod, &llActual);
	*pulActual = llActual & 0xFFFFFFFF;
	debuglocal(9,"NdpFileSetFilePtr %ld %lu %ld %d\n", lOffset, ulMethod, *pulActual, rc);
	return rc;
}

int APIENTRY NdpFileClose (HCONNECTION conn, NDFILEHANDLE handle)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;

	ENTER();

	debuglocal(9,"NdpFileClose in [%p] %d <%s>\n", pConn, pConn->file.fd, pConn->file.fd < 0 ? "!null!" : pConn->file.fname);

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}

		rc = smbwrp_close(pConn->cli, &pConn->file);

	} while (0);
	debuglocal(9,"NdpFileClose %d %d\n", pConn->file.fd, rc);

	pConn->file.fd = -1;
	LEAVE();
	return rc;
}

int APIENTRY NdpFileCommit (HCONNECTION conn, NDFILEHANDLE handle)
{
	debuglocal(9,"NdpFileCommit %d\n", NO_ERROR);
	return NO_ERROR;
}


int APIENTRY NdpFileNewSize (HCONNECTION conn, NDFILEHANDLE handle, ULONG ulLen)
{
	int rc = NdpFileNewSizeL(conn, handle, ulLen);
	debuglocal(9,"NdpFileNewSize %ld %d\n", ulLen, rc);
	return rc;
}

int APIENTRY NdpFileNewSizeL(HCONNECTION conn, NDFILEHANDLE handle, LONGLONG llLen)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long action;

	ENTER();

	debuglocal(9,"NdpFileNewSizeL in [%p]\n", pConn);

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}

		rc = smbwrp_setfilesize(pConn->cli, &pConn->file, llLen); 

	} while (0);
	debuglocal(9,"NdpFileNewSizeL <%s> %lld %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, llLen, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileRead (HCONNECTION conn, NDFILEHANDLE handle, void *pBuffer, ULONG ulRead, ULONG *pulActual)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;

	ENTER();

	debuglocal(9,"NdpFileRead in [%p], ulRead = %d\n", pConn, ulRead);

	ULONG ulActual;
	rc = smbwrp_read(pConn->cli, &pConn->file, (char *)pBuffer, ulRead, &ulActual);

	if (ulActual > 0)
	{
		rc = NO_ERROR; /* Still were able to read some data. */
	}

	if (rc == NO_ERROR)
	{
		*pulActual = ulActual;
	}

	debuglocal(9,"NdpFileRead <%s> %lu %lu %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, ulRead, *pulActual, rc);
	LEAVE();
	return rc;
}

int APIENTRY NdpFileWrite (HCONNECTION conn, NDFILEHANDLE handle, void *pBuffer, ULONG ulWrite, ULONG *pulActual)
{
	Connection *pConn = (Connection *)conn;
	Resource *pRes = pConn->pRes;
	int rc = 0;
	unsigned long done = 0;
	unsigned long onedone;
	unsigned long action;

	ENTER();

	debuglocal(9,"NdpFileWrite in [%p]\n", pConn);

	/* delete the dir cache
	this was moved from NdpFileClose() becasue if there are a lot files in the tree all are reread
	the problem when moved to here is, that last accessed time is not refreshed
	if this is needed, a new function needs to be done to update only one file in the cache */
	dircache_invalidate(pConn->file.fullname, pRes->pdc, 1);

	do {
		if (pConn->file.fd < 0)
		{
			rc = ERROR_INVALID_HANDLE;
			break;
		}
		rc = smbwrp_write(pConn->cli, &pConn->file, pBuffer, ulWrite, pulActual);

	} while (0);
	debuglocal(9,"NdpFileWrite <%s> %lu %lu %d\n", pConn->file.fd < 0 ? "!null!" : pConn->file.fname, ulWrite, *pulActual, rc);
	LEAVE();
	return rc;
}

