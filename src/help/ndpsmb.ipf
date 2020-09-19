#include "nversion.h"

:userdoc.

.nameit symbol=os text='<$OS>'
.nameit symbol=nd text='<$ND> for &os.'
.nameit symbol=ndcp text='<$ND> for &os. Control Panel'
.nameit symbol=mp text='mount point'
.nameit symbol=ressmb text='Samba Resource Properties'
.nameit symbol=usage text='Usage'
.nameit symbol=build text='Samba Client Plugin for &os. version <$NDPSMB_VERSION> build <$NDPSMB_BUILD>'

:title.<$ND> for &os. Control Panel Help

:docprof toc=123456.
.******************************************************
:h1.Introduction
:p.
Samba is an SMB/CIFS protocol file and print sharing client/server suite originally
created for the UNIX world in order to access Windows and &os. servers and
clients. &os. has several native implementations of SMB/CIFS client/server set - :hp1.IBM Peer:ehp1. (or :hp1.IBM LAN Requester:ehp1.,
or :hp1.IBM File and Print Client:ehp1.) and :hp1.IBM LAN Manager:ehp1.. These products haven&csq.t been updated for a long time and
have some compatibility problems with modern Windows SMB/CIFS implementations.
:p.
This package is the &os. port of the Samba client implemented as a plugin for&colon.
:ul.
:li.&nd. - a well known virtual file system for &os.
:li.EVFS - the eComStation Virtual Filesystem for SMB
:eul.:p.
The Samba Client Plugin for &os. provides seamless access to SMB/CIFS file shares over the network using pure TCP/IP as the
underlying transport protocol (no NetBIOS or NetBIOS over TCP/IP).
:p.
The Samba Client Plugin for &os. consists of one module&colon.
:ul.
:li.ndpsmb.dll - the <$ND> plugin itself
:eul.:p.
Releases prior to version 1.5.0 had a second module which is now deprecated&colon.
:ul.
:li.smbcd.exe  - the Samba client daemon, which actually processes the requests from ndpsmb.dll
:eul.
.******************************************************
:h1.License
:p.
Samba is freely available under the GNU General Public License v3. You may obtain the full source code either from the
project&csq.s SVN repository (please refer to the Samba for &os. homepage for details about downloading the source) or from
the Samba homepage (see :link reftype=hd res=003.Links:elink.).
:p.
The Samba Client Plugin for &os. is also available with full source code from the Samba for &os. SVN repository.
:p.
If you are using this plugin, we kindly ask you to support netlabs.org.
Please refer to the Samba for &os. homepage to learn how you may do that.
:h2 res=003.Links
:p.
:ul.
:li.Samba for &os. Homepage (http&colon.&slr.&slr.svn.netlabs.org&slr.samba)
:li.<$ND> Homepage (http&colon.&slr.&slr.www.bmtmicro.com&slr.netdrive)
:li.Samba Homepage (http&colon.&slr.&slr.www.samba.org)
:eul.
.*******************************************************
:h1 res=1000 group=2 x=left y=top width=100% height=100% scroll=both.&ressmb.
:i1 id=ressmb.&ressmb.
:ul.
:li.:link reftype=hd res=100.&ressmb.:elink.
:li.:link reftype=hd res=101.&usage.:elink.
:eul.
:h2 res=100.&ressmb.
:p.
The parameters for an SMB/CIFS FS resource are&colon.
:parml.
:pt.:hp2.Workgroup/Domain:ehp2.
:pd.The name of the workgroup or domain of the server.
:warning text='Default:'.
:hp1.blank:ehp1.
:ewarning.

:pt.:hp2.Server:ehp2.
:pd.The name or IP address of the server hosting the resource (share).
:warning text='Default:'.
:hp1.blank:ehp1.
:ewarning.

:pt.:hp2.Share:ehp2.
:pd.The name of the desired server resource (share).
:warning text='Default:'.
:hp1.blank:ehp1.
:ewarning.

:pt.:hp2.User ID:ehp2.
:pd.The username for authentication.
:warning text='Default:'.
:hp1.guest:ehp1.
:ewarning.

:pt.:hp2.Password:ehp2.
:pd.The password of the selected user.
:warning text='Default:'.
:hp1.blank:ehp1.
:ewarning.

