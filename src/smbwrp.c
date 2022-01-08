/* 
   Netdrive Samba client plugin
   samba library wrappers
   Copyright (C) netlabs.org 2003-2019

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

#include "includes.h"
#include "rpc_client/cli_pipe.h"
#include "librpc/gen_ndr/ndr_srvsvc_c.h"
#include "libsmb/libsmb.h"
#include "libsmb/clirap.h"
#include "lib/util/tevent_ntstatus.h"
#include "param.h"
#include "smb/smbXcli_base.h"
#include "trans2.h"
#include "smbwrp.h"
#include "util.h"
#include <math.h>
bool writeLog(); //defined in debug.c
int getfindinfoL(Connection * pConn, void * plist, smbwrp_fileinfo * finfo, u_long ulAttribute, char * mask); // defined in ndpsmb.c
struct smb2_hnd {
	uint64_t fid_persistent;
	uint64_t fid_volatile;
};

bool windows_parent_dirname(TALLOC_CTX *mem_ctx,
				const char *dir,
				char **parent,
				const char **name);
NTSTATUS map_fnum_to_smb2_handle(struct cli_state *cli,
				uint16_t fnum,		/* In */
				struct smb2_hnd **pph);	/* Out */

NTSTATUS parse_finfo_id_both_directory_info(uint8_t *dir_data,
				uint32_t dir_data_length,
				struct file_info *finfo,
				uint32_t *next_offset);
NTSTATUS net_share_enum_rpc(struct cli_state *cli,
                   void (*fn)(const char *name,
                              uint32_t type,
                              const char *comment,
                              void *state),
                   void *state);

/*
 * Wrapper for cli_errno to return not connected error on negative fd
 * Now returns an OS/2 return code instead of lerrno.
 */
int os2cli_errno(cli_state * cli)
{
#if 0 // cli->fd not available in Samba 4.x
	if (cli->fd == -1)
	{
		return maperror( ENOTCONN);
	}
#endif
	return maperror(cli_errno(cli));
}

void smbwrp_Logging()
{
	char slogfile[_MAX_PATH +1] = {0};
	char slogfilename[] = "log.smbc";
	char *env = getenv("LOGFILES");
	if (env != NULL)
	{
		strncpy(slogfile, env, sizeof(slogfile) -1);
		strncat(slogfile, "\\", sizeof(slogfile) - strlen(slogfile) -1);
		strncat(slogfile, slogfilename, sizeof(slogfile) - strlen(slogfile) -1);
	}
	else
	{
		strncpy(slogfile, slogfilename, sizeof(slogfile) -1);
	}

	// init samba for debug messages
	setup_logging(slogfile, DEBUG_FILE);
	lp_set_logfile(slogfile);
	reopen_logs();
}

const char * smbwrp_getVersion()
{
	return samba_version_string();
}

int _System smbwrp_getclisize(void)
{
	return sizeof(struct cli_state);
}

/***************************************************** 
initialise structures
*******************************************************/
int _System smbwrp_init(void)
{
	static int initialised = 0;
	char *p;

	struct loadparm_context *lp_ctx;
	TALLOC_CTX *frame = talloc_stackframe();

	if (initialised) 
	{
		return 0;
	}
	initialised = 1;

	smb_init_locale();

	if (!lp_load_client(get_dyn_CONFIGFILE())) {
		debuglocal(0,("Can't load %s, defaults are used!\n",get_dyn_CONFIGFILE()));
	}
	load_interfaces();

	if (!init_names())
	{
		return 1;
	}

	if (writeLog())
	{
		smbwrp_Logging();
	}

/*
	if ((p=smbw_getshared("RESOLVE_ORDER"))) {
		lp_set_name_resolve_order(p);
	}
*/
	return 0;

}

void smbwrp_initthread(void)
{
	/*
	Block SIGPIPE (from lib/util_sock.c: write())
	It is not needed and should not stop execution
	*/
	BlockSignals(True, SIGPIPE);
}

#if 0
/***************************************************** 
remove redundent stuff from a filename
*******************************************************/
void clean_fname(char *name)
{
	char *p, *p2;
	int l;
	int modified = 1;

	if (!name) return;

	while (modified) {
		modified = 0;

		if ((p=strstr(name,"/./"))) {
			modified = 1;
			while (*p) {
				p[0] = p[2];
				p++;
			}
		}

		if ((p=strstr(name,"//"))) {
			modified = 1;
			while (*p) {
				p[0] = p[1];
				p++;
			}
		}

		if (strcmp(name,"/../")==0) {
			modified = 1;
			name[1] = 0;
		}

		if ((p=strstr(name,"/../"))) {
			modified = 1;
			for (p2=(p>name?p-1:p);p2>name;p2--) {
				if (p2[0] == '/') break;
			}
			while (*p2) {
				p2[0] = p2[3];
				p2++;
			}
		}

		if (strcmp(name,"/..")==0) {
			modified = 1;
			name[1] = 0;
		}

		l = strlen(name);
		p = l>=3?(name+l-3):name;
		if (strcmp(p,"/..")==0) {
			modified = 1;
			for (p2=p-1;p2>name;p2--) {
				if (p2[0] == '/') break;
			}
			if (p2==name) {
				p[0] = '/';
				p[1] = 0;
			} else {
				p2[0] = 0;
			}
		}

		l = strlen(name);
		p = l>=2?(name+l-2):name;
		if (strcmp(p,"/.")==0) {
			if (p == name) {
				p[1] = 0;
			} else {
				p[0] = 0;
			}
		}

		if (strncmp(p=name,"./",2) == 0) {      
			modified = 1;
			do {
				p[0] = p[2];
			} while (*p++);
		}

		l = strlen(p=name);
		if (l > 1 && p[l-1] == '/') {
			modified = 1;
			p[l-1] = 0;
		}
	}
}
#endif

