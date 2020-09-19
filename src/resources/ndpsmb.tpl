#include "nversion.h"

<WARPIN VERSION="1.0.18" CODEPAGE=850">
<HEAD>
<TITLE>Samba Plugin installer</TITLE>

<REXX NAME=ChkREQ>
    call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
    call SysLoadFuncs
    /* based upon work from Dmitriy Kuminov */
    parse arg aArgs
    Message = ''; Package = ''
    if pos(" Message:",aArgs) = 0 
    then parse arg aFile " Package:" Package
    else parse arg aFile " Message:" Message
    name = filespec('N', aFile)
    ext = translate(right(name, 4))
    search_path = ""
    select
      when (ext == '.DLL') then do
        config_sys = SysBootDrive()'\CONFIG.SYS'
        do while lines(config_sys)
          line = strip(linein(config_sys))
          if (left(line, 8) == 'LIBPATH=') then do
            search_path = substr(line, 9)
            leave
          end
        end
        call lineout config_sys
        search_path = SysQueryExtLibPath('B')';'search_path';'SysQueryExtLibPath('E')
      end
      when (ext == '.EXE') then search_path = value('PATH',, 'OS2ENVIRONMENT')
      when (ext == '.HLP') then search_path = value('BOOKSHELF',, 'OS2ENVIRONMENT')
      otherwise search_path = ''
    end
    if (search_path \= '') then do
      ok = value('CHKREQ_SEARCH_PATH', search_path, 'OS2ENVIRONMENT')
      real_file = SysSearchPath('CHKREQ_SEARCH_PATH', name)
      ok = value('CHKREQ_SEARCH_PATH', '', 'OS2ENVIRONMENT')
    end
    else real_file = ""

    /* generate the message string to return */
    if real_file <> "" then MsgStr = ""; else do
      if Message = '' then MsgStr = 'REQUIRES="' || strip(Package) || '"'
      else MsgStr = Message
    end
    return MsgStr
</REXX>

<REXX NAME=chkndenv>
    rc=RxFuncAdd('SysLoadFuncs','REXXUTIL','SysLoadFuncs')
    rc=SysLoadFuncs
    BaseDir=translate(strip(value("NDFSDIR",,"OS2ENVIRONMENT")))
    if BaseDir = '' then do
        ms=WirexxShowMessage('NetDrive not found','Installing plugin for EVFS.',0)
        BaseDir = SysBootDrive()||'\ecs\dll'
    end
    rc = WirexxPutEnv('ndfs',BaseDir)
</REXX>

<REXX NAME=nls>
    call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
    call SysLoadFuncs
    langs = "EN,DE,FR"
    lang = translate(left(strip(value("LANG",,"OS2ENVIRONMENT")),2))
    if pos(lang,langs) = 0 then lang = 'EN'

    SELECT
        when lang = 'FR' then do
            rc = WirexxPutEnv('welcome','Bienvenue dans l''installation du composant enfichable Samba pour NetDrive!')
            rc = WirexxPutEnv('accept','Veuillez prendre connaissance et accepter l''accord de licence suivant.')
            rc = WirexxPutEnv('target','Veuillez sÇlectionner le dossier de destination')
            rc = WirexxPutEnv('re_install','Si vous effectuez une mise Ö jour de SearchPlus, le dossier d''installation par dÇfaut sera celui actuellement utilisÇ par l''ancienne version.')
            rc = WirexxPutEnv('ready','Veuillez confirmer les options d''installation.')
            rc = WirexxPutEnv('scfind','Modifications du fichier CONFIG.SYS prÇvues: paramÇtrage de la variable SCFINDUTILITY Ö SearchPlus.exe pour en faire le programme de recherche du systäme par dÇfaut. Ces modifications nÇcessiteront un redÇmarrage.')
            rc = WirexxPutEnv('over_write','Veuillez sÇlectionner de maniäre Ö Çcraser tout fichier existant.')
            rc = WirexxPutEnv('install','~Installation')
            rc = WirexxPutEnv('license','~J''accepte')
            rc = WirexxPutEnv('next','~Suivant')
            rc = WirexxPutEnv('fr_sel','SELECT DESELECT')
        end

        when lang = 'DE' then do
            rc = WirexxPutEnv('welcome','Willkommen zur Installation des Samba plugin fÅr NetDrive!')
            rc = WirexxPutEnv('accept','Bitte lesen folgenden Text und akzeptieren sie die Lizenzvereinbarung.')
            rc = WirexxPutEnv('target','WÑhlen Sie bitte das Zielverzeichnis.')
            rc = WirexxPutEnv('re_install','Wenn dies ein Update ist, dann ist das Installationsverzeichnis das aktuelle Verzeichnis des Plugins.')
            rc = WirexxPutEnv('install','~Installation')
            rc = WirexxPutEnv('license','~Ich stimme zu')
            rc = WirexxPutEnv('next','W~eiter')
            rc = WirexxPutEnv('de_sel','SELECT DESELECT')
        end

        otherwise do
            rc = WirexxPutEnv('welcome','Welcome to the installation of the Samba plugin for NetDrive!')
            rc = WirexxPutEnv('accept','Please read and accept the following license-agreement.')
            rc = WirexxPutEnv('target','Please select target directory.')
            rc = WirexxPutEnv('re_install','If you are updating the Samba plugin for NetDrive, the default installation directory will be the current directory.')
            rc = WirexxPutEnv('install','~Installation')
            rc = WirexxPutEnv('license','I ~accept')
            rc = WirexxPutEnv('next','~Next')
            rc = WirexxPutEnv('en_sel','SELECT DESELECT')
        end
    end
