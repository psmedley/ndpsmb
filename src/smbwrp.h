/* 
   Netdrive Samba client plugin
   samba library wrappers
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

#ifndef _SMBWRP_H
#define _SMBWRP_H

#include <sys/types.h>
#if !defined (O_RDONLY)
#define O_ACCMODE       0x03    /* mask */
#define O_RDONLY        0x00
#define O_WRONLY        0x01
#define O_RDWR          0x02
#define O_NONBLOCK      0x04
#define O_APPEND        0x08
#define O_TEXT          0x10

#define O_BINARY        0x0100
#define O_CREAT         0x0200
#define O_TRUNC         0x0400
#define O_EXCL          0x0800
#define O_SYNC          0x2000
#define O_NOCTTY        0x4000
#define O_SIZE          0x8000
#endif

#if !defined (DENY_NONE)
/* deny modes */
#define DENY_DOS 0
#define DENY_ALL 1
#define DENY_WRITE 2
#define DENY_READ 3
#define DENY_NONE 4
#define DENY_FCB 7
#endif


/* these define the attribute byte as seen by DOS */
#ifndef aRONLY
#define aRONLY (1L<<0)		/* 0x01 */
#define aHIDDEN (1L<<1)		/* 0x02 */
#define aSYSTEM (1L<<2)		/* 0x04 */
#define aVOLID (1L<<3)		/* 0x08 */
#define aDIR (1L<<4)		/* 0x10 */
#define aARCH (1L<<5)		/* 0x20 */
#endif

#pragma pack(1)

#ifdef CLI_BUFFER_SIZE
typedef	struct cli_state cli_state;
#else
typedef	unsigned long cli_state;
#endif

#define CAP_NOPATHINFO2 0x01000000

typedef struct smbwrp_server 
{
	char server_name[256];
	char share_name[256];
	char workgroup[256];
	char username[256];
	char password[256];
	char master[256];
	int ifmastergroup;
	int no_pathinfo2;
} smbwrp_server;

typedef struct smbwrp_file 
{
	int fd;
	unsigned long long offset;
	int openmode;
	int openattr;
	int denymode;
	unsigned long btime;
	unsigned long mtime;
	unsigned long atime;
	unsigned long ctime;
	int updatetime; 
	// 0 = time is not updated upon file close
	// 1 = modified time is updated to current time
	// 2 = create and modified time are updated according local file
	char fullname[261];
	char fname[261];
} smbwrp_file;

typedef struct smbwrp_fileinfo
{
	unsigned long long size;
	unsigned long attr;
	unsigned long btime;
	unsigned long mtime;
	unsigned long atime;
	unsigned long ctime;
	int easize;
	char fname[261];
} smbwrp_fileinfo;

typedef struct smbwrp_fileseek
{
	int whence;
	long offset;
	unsigned long result;
} smbwrp_fileseek;


#ifndef INCL_DOS
typedef struct _FSALLOCATE      /* fsalloc */
{
	unsigned long  idFileSystem;
	unsigned long  cSectorUnit;
	unsigned long  cUnit;
	unsigned long  cUnitAvail;
	unsigned short cbSector;
} FSALLOCATE;
#endif

struct DirectoryCache;

typedef struct _Resource
{
	int rootlevel;
	smbwrp_server srv;
	char logfile[_MAX_PATH + 1];
	char loglevel;
	int easupport;
	int krb5support;
	int ntlmv1support;
	int encryptionsupport;
        int cachetimeout;
        int cachedepth;
	struct DirectoryCache *pdc;
} Resource;

typedef struct _Connection
{
	Resource *pRes;
	cli_state* cli;
	smbwrp_file file;
} Connection;


typedef struct filelist_state
{
	unsigned long pipe;
	char * data;
	int datalen;
	int bufferlen;
	void ( * _System add_dir_entry)(void * st);
	char mask[ _MAX_PATH];
	char dir[ _MAX_PATH];
	char dir_mask[ _MAX_PATH];
	smbwrp_fileinfo finfo;
	Connection* pConn;
	void *plist;
	unsigned long ulAttribute;
	const char *fullpath;
} filelist_state;

#pragma pack()

int _System smbwrp_getclisize(void);
int _System smbwrp_init(void);
int _System smbwrp_connect(Resource * pRes, cli_state **);
void _System smbwrp_disconnect(Resource * pRes, cli_state *);
int _System smbwrp_open(cli_state * cli, smbwrp_file * file);
int _System smbwrp_read(cli_state * cli, smbwrp_file * file, void *buf, unsigned long count, unsigned long * result);
int _System smbwrp_write(cli_state * cli, smbwrp_file * file, void *buf, unsigned long count, unsigned long * result);
int _System smbwrp_lseek(cli_state * cli, smbwrp_file * file, int whence, long long offset);
int _System smbwrp_close(cli_state * cli, smbwrp_file * file);
int _System smbwrp_setattr(cli_state * cli, smbwrp_fileinfo *finfo);
int _System smbwrp_getattr(smbwrp_server *srv, cli_state * cli, smbwrp_fileinfo *finfo);
int _System smbwrp_fgetattr(cli_state * cli, smbwrp_file *file, smbwrp_fileinfo *finfo);
int _System smbwrp_filelist(smbwrp_server *srv, cli_state * cli, filelist_state * state);
int _System smbwrp_rename(cli_state * cli, char *oldname, char *newname);
int _System smbwrp_chdir(smbwrp_server *srv, cli_state * cli, char *fname);
int _System smbwrp_mkdir(cli_state * cli, char *fname);
int _System smbwrp_rmdir(cli_state * cli, char *fname);
int _System smbwrp_unlink(cli_state * cli, const char *fname);
int _System smbwrp_setfilesize(cli_state * cli, smbwrp_file * file, long long newsize);
int _System smbwrp_setea(cli_state * cli, char *fname, char * name, unsigned char * value, int size);
int _System smbwrp_fsetea(cli_state * cli, smbwrp_file *file, char * name, unsigned char * value, int size);
int _System smbwrp_listea(cli_state * cli, char *fname, void * buffer, unsigned long size);
int _System smbwrp_flistea(cli_state * cli, smbwrp_file *file, void * buffer, unsigned long size);
int _System smbwrp_dskattr(cli_state * cli, FSALLOCATE *pfsa);
int _System smbwrp_echo(cli_state * cli);

/* Directory cache helpers. */
int dircache_create(struct DirectoryCache **ppdc, unsigned long ulExpirationTime, int cMaxEntries);
void dircache_delete(struct DirectoryCache *pdc);

typedef uint32_t NTSTATUS;
typedef NTSTATUS FNADDDIRENTRY(const char*, smbwrp_fileinfo *, const char *, void *);
typedef FNADDDIRENTRY *PFNADDDIRENTRY;

/* Note: dircache_list_files or dircache_write_begin construct the directory path
 *       using information in the filelist_state structure.
 */
int dircache_list_files(PFNADDDIRENTRY fn,
                        filelist_state *state,
                        int *ptotal_received);

void *dircache_write_begin(filelist_state *state,
                           int cFiles);
void dircache_write_entry(void *dircachectx, const smbwrp_fileinfo *finfo);
void dircache_write_end(void *dircachectx);

void dircache_invalidate(const char *path,
                         struct DirectoryCache *pdc,
                         int fParent);

int dircache_find_path(struct DirectoryCache *pdc,
                       const char *path,
                       smbwrp_fileinfo *finfo,
                       unsigned long *pulAge);

/* Prototype the debug log helper. */
void debuglocal(int level, const char * fmt, ...);

void smbwrp_initthread(void);

#endif /* _SMBWRP_H */