/***************************************************** 
return a connection to a server
loosely based on do_connect() from libsmb/clidfs.c
*******************************************************/
int _System smbwrp_connect( Resource* pRes, cli_state ** cli)
{
    	smbwrp_server * srv = &pRes->srv;
	char * server = srv->server_name;
	char * share = *(srv->share_name) ? srv->share_name : "IPC$";
	char * workgroup = NULL;
	struct nmb_name called, calling;
	char *p, *server_n = server;
	struct cli_state * c = NULL;
	char* dev_type;
	int loginerror = 0;
	NTSTATUS status;
	int max_protocol = lp__client_max_protocol();
	int port = 0; //NBT_SMB_PORT;
	int name_type= 0x20;
	int flags = 0;
	int signing_state = SMB_SIGNING_DEFAULT;
	enum protocol_types protocol = PROTOCOL_NONE;
	const char *name = NULL;
	struct cli_credentials *creds = NULL;
	TALLOC_CTX *ctx = talloc_tos();


	if (strlen(srv->workgroup)>0) {
		workgroup = srv->workgroup;
	}

	if (pRes->krb5support) {
		flags |= CLI_FULL_CONNECTION_USE_KERBEROS;
		debuglocal(1,"Connecting to \\\\%s:%s\\%s using kerberos authentication. Master %s:%d\n", workgroup, server, share, srv->master, srv->ifmastergroup);
	} else {
		debuglocal(1,"Connecting to \\\\%s:*********@%s:%s\\%s. Master %s:%d\n", srv->username,  workgroup, server, share, srv->master, srv->ifmastergroup);
	}

	if (pRes->ntlmv1support) {
		lp_set_cmdline("client ntlmv2 auth","no");
	}

	if (pRes->encryptionsupport) {
		signing_state = SMB_SIGNING_REQUIRED;
	}

	status = cli_connect_nb(
		server, NULL, port, name_type, NULL,
		signing_state, flags, &c);

	if (!NT_STATUS_IS_OK(status)) {
		debuglocal(4,"Connection to %s failed (Error %s)\n",
				server,
				nt_errstr(status));
		return 3;
	}

	if (max_protocol == PROTOCOL_DEFAULT) {
		max_protocol = PROTOCOL_LATEST;
	}
	DEBUG(4,(" session request ok, c->timeout = %d\n",c->timeout));

	status = smbXcli_negprot(c->conn, c->timeout,
				 lp_client_min_protocol(),
				 max_protocol);

	if (!NT_STATUS_IS_OK(status)) {
		debuglocal(4,"protocol negotiation failed: %s\n",
			 nt_errstr(status));
		cli_shutdown(c);
		return status;
	}

	protocol = smbXcli_conn_protocol(c->conn);
	debuglocal(4,(" negotiated dialect[%s] against server[%s]\n",
		 smb_protocol_types_string(protocol),
		 smbXcli_conn_remote_name(c->conn)));

	if (smbXcli_conn_protocol(c->conn) >= PROTOCOL_SMB2_02) {
		/* Ensure we ask for some initial credits. */
		smb2cli_conn_set_max_credits(c->conn, DEFAULT_SMB2_MAX_CREDITS);
	}

	creds = cli_session_creds_init(ctx,
				       srv->username,
				       workgroup,
				       NULL, /* realm */
				       srv->password,
				       pRes->krb5support, /* use_kerberos */
				       pRes->krb5support, /* fallback_after_kerberos */ /* 2020-09-18 was false */
				       false, /* use_ccache */
				       false); /* password_is_nt_hash */

	status = cli_session_setup_creds(c, creds);
	if (!NT_STATUS_IS_OK(status)) {
		loginerror = 1; // save the login error

		/* If a password was not supplied then
		 * try again with a null username. */
		if (srv->password[0] || !srv->username[0] ||
			pRes->krb5support ||
			cli_credentials_authentication_requested(creds) ||
			cli_credentials_is_anonymous(creds) ||
		!NT_STATUS_IS_OK(status = cli_session_setup_anon(c))) {
			debuglocal(1,"session setup failed: %s\n",
				 nt_errstr(status));
			if (NT_STATUS_EQUAL(status,
					    NT_STATUS_MORE_PROCESSING_REQUIRED))
				debuglocal(4,"did you forget to run kinit?\n");
			cli_shutdown(c);
			return 6;
		}
		debuglocal(4,"Anonymous login successful\n");
	}
	TALLOC_FREE(creds);
	if (!NT_STATUS_IS_OK(status)) {
		debuglocal(4,"cli_init_creds() failed\n");
		cli_shutdown(c);
		// if loginerror is != 0 means normal login failed, but anonymous login worked
		if (loginerror !=0)
			return 6;
		else
			return 7;
	}

	debuglocal(4," session setup ok. Sending tconx <%s> <********>\n", share);

#if 0 /* Causes connections to fail with Samba 4.x */
	// YD ticket:58 we need to check resource type to avoid connecting to printers.
	// dev type is set to IPC for IPC$, A: for everything else (printers use LPT1:)
	if (!strcmp( share, "IPC$"))
	    dev_type = "IPC";
	else
	    dev_type = "A:";

	if (!NT_STATUS_IS_OK(cli_tcon_andx(c, share, dev_type,
			    srv->password, strlen(srv->password)+1))) {
		cli_shutdown(c);
		// if loginerror is != 0 means normal login failed, but anonymous login worked
		if (loginerror !=0)
			return 6;
		else
			return 7;
	}
#endif

	/* must be a normal share */
 
	status = cli_tree_connect_creds(c, share, "?????", creds);

	if (!NT_STATUS_IS_OK(status)) {
		debuglocal(4,"tree connect failed: %s\n", nt_errstr(status));
		cli_shutdown(c);
		return status;
	}

	if (pRes->encryptionsupport) {
	debuglocal(4,"Attempting to force encryption\n");
		status = cli_cm_force_encryption_creds(c,
						       creds,
						       share);
		if (!NT_STATUS_IS_OK(status)) {
			cli_shutdown(c);
			return status;
		}
	}

	debuglocal(4," tconx ok.\n");
	
	// save cli_state pointer
	*cli = c;

	return 0;
}

/***************************************************** 
close a connection to a server
*******************************************************/
void _System smbwrp_disconnect( Resource* pRes, cli_state * cli)
{
	if (pRes && cli)
	{
		// this call will free all buffers, close handles and free cli mem
		cli_shutdown( cli);
	}
}