return
</REXX>

<REXX NAME=get_env>
    Parse Arg subject .
    Return WirexxGetEnv(subject)
</REXX>

<PCK INDEX=1
    PACKAGEID="netlabs.org\Samba\Client plugin\<$NDPSMB_INSTALL>"
    =("nls")
    =("chkndenv")
    =("ChkREQ LIBC065.DLL Package:netlabs.org\kLIBC\LIBC 0.6 Runtime\0\6\5")
    =("ChkREQ GCC446.DLL Package:netlabs.org\GCC4\Core\1\2\0")
    =("ChkREQ STDCPP.DLL Package:netlabs.org\GCC4\Core\1\2\1")
    TITLE="Samba Client Plugin for Netdrive and EVFS"
    TARGET="$(NDFS)\InstallPackages\ndpsmb"
    FIXED
    SELECT
    DEEXECUTE="$(NDFS)\InstallPackages\ndpsmb\uninst.cmd"
    >This package contains the core files.
</PCK>

<PCK INDEX=2
    PACKAGEID="netlabs.org\Samba\Client plugin help_EN\<$NDPSMB_INSTALL>"
    TARGET="$(1)"
    FIXED =("get_env en_sel")
    REQUIRES=1
    TITLE="English helpfile"
    EXECUTE="$(1)\instpl.cmd EN"
    >This package contains the English helpfile
</PCK>

<PCK INDEX=3
    PACKAGEID="netlabs.org\Samba\Client plugin help_FR\<$NDPSMB_INSTALL>"
    TARGET="$(1)"
    FIXED =("get_env fr_sel")
    REQUIRES=1
    TITLE="French helpfile"
    EXECUTE="$(1)\instpl.cmd FR"
    >This package contains the French helpfile
</PCK>

<PCK INDEX=4
    PACKAGEID="netlabs.org\Samba\Client plugin help_DE\<$NDPSMB_INSTALL>"
    TARGET="$(1)"
    FIXED =("get_env de_sel")
    REQUIRES=1
    TITLE="Deutsche Hilfedatei"
    EXECUTE="$(1)\instpl.cmd DE"
    >Dieses Paket enthÑlt die deutsche Hilfedatei
</PCK>

</HEAD>

<!-- Here come the different pages. They are linked by
     the <NEXTBUTTON> tags, which must have a target.
     Each page must have a TYPE= attribute, which tells
     WarpIn what will be visible on that page. -->
<BODY>
<!-- page 1: introductory page -->

<PAGE INDEX=1 TYPE=README>
    <NEXTBUTTON TARGET=2>=("get_env next")</NEXTBUTTON>
    <TEXT>=("get_env welcome")</TEXT>
    <README EXTRACTFROMPCK="1" format=PLAIN>readme.txt</README>
</PAGE> 
<PAGE INDEX=2 TYPE=README>
    <NEXTBUTTON TARGET=3>=("get_env next")</NEXTBUTTON>
<TEXT>
Additional requirements:
</TEXT>
<README FORMAT=HTML>
<P>=("ChkREQ Z.DLL Message:Please install the <B>Z.DLL</B> from http:/rpm.netlabs.org/release/00/zip/zlib-1_2_5-4_oc00.zip")
</README>
</PAGE>

<PAGE INDEX=3 TYPE=TEXT>
    <NEXTBUTTON TARGET=4>=("get_env next")</NEXTBUTTON>
<TEXT>
Additional explanation for Netdrive:

The NetDrive Control Program (ndctl.exe) must be started before the installation of this plugin. Normally it is started during boot.

You can start it via the "Control Panel" in your NetDrive folder (Menu NetDrive - Control Program - Start) or via commandline in your NDFSDIR (=("get_env ndfs")) (DETACH NDCTL.EXE).

INSTALLATION (OR DEINSTALLATION) WILL FAIL, IF THE CONTROL PROGRAM HAS NOT BEEN STARTED!

If you are not sure, please start the Control Program NOW and then continue the installation.</TEXT>

</PAGE>
    
<PAGE INDEX=4 TYPE=README>
      <NEXTBUTTON TARGET=5>=("get_env license")</NEXTBUTTON>

      <TEXT>=("get_env accept")</TEXT>
      
      <README EXTRACTFROMPCK="1" format=PLAIN>COPYING</README>

    </PAGE>
    <!-- The TYPE=CONTAINER will list the packages which can be installed. -->
    
   
    <PAGE INDEX=5 TYPE=CONTAINER>
      <NEXTBUTTON TARGET=0>=("get_env install")</NEXTBUTTON>
      <TEXT>=("get_env target") 
=("get_env re_install")</TEXT>
    </PAGE>

    <!-- Display another TEXT page to inform the user
            that installation will begin. We use the TARGET=0
            with the NEXTBUTTON tag which starts installation. -->
      </BODY>
</WARPIN>
