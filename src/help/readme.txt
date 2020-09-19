#include <nversion.h>
                      Samba Client Plugin for <$OS>
                      Version <$NDPSMB_VERSION>
                      Build <$NDPSMB_BUILD>
                      __________________________________

CONTENTS
________

 1. Introduction
 2. License
 3. Installation
 3.1 <$ND>
 3.2 EVFS
 3.3 Uninstallation
 4. NDPSMB.DLL usage
 4.1 <$ND>
 4.2 EVFS
 5. Debugging
 6. Changelog


1. Introduction
_______________

Samba is a SMB/CIFS protocols file and print sharing server/client suite
originally made for the UNIX world to cooperate with corresponding Windows and 
<$OS> servers or clients. <$OS> has a native implementation of SMB/CIFS
server/client set - IBM LAN Manager and IBM Peer. These products haven't been 
updated for a long time and have some compatibility problems with modern 
Windows SMB/CIFS implementations.

This package is the <$OS> port of Samba client made as the plugin for

- <$ND> for <$OS> - a well known virtual file system for <$OS> or
- EVFS - the eComStation Virtual Filesystem for SMB.

<$OS> Samba client provides seamless access to SMB/CIFS file shares over 
your network using TCP/IP as the underlying transport protocol.

The <$OS> Samba client consists of one module:
   ndpsmb.dll - the <$ND>/EVFS plugin itself.
   
Releases prior to version 1.5.0 had a second module which is now deprecated:
   smbcd.exe  - the Samba client daemon, which actually processes the requests
                from ndpsmb.dll

2. License
__________

Samba is freely available under the GNU General Public License. You can get
the full sourcecode from our SVN repository, please refer to the Samba for 
<$OS> Homepage for details about downloading the source.

The <$ND> plugin is also available with full sourcecode.

So if you are using our plugin we kindly ask you to support netlabs.org.
Please refer to the Samba for <$OS> Homepage to learn how you can do
that!


Links:

Samba for <$OS> Homepage: http://samba.netlabs.org/
<$ND> Homepage: http://www.blueprintsoftwareworks.com/netdrive/
Samba Homepage: http://www.samba.org/


3. Installation
_______________

The plugin needs the klibc runtime named libc06*.dll (the innotek gcc runtime 
DLL) somewhere in your LIBPATH, if it is not there already (It comes with any
recent eComStation release).
The plugin needs the 32 bit TCP/IP stack. This stack comes with all eComStation
releases. If you have still the 16 bit stack, please search the net for the
32 bit stack.

3.1 <$ND>
____________

You must have <$ND> 2.2.1 or newer installed prior to installation of this
package. If you want to use 64bit file API, you should install 2.3 or newer
version of <$ND>. The installation is semi-automatic and consists of the 
following:

  - ndpsmb.dll installation :
      Run instpl.cmd from this package. It will place all files to the <$ND> 
      plugin directory.
      
3.2 EVFS
________

  - ndpsmb.dll installation : 
      Put ndpsmb.dll into x:\ecs\dll (where x: is your bootdrive)

3.3 Uninstallation
__________________

Run the uninstpl.cmd script in order to deinstall the plugin again.
This will work for both EVFS and <$ND>.

4. NDPSMB.DLL 
_____________


4.1. Usage with <$ND>
______________________

<$ND> Samba plugin is used according to general <$ND> usage rules. Read
the <$ND> documentation for more information. The SMB/CIFS resources could
be mounted either using command line nd.exe utility or WPS ndpm.exe utility
of <$ND>.

The <$ND> Samba plugin parameters are:

   workgroup     - the name of workgroup to connect to.
   server        - the name of server to connect to.
   share         - the name of server resource (share) to connect to.
   user          - the name of user to login under.
   password      - the password of selected user.
   master        - the name of 'master' server or workgroup, used to get the
                   list of workgroups when the concrete workgroup and server
                   not specified.
   masterttype   - 0 if 'master' is the name of master server, 1 if 'master'
                   is the name of master workgroup.
   cache timeout - the time in seconds for which the cache is valid. If the
                   time is over the client rereads the directory again. For a
                   directory with many files it may be better to increase the
                   timeout. (rule of thumb: per 500-800 files 10 seconds).
                   Default: 10 seconds
   cache listings- the amount of directories the cache should hold. Be aware:
                   the higher the value the more memory is used.
                   Default: 32 listings
   easupport     - 1 to support <$OS> Extended Attributes, 0 to not support it
   r/w           - define whether the resource should be read-only (r) or 
                   writeable (w)

To mount the exact server's share one need to specify both 'server' and
'share' parameters, the other parameters are optional. The root path of
server's share will be mounted to the mounting path.

If one want to access all server shares, then only the 'server', not the
'share' parameter, should be specified,the other parameters are optional.
The list of server shares will be mounted to the mounting path.

If one want to access all servers within the workgroup, then only the
'workgroup', not the 'server' parameter, should be specified, the other
parameters are optional. The list of servers within the specified workgroup
will be mounted to the mounting path.

If one want to access all available workgroups within one master browser
within the network, then only the 'master', not the 'workgroup' nor the
'server' parameter, should be specified, the other parameters are optional.
The list of workgroups will be mounted to the mounting path. The 'master'
workgroup can be any known workgroup in the net. The 'master' server should
be the master browser for workgroup[s].

By default the 'guest' user with blank password is used to access selected
resources. If you get access denied errors, try to specify another user 
and password combination.

4.2 Usage with EVFS
___________________

eComStation comes with a GUI program named EVFSGUI which is even more feature
rich than the <$ND> GUI. EFVSGUI can also be used together with <$ND>.
Refer to the documentaion that comes with eCS.

5. Debugging
____________

To produce a logfile please add an empty file called ndpsmb.dbg into the 
root drive.

The logfile is placed to your LOGFILES env path, if available. If the 
LOGFILES environment variable is missing, the logfile is placed into the 
<$ND> directory. The logfile is called log.ndpsmb for the <$ND> part
and log.smbc for the Samba part.


6. Changelog
____________

#include <changelog.txt>