/***************************************************** 
a wrapper for open()
*******************************************************/
int _System smbwrp_open(cli_state * cli, smbwrp_file * file)
{
	uint16_t fd = 0;

	if (!cli || !file || !*file->fname) 
	{
		return maperror(EINVAL);
	}
	if (file->denymode < DENY_ALL || file->denymode > DENY_NONE) 
	{
		file->denymode = DENY_NONE;
	}

	debuglocal(4,"cli_open(%s) attr %08x mode %02x denymode %02x\n", file->fname, file->openattr, file->openmode, file->denymode);

	if (!NT_STATUS_IS_OK(cli_open(cli, file->fname, file->openmode, file->denymode, &fd)))
	{	
		debuglocal(4,"cli_open failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	file->fd = fd;
	file->updatetime = 0;
	file->offset = 0;
	return 0;
}

/***************************************************** 
a wrapper for read()
*******************************************************/
int _System smbwrp_read(cli_state * cli, smbwrp_file * file, void *buf, unsigned long count, unsigned long * result)
{
	int ret;

	if (!cli || !file || !buf || !result) 
	{
		return maperror(EINVAL);
	}
	size_t nread;
	*result = 0;
	ret = cli_read(cli, file->fd, buf, file->offset, count, &nread);
	if (ret == -1) 
	{
		debuglocal(4,"cli_read failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}

	file->offset += nread;
	*result = nread;
	debuglocal(4," smbwrp_read successful, nread = %d, ret = %d\n",nread,ret);
	return 0;	
}

	

/***************************************************** 
a wrapper for write()
*******************************************************/
int _System smbwrp_write(cli_state * cli, smbwrp_file * file, void *buf, unsigned long count, unsigned long * result)
{
	NTSTATUS status;
	size_t ret;

	if (!cli || !file || !buf || !result) 
	{
		return maperror(EINVAL);
	}
	
	*result = 0;
//debuglocal(1,("Write %x %d %lld %d", cli, file->fd, file->offset, count));
	status = cli_writeall(cli, file->fd, 0, buf, file->offset, count, &ret);
	if (!NT_STATUS_IS_OK(status)) {
		debuglocal(4,"cli_writeall failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}

	file->updatetime = 1;
	file->offset += ret;
	*result = ret;
	return 0;	
}

/***************************************************** 
a wrapper for close()
*******************************************************/
int _System smbwrp_close(cli_state * cli, smbwrp_file * file)
{
	int rc = 0;
	if (!cli || !file) 
	{
		return maperror(EINVAL);
	}

	debuglocal(4,"smpwrp_close updatetime: %d\n", file->updatetime);

	if (file->updatetime == 1)
	{
		file->btime = 0; //no change
		file->mtime = time(NULL);
		file->atime = 0; //no change
		file->ctime = 0; //no change
		debuglocal(4,"cli_close new mtime %lu\n", file->mtime);
	}
	
	if (!NT_STATUS_IS_OK(cli_close(cli, file->fd)))
	{
		debuglocal(4,"cli_close failed - %s\n", cli_errstr(cli));
		rc = os2cli_errno(cli);
	}

	if (!rc && (file->openattr || file->mtime || file->ctime))
	{
		debuglocal(4,"Set pathinfo on close %s %08x b%d a%d m%d c%d\n", file->fname, file->openattr, file->btime, file->atime, file->mtime, file->ctime);

#if 0
		if (!NT_STATUS_IS_OK(cli_setpathinfo_basic(cli, file->fname, file->ctime, 0, file->mtime, 0, file->openattr))) 
#else
		if (!NT_STATUS_IS_OK(cli_setpathinfo_basic(cli, file->fname, file->btime, file->atime, file->mtime, file->ctime, file->openattr))) 
#endif
		{
		debuglocal(4,"cli_setpathinfo_basic in smbwrp_close failed - %s\n", cli_errstr(cli));
				//rc = os2cli_errno(cli);
		}
	}

	file->openattr = 0;
	file->btime = 0;
	file->atime = 0;
	file->mtime = 0;
	file->ctime = 0;
	file->updatetime = 0;
	file->fd = -1;
	file->offset = 0;
	*file->fname = 0;
	return rc;
}

int _System smbwrp_setfilesize(cli_state * cli, smbwrp_file * file, long long newsize)
{
	int rc = 0;
	if (!cli || !file) 
	{
		return maperror(EINVAL);
	}

	debuglocal(4,"smbwrp_setnewfilesize(%s) %lld\n", file->fname, newsize);
	if (!NT_STATUS_IS_OK(cli_ftruncate(cli, file->fd, newsize)))
	{
	debuglocal(4,"smbwrp_setnewfilesize - cli_ftruncate errno = %d\n",cli_errno(cli));
#if 0 /* This is all sorts of bad - if cli_ftruncate fails, it creates a new file in it's place */
		if (newsize)
		{
			rc = os2cli_errno(cli);
		}

		if (!NT_STATUS_IS_OK(cli_close(cli, file->fd)))
		{
			return os2cli_errno(cli);
		}
		uint16_t fd = 0;
		file->fd = -1;
		file->offset = 0;			
		file->openmode &= ~(O_CREAT | O_EXCL);
		file->openmode |= O_TRUNC;
		debuglocal(4,"smbwrp_setnewfilesize : cli_open(%s) attr %08x mode %02x denymode %02x\n", file->fname, file->openattr, file->openmode, file->denymode);
		if (!NT_STATUS_IS_OK(cli_open(cli, file->fname, file->openmode, file->denymode, &fd)))
		{	
			return os2cli_errno(cli);
		}
		file->fd = fd;
#endif
	}	
	return 0;
}

/***************************************************** 
a wrapper for rename()
*******************************************************/
int _System smbwrp_rename(cli_state * cli, char *oldname, char *newname)
{
	if (!cli || !oldname || !newname) 
	{
		return maperror(EINVAL);
	}

	debuglocal(1,"Rename <%s> -> <%s>\n", oldname, newname);
	if (!NT_STATUS_IS_OK(cli_rename(cli, oldname, newname, false)))
	{
		debuglocal(4,"cli_rename failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}


/***************************************************** 
a wrapper for chmod()
*******************************************************/
int _System smbwrp_setattr(cli_state * cli, smbwrp_fileinfo *finfo)
{
	if (!cli || !finfo || !*finfo->fname) 
	{
		return maperror(EINVAL);
	}

debuglocal(4,"Setting on <%s> attr %04x, time %lu (timezone /%lu\n", finfo->fname, finfo->attr, finfo->mtime, finfo->mtime + get_time_zone(finfo->mtime));
	// we already have gmt time, so no need to add timezone
	// if (!cli_setatr(cli, finfo->fname, finfo->attr, finfo->mtime + (finfo->mtime == 0 ? 0 : get_time_zone(finfo->mtime)))
	if (!NT_STATUS_IS_OK(cli_setatr(cli, finfo->fname, finfo->attr, finfo->mtime))
		&& !NT_STATUS_IS_OK(cli_setatr(cli, finfo->fname, finfo->attr, 0)))
	{
		debuglocal(4,"cli_setatr failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}	
	return 0;
}

/***************************************************** 
a wrapper for unlink()
*******************************************************/
int _System smbwrp_unlink(cli_state * cli, const char *fname)
{
	if (!cli || !fname) 
	{
		return maperror(EINVAL);
	}
#if 0
	if (strncmp(cli->dev, "LPT", 3) == 0) 
	{
		int job = smbw_stat_printjob(cli, fname, NULL, NULL);
		if (job == -1) 
		{
			goto failed;
		}
		if (cli_printjob_del(cli, job) != 0) 
		{
			goto failed;
		}
	} else 
#endif
	if (!NT_STATUS_IS_OK(cli_unlink(cli, fname, aSYSTEM | aHIDDEN))) 
	{
		debuglocal(4,"cli_unlink failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}

/***************************************************** 
a wrapper for lseek()
*******************************************************/
int _System smbwrp_lseek(cli_state * cli, smbwrp_file * file, int whence, long long offset)
{
	off_t size;
	NTSTATUS status;

	if (!cli || !file) 
	{
		return maperror(EINVAL);
	}

	debuglocal(4,"lseek %d %lld %lld\n", whence, offset, file->offset);

	switch (whence) {
	case SEEK_SET:
		if (offset < 0)
		{
			return maperror(EINVAL);
		}
		file->offset = offset;
		break;
	case SEEK_CUR:
		file->offset += offset;
		break;
	case SEEK_END:
		if (offset > 0)
		{
			return maperror(EINVAL);
		}
		status = cli_qfileinfo_basic(cli, file->fd, 
				   NULL, &size, NULL, NULL, NULL, 
				   NULL, NULL);
		if (!NT_STATUS_IS_OK(status)) {
			status = cli_getattrE(cli, file->fd, 
				  NULL, &size, NULL, NULL, NULL);
			if (!NT_STATUS_IS_OK(status)) {
				debuglocal(4,"call to cli_getattrE from smbwrp_lseek failed (Error %s)\n", nt_errstr(status));
				return os2cli_errno(cli);
			}
		}
		file->offset = size + offset;
		break;
	default: return maperror(EINVAL);
	}

	return 0;
}

/***************************************************** 
try to do a QPATHINFO and if that fails then do a getatr
this is needed because win95 sometimes refuses the qpathinfo
loosely based on SMBC_getatr() from source3/libsmb/libsmb_file.c
*******************************************************/
int _System smbwrp_getattr(smbwrp_server *srv, cli_state * cli, smbwrp_fileinfo *finfo)
{
	SMB_INO_T ino = 0;
	struct timespec btime;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;

	if (!cli || !finfo || !*finfo->fname) 
	{
		return maperror(EINVAL);
	}
	debuglocal(4,"getattr %d %d <%s>\n", smb1cli_conn_capabilities(cli->conn) & CAP_NOPATHINFO2, smb1cli_conn_capabilities(cli->conn) & CAP_NT_SMBS, finfo->fname);

	if (NT_STATUS_IS_OK(cli_qpathinfo2(cli, finfo->fname, &btime, &atime, &mtime, &ctime,
			   (off_t *)&finfo->size, (unsigned short *)&finfo->attr, &ino)))
	{
		finfo->attr &= 0x7F;
		finfo->btime = convert_timespec_to_time_t(btime);
		finfo->atime = convert_timespec_to_time_t(atime);
		finfo->mtime = convert_timespec_to_time_t(mtime);
		finfo->ctime = convert_timespec_to_time_t(ctime);
		return 0;
	}

#if 0 // cli->fd not available in Samba 4.x
	if (cli->fd == -1)
	{
	   /* fd == -1 means the connection is broken */
	   return maperror(ENOTCONN);
	}
#endif

	debuglocal(4, "smbwrp_getattr, calling cli_qpathinfo3\n");
	if (NT_STATUS_IS_OK(cli_qpathinfo3(cli, finfo->fname, &btime, &atime, &mtime, &ctime,
			   (off_t *)&finfo->size, (unsigned short *)&finfo->attr, &ino)))
	{
		finfo->attr &= 0x7F;
		finfo->btime = convert_timespec_to_time_t(btime);
		finfo->atime = convert_timespec_to_time_t(atime);
		finfo->mtime = convert_timespec_to_time_t(mtime);
		finfo->ctime = convert_timespec_to_time_t(ctime);
		return 0;
	}

	/* If the path is not on a share (it is a workgroup or a server),
	 * then cli_qpathinfo2 obviously fails. Return some fake information
	 * about the directory.
	 */
	if (   *srv->server_name == 0
#if 0 /* Causes crashes with Samba 4.x */
	    || (strcmp(cli->dev,"IPC") == 0)
#endif
	    || *srv->share_name == 0 
	    || (stricmp(srv->share_name,"IPC$") == 0)
#if 0 /* Causes crashes with Samba 4.x */
	    || (strncmp(cli->dev,"LPT",3) == 0) 
#endif
	   ) 
	{
	    debuglocal(4,"getattr not a share.\n");
	    *(time_t *)&finfo->btime = time (NULL);
	    *(time_t *)&finfo->atime = time (NULL);
	    *(time_t *)&finfo->mtime = time (NULL);
	    *(time_t *)&finfo->ctime = time (NULL);
	    finfo->size = 0;
	    finfo->easize = 0;
	    finfo->attr = aDIR;
	    return 0;
	}
	
	/* if this is NT then don't bother with the getatr */
	if (smb1cli_conn_capabilities(cli->conn) & CAP_NT_SMBS/* && !(smb1cli_conn_capabilities(cli->conn) & CAP_NOPATHINFO2)*/) 
	{
		int ret = cli_errno(cli);
		// cli_qpathinfo* reports EINVAL when path of given file not exists
		// thus there is no real situation when EINVAL should be returned to 
		// client at this point, we just replace it to ENOTDIR
		if (ret == EINVAL)
		{
			ret = ENOTDIR;
		}
		return maperror(ret);
	}

	if (NT_STATUS_IS_OK(cli_getatr(cli, finfo->fname, (unsigned short *)&finfo->attr, &finfo->size, (time_t *)&finfo->mtime)))
	{
//debuglocal(2,("gotattr1 %08x <%s>\n", finfo->attr, finfo->fname));
		finfo->btime = finfo->btime;  
		finfo->atime = finfo->atime;  //was mtime
		finfo->mtime -= get_time_zone(finfo->mtime);
		finfo->ctime = finfo->ctime;  //was mtime
		return 0;
	}
	debuglocal(4,"smbwrp_getattr failed - %s\n", cli_errstr(cli));
	return os2cli_errno(cli);
}

/***************************************************** 
try to do a QPATHINFO and if that fails then do a getatr
this is needed because win95 sometimes refuses the qpathinfo
*******************************************************/
int _System smbwrp_fgetattr(cli_state * cli, smbwrp_file *file, smbwrp_fileinfo *finfo)
{
	struct timespec btime;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
	SMB_INO_T ino = 0;
	NTSTATUS status;

	if (!cli || !file || !finfo) 
	{
		return maperror(EINVAL);
	}

	strncpy(finfo->fname, file->fname, sizeof(finfo->fname) - 1);
	status = cli_qfileinfo_basic(cli, file->fd, 
			   (unsigned short *)&finfo->attr, (off_t *)&finfo->size, &btime, &atime, &mtime, &ctime,
			   &ino);
	if (!NT_STATUS_IS_OK(status))
	{
		status = cli_getattrE(cli, file->fd, 
			  (unsigned short *)&finfo->attr, (&finfo->size), (time_t *)&finfo->ctime, (time_t *)&finfo->atime, (time_t *)&finfo->mtime);
		if (!NT_STATUS_IS_OK(status))
		{
			debuglocal(4,"call to cli_getattrE from smbwrp_fgetattr failed (Error %s)\n", nt_errstr(status));
			return os2cli_errno(cli);
		}
		else
		{
			finfo->ctime -= get_time_zone(finfo->ctime);
			finfo->atime -= get_time_zone(finfo->atime);
			finfo->mtime -= get_time_zone(finfo->mtime);
		}
	}
	else
	{
		finfo->btime = convert_timespec_to_time_t(btime);
		finfo->atime = convert_timespec_to_time_t(atime);
		finfo->mtime = convert_timespec_to_time_t(mtime);
		finfo->ctime = convert_timespec_to_time_t(ctime);
	}

	return 0;
}

// =============================DIRECTORY ROUTINES============================
														
/***************************************************** 
add a entry to a directory listing
*******************************************************/
static NTSTATUS smbwrp_dir_add(const char* mnt, smbwrp_fileinfo *finfo, const char *mask, void *state)
{
	if (state && finfo)
	{
		filelist_state * st  = (filelist_state *)state;
	    	char fullname[ _MAX_PATH] = {0};
	    	debuglocal(8,"adding <%s> %d %d %d\n", finfo->fname, sizeof(st->finfo), st->datalen, sizeof(st->finfo.fname));
		memcpy(&st->finfo, finfo, sizeof(st->finfo));
		strncpy(fullname, st->dir, strlen(st->dir));
		strncat(fullname, finfo->fname, sizeof(fullname) - strlen(fullname) -1);
		strncpy(st->finfo.fname, fullname, sizeof(st->finfo.fname));
		getfindinfoL( st->pConn, st->plist, &st->finfo, st->ulAttribute, st->dir_mask);
	}
}

static void smbwrp_special_add(const char * name, void * state)
{
	smbwrp_fileinfo finfo = {0};

	if (!name)
	{
		return;
	}

	ZERO_STRUCT(finfo);

	strncpy(finfo.fname, name, sizeof(finfo.fname) - 1);
	finfo.attr = aRONLY | aDIR;

	smbwrp_dir_add("", &finfo, NULL, state);
}

static void smbwrp_printjob_add(struct print_job_info *job, void * state)
{
	smbwrp_fileinfo finfo = {0};

	ZERO_STRUCT(finfo);

//printf("Printjob <%s>\n", job->name);

	strncpy(finfo.fname, job->name, sizeof(finfo.fname) - 1);
	finfo.mtime = job->t - get_time_zone(job->t);
	finfo.atime = finfo.atime; //was mtime
	finfo.ctime = finfo.ctime; //was mtime
	finfo.btime = finfo.btime; //was mtime
	finfo.attr = aRONLY;
	finfo.size = job->size;

	smbwrp_dir_add("", &finfo, NULL, state);
}

static void smbwrp_share_add(const char *share, uint32_t type, 
			   const char *comment, void *state)
{
	smbwrp_fileinfo finfo = {0};

	// strip administrative names and printers from list
	if (type == STYPE_PRINTQ || strcmp(share,"IPC$") == 0) return;

	ZERO_STRUCT(finfo);

	strncpy(finfo.fname, share, sizeof(finfo.fname) - 1);
	finfo.attr = aRONLY | aDIR;	

	smbwrp_dir_add("", &finfo, NULL, state);
}

/***************************************************************
 Wrapper that allows SMB2 to list a directory.
 Synchronous only. 
 Based on cli_smb2_list
***************************************************************/

NTSTATUS list_files_smb2(struct cli_state *cli,
			const char *pathname,
			uint16_t attribute,
			NTSTATUS (*fn)(const char *,
				struct smbwrp_fileinfo *,
				const char *,
				void *),
			void *state)
{
	NTSTATUS status;
	uint16_t fnum = 0xffff;
	char *parent_dir = NULL;
	const char *mask = NULL;
	struct smb2_hnd *ph = NULL;
	bool processed_file = false;
	TALLOC_CTX *frame = talloc_stackframe();
	TALLOC_CTX *subframe = NULL;
	bool mask_has_wild;
	uint32_t max_trans;
	uint32_t max_avail_len;
	bool ok;

	void *dircachectx = NULL;
	smbwrp_fileinfo wrpfinfo;

	//Init cache - don't know number of files so init with 128
	dircachectx = dircache_write_begin(state, 128);

	if (smbXcli_conn_has_async_calls(cli->conn)) {
		/*
		 * Can't use sync call while an async call is in flight
		 */
		status = NT_STATUS_INVALID_PARAMETER;
		goto fail;
	}

	if (smbXcli_conn_protocol(cli->conn) < PROTOCOL_SMB2_02) {
		status = NT_STATUS_INVALID_PARAMETER;
		goto fail;
	}

	/* Get the directory name. */
	if (!windows_parent_dirname(frame,
				pathname,
				&parent_dir,
				&mask)) {
                status = NT_STATUS_NO_MEMORY;
		goto fail;
        }

	mask_has_wild = ms_has_wild(mask);

	status = cli_smb2_create_fnum(cli,
			parent_dir,
			0,			/* create_flags */
			SMB2_IMPERSONATION_IMPERSONATION,
			SEC_DIR_LIST|SEC_DIR_READ_ATTRIBUTE,/* desired_access */
			FILE_ATTRIBUTE_DIRECTORY, /* file attributes */
			FILE_SHARE_READ|FILE_SHARE_WRITE, /* share_access */
			FILE_OPEN,		/* create_disposition */
			FILE_DIRECTORY_FILE,	/* create_options */
			NULL,
			&fnum,
			NULL,
			NULL,
			NULL);

	if (!NT_STATUS_IS_OK(status)) {
		goto fail;
	}

	status = map_fnum_to_smb2_handle(cli,
					fnum,
					&ph);
	if (!NT_STATUS_IS_OK(status)) {
		goto fail;
	}

	/*
	 * ideally, use the max transaction size, but don't send a request
	 * bigger than we have credits available for
	 */
	max_trans = smb2cli_conn_max_trans_size(cli->conn);
	ok = smb2cli_conn_req_possible(cli->conn, &max_avail_len);
	if (ok) {
		max_trans = MIN(max_trans, max_avail_len);
	}

	do {
		uint8_t *dir_data = NULL;
		uint32_t dir_data_length = 0;
		uint32_t next_offset = 0;
		subframe = talloc_stackframe();

		status = smb2cli_query_directory(cli->conn,
					cli->timeout,
					cli->smb2.session,
					cli->smb2.tcon,
					SMB2_FIND_ID_BOTH_DIRECTORY_INFO,
					0,	/* flags */
					0,	/* file_index */
					ph->fid_persistent,
					ph->fid_volatile,
					mask,
					max_trans,
					subframe,
					&dir_data,
					&dir_data_length);

		if (!NT_STATUS_IS_OK(status)) {
			if (NT_STATUS_EQUAL(status, STATUS_NO_MORE_FILES)) {
				break;
			}
			goto fail;
		}

		do {
			struct file_info *finfo = talloc_zero(subframe,
							struct file_info);

			if (finfo == NULL) {
				status = NT_STATUS_NO_MEMORY;
				goto fail;
			}

			status = parse_finfo_id_both_directory_info(dir_data,
						dir_data_length,
						finfo,
						&next_offset);

			if (!NT_STATUS_IS_OK(status)) {
				goto fail;
			}

			if (dir_check_ftype((uint32_t)finfo->mode,
					(uint32_t)attribute)) {
				/*
				 * Only process if attributes match.
				 * On SMB1 server does this, so on
				 * SMB2 we need to emulate in the
				 * client.
				 *
				 * https://bugzilla.samba.org/show_bug.cgi?id=10260
				 */
				processed_file = true;

				//as samba and this client have different finfo, we need to convert
				memset(&wrpfinfo, 0, sizeof(wrpfinfo));
				wrpfinfo.size = finfo[0].size;
				wrpfinfo.attr = finfo[0].mode;
				wrpfinfo.btime = convert_timespec_to_time_t(finfo[0].btime_ts);
				wrpfinfo.atime = convert_timespec_to_time_t(finfo[0].atime_ts);
				wrpfinfo.mtime = convert_timespec_to_time_t(finfo[0].mtime_ts);
				wrpfinfo.ctime = convert_timespec_to_time_t(finfo[0].ctime_ts);
				wrpfinfo.easize = -1;
				strncpy(wrpfinfo.fname, finfo[0].name, sizeof(wrpfinfo.fname) -1);

				status = fn(cli->dfs_mountpoint,
					&wrpfinfo,
					pathname,
					state);
				// Also add the entry to the cache.
				dircache_write_entry(dircachectx, &wrpfinfo);

				if (!NT_STATUS_IS_OK(status)) {
					/* not sure why this is required on OS/2 */
					if (status != NT_STATUS_WAIT_1)
						break;
				}
				if (status == NT_STATUS_WAIT_1)
					status = NT_STATUS_OK;
			}
			TALLOC_FREE(finfo);

			/* Move to next entry. */
			if (next_offset) {
				dir_data += next_offset;
				dir_data_length -= next_offset;
			}
		} while (next_offset != 0);

		TALLOC_FREE(subframe);

		if (!mask_has_wild) {
			/*
			 * MacOSX 10 doesn't set STATUS_NO_MORE_FILES
			 * when handed a non-wildcard path. Do it
			 * for the server (with a non-wildcard path
			 * there should only ever be one file returned.
			 */
			status = STATUS_NO_MORE_FILES;
			break;
		}

	} while (NT_STATUS_IS_OK(status));

	if (NT_STATUS_EQUAL(status, STATUS_NO_MORE_FILES)) {
		status = NT_STATUS_OK;
	}

	if (NT_STATUS_IS_OK(status) && !processed_file) {
		/*
		 * In SMB1 findfirst returns NT_STATUS_NO_SUCH_FILE
		 * if no files match. Emulate this in the client.
		 */
		status = NT_STATUS_NO_SUCH_FILE;
	}
	dircache_write_end(dircachectx);
  fail:

	if (fnum != 0xffff) {
		cli_smb2_close_fnum(cli, fnum);
	}
	TALLOC_FREE(subframe);
	TALLOC_FREE(frame);
	return status;
}


/****************************************************************************
 Do a directory listing, calling fn on each file found.
 Modified from cli_list() in source3/libsmb/clilist.c
****************************************************************************/
static int list_files(struct cli_state *cli, const char *mask, uint16_t attribute,
		  NTSTATUS (*fn)(const char *, smbwrp_fileinfo *, const char *,
			     void *), void *state)
{
	TALLOC_CTX *frame = NULL;
	struct tevent_context *ev;
	struct tevent_req *req;
	NTSTATUS status = NT_STATUS_NO_MEMORY;
	struct file_info *finfo;
	size_t i, num_finfo;
	uint16_t info_level;
	void *dircachectx = NULL;
	smbwrp_fileinfo wrpfinfo;

	/* Try to get the listing from cache. */
	if (dircache_list_files(fn, state, &num_finfo))
	{
		/* Got from cache. */
		return(num_finfo);
	}

	if (smbXcli_conn_protocol(cli->conn) >= PROTOCOL_SMB2_02) {
		return list_files_smb2(cli, mask, attribute, fn, state);
	}

	frame = talloc_stackframe();

	if (smbXcli_conn_has_async_calls(cli->conn)) {
		/*
		 * Can't use sync call while an async call is in flight
		 */
		status = NT_STATUS_INVALID_PARAMETER;
		goto fail;
	}
	ev = samba_tevent_context_init(frame);
	if (ev == NULL) {
		goto fail;
	}

	info_level = (smb1cli_conn_capabilities(cli->conn) & CAP_NT_SMBS)
		? SMB_FIND_FILE_BOTH_DIRECTORY_INFO : SMB_FIND_EA_SIZE;

	req = cli_list_send(frame, ev, cli, mask, attribute, info_level);
	if (req == NULL) {
		goto fail;
	}
	if (!tevent_req_poll_ntstatus(req, ev, &status)) {
		goto fail;
	}

	status = cli_list_recv(req, frame, &finfo, &num_finfo);
	if (!NT_STATUS_IS_OK(status)) {
		goto fail;
	}

	dircachectx = dircache_write_begin(state, num_finfo);

	for (i=0; i<num_finfo; i++) {
		//as samba and this client have different finfo, we need to convert
		memset(&wrpfinfo, 0, sizeof(wrpfinfo));
		wrpfinfo.size = finfo[i].size;
		wrpfinfo.attr = finfo[i].mode;
		wrpfinfo.btime = convert_timespec_to_time_t(finfo[i].btime_ts);
		wrpfinfo.atime = convert_timespec_to_time_t(finfo[i].atime_ts);
		wrpfinfo.mtime = convert_timespec_to_time_t(finfo[i].mtime_ts);
		wrpfinfo.ctime = convert_timespec_to_time_t(finfo[i].ctime_ts);
		wrpfinfo.easize = -1;
		strncpy(wrpfinfo.fname, finfo[i].name, sizeof(wrpfinfo.fname) -1);

		fn(cli->dfs_mountpoint, &wrpfinfo, mask, state);
		// Also add the entry to the cache.
		dircache_write_entry(dircachectx, &wrpfinfo);
	}

	dircache_write_end(dircachectx);
 fail:
	TALLOC_FREE(frame);
	return num_finfo;
}

/***************************************************** 
open a directory on the server
*******************************************************/
int _System smbwrp_filelist(smbwrp_server *srv, cli_state * cli, filelist_state * state)
{
	if (!srv || !cli || !state || !*state->mask)
	{
		return maperror(EINVAL);
	}
	debuglocal(1,"Filelist <%s> on master <%s> wgrp <%s> server <%s> share <%s> clidev <%s>\n", state->mask, srv->master, srv->workgroup, srv->server_name, srv->share_name, cli->dev);
	if (*srv->workgroup == 0 && *srv->server_name == 0) 
	{
		smbwrp_special_add(".", state);
		smbwrp_special_add("..", state);
		cli_NetServerEnum(cli, srv->master, SV_TYPE_DOMAIN_ENUM,
				   smbwrp_share_add, state);
	} else 
	if (*srv->server_name == 0) 
	{
		smbwrp_special_add(".", state);
		smbwrp_special_add("..", state);

		cli_NetServerEnum(cli, srv->workgroup, SV_TYPE_ALL,
				   smbwrp_share_add, state);
	} else 
#if 0 /* Causes crashes with Samba 4.x */
	if ((strcmp(cli->dev,"IPC") == 0) || *srv->share_name == 0 || (stricmp(srv->share_name,"IPC$") == 0)) 
#else
	if (/*(strcmp(cli->dev,"IPC") == 0) ||*/ *srv->share_name == 0 || (stricmp(srv->share_name,"IPC$") == 0)) 
#endif
	{
		smbwrp_special_add(".", state);
		smbwrp_special_add("..", state);

		if (net_share_enum_rpc(cli, smbwrp_share_add, state) < 0 &&
			    cli_RNetShareEnum(cli,smbwrp_share_add, state) < 0) 
		{
			debuglocal(4,"cli_RNetShareEnum failed - %s\n", cli_errstr(cli));
			return os2cli_errno(cli);
		}
	} else 
#if 0 /* Causes crashes with Samba 4.x */
	if (strncmp(cli->dev,"LPT",3) == 0) 
	{
		smbwrp_special_add(".", state);
		smbwrp_special_add("..", state);
		if (cli_print_queue_state(cli, smbwrp_printjob_add, state) < 0) 
		{
			return os2cli_errno(cli);
		}
	} 
	else 
#endif
	{
		if (list_files(cli, state->mask, aHIDDEN|aSYSTEM|aDIR, 
			     smbwrp_dir_add, state) < 0) 
		{									      
			return os2cli_errno(cli);
		}
	}																		

	return 0;
}

/***************************************************** 
a wrapper for chdir()
*******************************************************/
int _System smbwrp_chdir(smbwrp_server *srv, cli_state * cli, char *fname)
{
	unsigned short mode = aDIR;
	smbwrp_fileinfo finfo = {0};
	if (!cli || !fname) 
	{
		return maperror(EINVAL);
	}

	strncpy(finfo.fname, fname, sizeof(finfo.fname) - 1);
	if (smbwrp_getattr(srv, cli, &finfo)) 
	{
		return os2cli_errno(cli);
	}

	if (!(finfo.attr & aDIR)) {
		return maperror(ENOTDIR);
	}

	return 0;
}


/***************************************************** 
a wrapper for mkdir()
*******************************************************/
int _System smbwrp_mkdir(cli_state * cli, char *fname)
{
	if (!cli || !fname) 
	{
		return maperror(EINVAL);
	}

	if (!NT_STATUS_IS_OK(cli_mkdir(cli, fname)))
	{
		debuglocal(4,"cli_mkdir failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}

/***************************************************** 
a wrapper for rmdir()
*******************************************************/
int _System smbwrp_rmdir(cli_state * cli, char *fname)
{
	if (!cli || !fname) 
	{
		return maperror(EINVAL);
	}

	if (!NT_STATUS_IS_OK(cli_rmdir(cli, fname)))
	{
		debuglocal(4,"cli_rmdir failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}

/***************************************************** 
set EA for a path
*******************************************************/
int _System smbwrp_setea(cli_state * cli, char *fname, char * name, unsigned char * value, int size)
{
	if (!cli || !fname || !name) 
	{
		return maperror(EINVAL);
	}
	if (!NT_STATUS_IS_OK(cli_set_ea_path(cli, fname, name, value, size)))
	{
		debuglocal(4,"cli_set_ea_path failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}

/***************************************************** 
set EA for a file
*******************************************************/
int _System smbwrp_fsetea(cli_state * cli, smbwrp_file *file, char * name, unsigned char * value, int size)
{
	if (!cli || !file || !name) 
	{
		return maperror(EINVAL);
	}
	if (!NT_STATUS_IS_OK(cli_set_ea_fnum(cli, file->fd, name, value, size)))
	{
		debuglocal(4,"cli_set_ea_fnum failed - %s\n", cli_errstr(cli));
		return os2cli_errno(cli);
	}
	return 0;
}


#pragma pack(1)
typedef struct _FEA	 /* fea */
{
	 unsigned char fEA;	   /* flags			      */
	 unsigned char cbName;	/* name length not including NULL */
	 unsigned short cbValue;     /* value length */
} FEA;

typedef struct _FEALIST     /* feal */
{
	unsigned long cbList;       /* total bytes of structure including full list */
	FEA list[1];	/* variable length FEA structures */
} FEALIST;
#pragma pack()

static int unilistea(cli_state * cli, char *fname, void * buffer, unsigned long size)
{
	int fnum, i;
	int gotsize = sizeof(unsigned long);
	size_t num_eas;
	struct ea_struct *ea_list = NULL;
	TALLOC_CTX *mem_ctx;
	FEA * p;
	FEALIST * pfealist;
	char * q;

	mem_ctx = talloc_init("%d: ealist", _gettid());
	pfealist = (FEALIST *)buffer;
	pfealist->cbList = 0;

	if (!NT_STATUS_IS_OK(cli_get_ea_list_path(cli, fname, mem_ctx, &num_eas, &ea_list)))
	{
		debuglocal(4,"cli_ea_get_file list failed - %s\n", cli_errstr(cli));
		talloc_destroy(mem_ctx);
		return os2cli_errno(cli);
	}

	debuglocal(4,"num_eas = %d\n", num_eas);

	// we will count that os/2 max EA size for file is 64kb
	p = pfealist->list;
	for (i = 0; i < num_eas; i++) 
	{
		int namelen = strlen(ea_list[i].name);
		debuglocal(4, "%d Got EA <%s> with namelen %d, size %d. Gross %d. Buf %d\n", i, ea_list[i].name, namelen, ea_list[i].value.length, gotsize, size);
		if (namelen > 0xFF || ea_list[i].value.length > 0xFFFF)
		{
			debuglocal(4, "Skip EA <%s> with namelen %d, size %d\n", ea_list[i].name, namelen, ea_list[i].value.length);
			continue;
		}
		gotsize += sizeof(FEA) + namelen + ea_list[i].value.length + 1;
		if (size >= gotsize)
		{
			p->fEA = 0;
			p->cbName = namelen;
			p->cbValue = ea_list[i].value.length;
			q = (char *)(p + 1);
			strncpy(q, ea_list[i].name, namelen + 1);
			q += namelen + 1;
			memcpy(q, ea_list[i].value.data, ea_list[i].value.length);
			p = (FEA *)(q + ea_list[i].value.length);
		}
	}
	pfealist->cbList = gotsize;

	talloc_destroy(mem_ctx);
	return 0;
}

/***************************************************** 
lists EA of a path
*******************************************************/
int _System smbwrp_listea(cli_state * cli, char *fname, void * buffer, unsigned long size)
{
	if (!cli || !fname || !buffer)
	{
		return maperror(EINVAL);
	}

	debuglocal(4,"EAList for <%s>\n", fname);
	return unilistea(cli, fname, buffer, size);
}

/***************************************************** 
lists EA of a file
*******************************************************/
int _System smbwrp_flistea(cli_state * cli, smbwrp_file *file, void * buffer, unsigned long size)
{
	if (!cli || !file || !buffer)
	{
		return maperror(EINVAL);
	}

	debuglocal(4,"FEALIst for <%s>\n", file->fname);
	return unilistea(cli, file->fname, buffer, size);
}

/****************************************************************************
Check the space on a device.
****************************************************************************/
int _System smbwrp_dskattr(cli_state * cli, FSALLOCATE *pfsa)
{
	uint64_t total, bsize, avail;

	if (!cli || !pfsa)
	{
		return maperror(EINVAL);
	}

	if (!NT_STATUS_IS_OK(cli_disk_size(cli, "", &bsize, &total, &avail))) 
	{
		debuglocal(4,"Error in cli_disk_size: %s\n",cli_errstr(cli));
		return os2cli_errno(cli);
	}

	debuglocal(4,"\n\t\t%" PRIu64
		" blocks of size %" PRIu64
		". %" PRIu64 " blocks available\n",
		total, bsize, avail);

	// YD currently Samba return it in MB!
	pfsa->cSectorUnit = 1;
	if (bsize >= 65536) 
	{
		pfsa->cUnit = total*1024;
		pfsa->cUnitAvail = avail*1024;
		pfsa->cbSector = bsize/1024;
	} 
	else if (total > 4294967295)
	{
		/* total will overflow unsigned long */
		int scale = ceil (total/4294967295);
		pfsa->cUnit = total/scale;
		pfsa->cUnitAvail = avail/scale;
		pfsa->cbSector = bsize*scale;
	}
	else 
	{
		pfsa->cUnit = total;
		pfsa->cUnitAvail = avail;
		pfsa->cbSector = bsize;
	}

	return 0;
}

/***************************************************** 
Send an echo to the server to confirm it is still alive
*******************************************************/
int _System smbwrp_echo(cli_state * cli)
{
	unsigned char garbage[16];
	NTSTATUS status;
	if (!cli)
	{
		return maperror(EINVAL);
	}
	/* Ping the server to keep the connection alive using SMBecho. */
	memset(garbage, 0xf0, sizeof(garbage));
	unsigned int old_timeout = cli->timeout;
	cli->timeout = 2000;// we don't want to wait 20 seconds
	status = cli_echo(cli, 1, data_blob_const(garbage, sizeof(garbage)));
	cli->timeout = old_timeout; // reset back to previous value
	if (NT_STATUS_IS_OK(status)) {
		return 0;
	} else {
		debuglocal(4," cli_echo failed: %s\n", nt_errstr(status));
		return -1;
	}
}
