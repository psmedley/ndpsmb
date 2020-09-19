/* Uninstall routines for the plugin */

rc=RxFuncAdd('SysLoadFuncs','REXXUTIL','SysLoadFuncs')
rc=SysLoadFuncs

BaseDir=translate(strip(value("NDFSDIR",,"OS2ENVIRONMENT")))
if BaseDir <> "" then do
    ok = directory(BaseDir)
    address cmd 'nd.exe plugin REMOVE ndpsmb.ndp'
end
else do /* EVFS */
    BaseDir = SysBootDrive()'\ecs'
    ok = SysFileDelete(Basedir'\dll\ndpsmb.dll')
    ok = SysFileDelete(Basedir'\help\ndpsmb.hlp')
end
