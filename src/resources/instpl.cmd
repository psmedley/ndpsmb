/* Install NetDrive Plugin */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs
'@echo off'

parse source . . script
scriptpath = strip(filespec('D', script)||filespec('P', script),'T','\')
ok = directory(scriptpath)

/* Add supported languages here */
langs = "DE,FR,EN"

ok = SysFileTree("ndp*.dll",exist.,"FO")
if exist.0 = 0 then do
  say "Plugin not found in current directory! Aborting..."
  exit -1
end
else plugin = translate(left(filespec("N",exist.1),length(filespec("N",exist.1))-4))

lang = translate(arg(1))
if lang = "" then do
  lang = translate(left(strip(value("LANG",,"OS2ENVIRONMENT")),2))
end

NDFSDir = value("NDFSDIR",,"OS2ENVIRONMENT")
/* For SMB we also support EVFS */
if NDFSDir = "" & plugin = "NDPSMB" then do
   say 'Installing 'plugin' for EVFS.'
   say 'Install 'plugin'.dll    to 'SysBootDrive()'\ecs\dll\'plugin'.dll'
   'copy 'plugin'.DLL 'SysBootDrive()'\ecs\dll\'plugin'.dll 1>NUL'
   say 'Install 'lang'\'plugin'.hlp to 'SysBootDrive()'\ecs\help\'plugin'.hlp'
   'copy .\'lang'\'plugin'.hlp 'SysBootDrive()'\ecs\help\'plugin'.hlp 1>NUL'
   exit 0
end

if pos(lang,langs) = 0 then lang = 'EN'
say 'Installing 'plugin' for 'lang
'copy .\'lang'\'plugin'.hlp 1>NUL'
NDFSDir'\nd.exe plugin install 'plugin'.ndp'
ok = SysFileDelete(plugin".hlp")
exit 0