:pt.:hp2.Master:ehp2.
:pd.The name of the browse master server or workgroup, used to get the list of available workgroups when the workgroup and server have not been specified.
:warning text='Default:'.
:hp1.WORKGROUP:ehp1.
:ewarning.

:pt.:hp2.Master type:ehp2.
:pd.Select :hp2.server:ehp2. or :hp2.workgroup:ehp2. to identify the type of master selected.
:warning text='Default:'.
:hp1.workgroup:ehp1.
:ewarning.

:pt.:hp2.Cache timeout:ehp2.
:pd.The time in seconds for which the cache is valid. Once the timeout is reached, the client re-reads the directory. For a directory with many
files, it may be better to increase the timeout (rule of thumb: per 500-800 files 10 seconds).
:warning text='Default:'.
:hp1.10 seconds:ehp1.
:ewarning.

:pt.:hp2.Cache listings:ehp2.
:pd.The number of directories the cache should hold. Be aware that the higher the value, the more memory consumed.
:warning text='Default:'.
:hp1.32 directories:ehp1.
:ewarning.

:pt.:hp2.Supports EA:ehp2.
:pd.Check to support <$OS> Extended Attributes in the target filesystem (and by the target server).
:warning text='Default:'.
:hp1.checked:ehp1.
:ewarning.

:pt.:hp2.r/w:ehp2.
:pd.define whether the resource should be read-only (r) or writeable (w).
:eparml.
.******************************************************
:h2 res=101.&usage.
:p.
To mount a specified share, it is necessary to specify both the :hp2.Server:ehp2. and
:hp2.Share:ehp2. parameters&semi. the other parameters are optional. The root of the 
server&csq.s share will be mounted to the &mp..
:p.
To access all server shares, only the :hp2.Server:ehp2. (not the :hp2.Share:ehp2.) 
should be specified&semi. the other parameters are optional. The list of
server shares will be mounted to the &mp..
:p.
To access all servers within the workgroup or domain, only the :hp2.Workgroup:ehp2.
(not the :hp2.Server:ehp2.) should be specified&semi. the other parameters are
optional. The list of servers within the specified workgroup or domain will be mounted to
the &mp..
:p.
To access all available workgroups listed by one master browser
on the network, only the :hp2.Master:ehp2. (not the :hp2.Workgroup:ehp2. nor the :hp2.Server:ehp2.) 
should be specified&semi. the other parameters are optional. The list of workgroups
will be mounted to the &mp.. The master workgroup can be any known (reachable)
workgroup on the network. The master server should be the master browser for the
workgroup(s).
:p.
By default, the :hp1.guest:ehp1. user with blank password is used to access the selected
resource. If guest logins are not permitted by the server, you will likely see &csq.access denied&csq.
errors. Specify an actual username and password combination.

.******************************************************
:h1.Version
:p.
&build.
:h1.Debugging
:p.
To produce a log file, please create an empty file named :hp1.ndpsmb.dbg:ehp1. in the root of the boot volume.
:p.
The log is created in the directory listed in the LOGFILES environment variable, if available.
If the LOGFILES environment variable has not been set, the log is created in the &nd. directory. The filename is
:hp1.log.ndpsmb:ehp1. for the &nd. log and :hp1.log.smbd:ehp1. for the Samba Client Plugin for &os. log.

.******************************************************
:h1.History
:p.Release history of the Samba client for &nd.
:ul.
:li.:link reftype=hd res=011.Version 1.0:elink.
:li.:link reftype=hd res=012.Version 1.5:elink.
:li.:link reftype=hd res=013.Version 2.0:elink.
:li.:link reftype=hd res=014.Version 2.1:elink.
:li.:link reftype=hd res=015.Version 2.2:elink.
:li.:link reftype=hd res=016.Version 2.3:elink.
:eul.
#include <changelog.txt>
.******************************************************
:h1.Credits
:ul.
:li.Silvan Scherrer (aka diver)
:li.Herwig Bauernfeind (aka HerwigB)
:li.Vitali Pelenyov (aka sunlover)
:li.Paul Smedley (aka Smedles)
:li.Yuri Dario (aka Paperino)
:li.Alex Taylor (aka AlexT)
:li.Nikolay Kolosov (aka nickk)
:li.Ko Myun Hun (aka komh)
:li.and all we missed 
:eul.
.******************************************************

:euserdoc.
