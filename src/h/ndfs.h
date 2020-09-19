/*
 * NetDrive File System (NDFS)
 *
 * Version 2.3.x
 *
 * (C) COPYRIGHT Blueprint Software Works, Inc. 1999-2004.
 * ALL RIGHTS RESERVED. 
 */

#ifndef __NDFS__H
#define __NDFS__H

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#if defined(__IBMC__) || defined(__IBMCPP__)
#define FNPTR(a) * a
#else
#define FNPTR(a) a *
#endif

#define NDENTRY APIENTRY
#define PNDENTRY FNPTR(NDENTRY)

#ifdef __cplusplus
    extern "C" {
#endif

    /* This is the file system ID as in DOSQUERYFSINFO */
    #define NDFS_ID              0x21401785UL
    
    /* Maximum length of a type name, including trailing '\0',
     * i.e. len is up to 7 characters and one nul character
     */
    #define ND_MAX_TYPE_SIZE     8

    /* resource independent flags for NdpMount */
    #define ND_FLAG_WRITEPROTECT      0x1
    #define ND_FLAG_CHECKDUPLICATE    0x2
    
    /* mount points flags */
    #define ND_MP_FLAG_NORMAL         0x0
    
    /* NdQueryInfo infolevels */
    #define ND_INFO_IFS            0
    #define ND_INFO_CTL            1
    #define ND_INFO_NDFS_PATH      2
    #define ND_INFO_DRIVES         3
    
    #pragma pack(1)
    typedef struct _RIDINFO
    {
        ULONG  ulReqCompleted;
        USHORT usReqMax;
        USHORT usReqInProcess;
        USHORT usReqMaxInProcess;
        USHORT usReqRetCnt;
        USHORT usReqCptCnt;
        ULONG  ulFreeHits;
        ULONG  ulCptHits;
        ULONG  ulFreeFaults;
        ULONG  ulCptFaults;
        ULONG  ulFreeScanned;
        ULONG  ulCptScanned;
    } RIDINFO;
    
    typedef struct _IFSINFO
    {
        CHAR    achName[8];
        USHORT  usVerMajor;
        USHORT  usVerMinor;
        RIDINFO ridInfo;
        USHORT  usCpgmConnected;
        USHORT  usCpgmPID;
    } IFSINFO;
    
    typedef struct _CTLINFO
    {
        USHORT  usCpgmConnected;
        USHORT  usCpgmPID;
        ULONG   ulVersion;
    } CTLINFO;
    
    typedef struct _NDFSPATHINFO
    {
        CHAR achNDFSPath[CCHMAXPATH];
    } NDFSPATHINFO;

    #pragma pack()


    APIRET NDENTRY  NdQueryInfo (ULONG infolevel,
                                  PVOID pBuf,
                                  PULONG pcbBuf);
                                 
    APIRET NDENTRY  NdAttach (PSZ pszDevice,
                               PSZ pszLabel,
                               ULONG cUnit,
                               ULONG cUnitAvail);

    APIRET NDENTRY  NdDetach (PSZ pszDevice);
    
    APIRET NDENTRY  NdSetDriveInfo (PSZ pszDevice,
                                     ULONG cUnit,
                                     ULONG cUnitAvail);
                                     
    APIRET NDENTRY  NdMount (PSZ pszType,
                              ULONG ulFlags,
                              PSZ pszDest, 
                              PSZ pszSrc);

    APIRET NDENTRY  NdRemount (PSZ pszType,
                                ULONG ulIndex,
                                ULONG ulFlags,
                                PSZ pszDest, 
                                PSZ pszSrc);

    APIRET NDENTRY  NdUnmount (PSZ pszDest, 
                                ULONG ulIndex);

    APIRET NDENTRY  NdIoctl (ULONG ulFunction,
                              PSZ pszDest, 
                              PVOID pData, 
                              ULONG ulDataLenIn,
                              PULONG pulDataLenOut);
                              
    APIRET NDENTRY  NdPluginIoctl (ULONG ulFunction,
                                    PSZ pszPlugin, 
                                    PSZ pszRsrcType, 
                                    PSZ pszDest, 
                                    PVOID pData, 
                                    ULONG ulDataLenIn,
                                    PULONG pulDataLenOut);

    APIRET NDENTRY NdCreateMountPoint (PSZ pszPath,
                                        ULONG ulFlags);
    
    APIRET NDENTRY NdDeleteMountPoint (PSZ pszPath);
    
    APIRET NDENTRY NdKill (VOID);
    
    APIRET NDENTRY NdLoadPlugin (PSZ pszName);

    APIRET NDENTRY NdFreePlugin (PSZ pszName);

    #define ORD_NDQUERYINFO           1
    #define ORD_NDATTACH              2
    #define ORD_NDDETACH              3
    #define ORD_NDSETDRIVEINFO        4
    #define ORD_NDMOUNT               5
    #define ORD_NDUNMOUNT             6
    #define ORD_NDIOCTL               7
    #define ORD_NDCREATEMOUNTPOINT    8
    #define ORD_NDDELETEMOUNTPOINT    9
    #define ORD_NDKILL               10
    #define ORD_NDLOADPLUGIN         11
    #define ORD_NDFREEPLUGIN         12
    #define ORD_NDPLUGINIOCTL        14
    #define ORD_NDREMOUNT            15
    

        /* IOCTL functions */
        /* Path related function, pszDest != NULL */
        #define ND_IOCTL_QUERYMOUNTPOINT        0
        #define ND_IOCTL_QUERYMOUNTINFO         1
        #define ND_IOCTL_REFRESH                2
        #define ND_IOCTL_QUERYMOUNTINFOSHORT    3
        /* 4 is obsoleted */
        #define ND_IOCTL_RSRCINFO               5
        #define ND_IOCTL_REFRESHTREE            6
        #define ND_IOCTL_QUERYPATHINFO          7
        #define ND_IOCTL_SETWRITEPROTECT        8
        
        /* Plugin related function, pszDest == NULL */
        #define ND_IOCTL_QUERYPLUGINS         100
        #define ND_IOCTL_QUERYRSRCTYPES       101
        #define ND_IOCTL_QUERYTYPEINFO        102

        
        #pragma pack(1)
        
        /* IOCTL in and out data */
        
        /* ND_IOCTL_QUERYMOUNTPOINT: 
         *   Query if the specified path is an existing mount point.
         */
        /* no in */
        /* no out */
        
        
        /* ND_IOCTL_QUERYMOUNTINFO:
         *   Query mounted resources and child mount point information
         *   on the specified mount point path.
         */
        /* no in */
        /* out */
        typedef struct _ND_DATA_MOUNTINFO        
        {
            USHORT usMountPointsTotal;
            USHORT usMountPointsReturned;
            USHORT usResourcesTotal;
            USHORT usResourcesReturned;
            ULONG  cbName;      /* length of the name without the trailing '\0' */
            CHAR   achName[1];  /* name of the mount point */
            /* usMountPointsReturned ND_DATA_MOUNTPOINT structures */
            /* usResourcesReturned ND_DATA_MOUNTRESOURCE structures */
        } ND_DATA_MOUNTINFO;
        typedef ND_DATA_MOUNTINFO *PND_DATA_MOUNTINFO;
        
        typedef struct _ND_DATA_MOUNTPOINT
        {
            ULONG  cbName;      /* length of the name without the trailing '\0' */
            CHAR   achName[1];  /* name of the mount point */
        } ND_DATA_MOUNTPOINT;
        typedef ND_DATA_MOUNTPOINT *PND_DATA_MOUNTPOINT;
        
        typedef struct _ND_DATA_MOUNTRESOURCE
        {
            CHAR   achType[ND_MAX_TYPE_SIZE]; /* asciiz type of the resource */
            ULONG  ulFlags;         /* resource flags, 
                                     * low 16 bit - resource independent, 
                                     * high 16 bit - resource private flags 
                                     */
            ULONG  cbData;          /* length of the data without the trailing '\0' */
            CHAR   achData[1];      /* resource data */
        } ND_DATA_MOUNTRESOURCE;
        typedef ND_DATA_MOUNTRESOURCE *PND_DATA_MOUNTRESOURCE;
        
        
        /* ND_IOCTL_REFRESH:
         *   Refresh content of the specified directory. Allows to invalidate
         *   cache for this directory listing.
         */
        /* no in */
        /* no out */
         
        
        /* ND_IOCTL_QUERYMOUNTINFOSHORT:
         *   Query mounted resources and child mount point numbers
         *   on the specified mount point path.
         */
        /* no in */
        /* out */
        typedef struct _ND_DATA_MOUNTINFOSHORT
        {
            USHORT usMountPointsTotal;
            USHORT usResourcesTotal;
            ULONG  cbName;        /* length of the name without the trailing '\0' */
            CHAR   achName[CCHMAXPATH]; /* name of the mount point */
        } ND_DATA_MOUNTINFOSHORT;
        typedef ND_DATA_MOUNTINFOSHORT *PND_DATA_MOUNTINFOSHORT;
        
        
        /* ND_IOCTL_RSRCINFO:
         *   Query information about resource with specified index in the
         *   specified mount point.
         */
        /* in */
        typedef struct _ND_PARAM_INDEX
        {
            ULONG ulIndex; /* index of resource */
        } ND_PARAM_INDEX;
        /* out */
        typedef struct _ND_DATA_RSRCINFO
        {
            ULONG  ulType;       /* numeric value of resource type, index in types array */
            ULONG  ulFlags;      /* flags of the resource */
            ULONG  cbData;       /* length of achData without trailing '\0' */
            BYTE   cbType;       /* length of achType without trailing '\0' */
            BYTE   bExternal;    /* =1 if external plugin */
            BYTE   cbPlugin;     /* length of achPlugin without trailing '\0' */
            CHAR   achType[1];   /* type of resource */
            CHAR   achPlugin[1]; /* name of plugin */
            CHAR   achData[1];   /* resource data */
        } ND_DATA_RSRCINFO;
        
        #define RSRCINFO_SIZE(p) (sizeof (*p) + p->cbData + p->cbType + p->cbPlugin)
        #define RSRCINFO_TYPE(p) (&(p->achType[0]))
        #define RSRCINFO_PLUGIN(p) (RSRCINFO_TYPE(p) + sizeof (p->achType) + p->cbType)
        #define RSRCINFO_DATA(p) (RSRCINFO_PLUGIN(p) + sizeof (p->achPlugin) + p->cbPlugin)
        
        #define RSRC_TYPE_ERROR 0xFFFF0000L
        
         
        /* ND_IOCTL_REFRESHTREE:
         *   Refresh content of the specified directory and its descendants. 
         *   Allows to invalidate cache for the whole tree.
         */
        /* no in */
        /* no out */
         
         
        /* ND_IOCTL_QUERYPATHINFO:
         *   Query NetDrive path information.
         */
        /* no in */
        /* out */
        typedef struct _ND_DATA_QUERYPATHINFO        
        {
            USHORT cb;           /* size of the structure including the cb */
            BYTE   bPathType;    /* see ND_PT_* */
            ULONG  ulType;       /* numeric value of resource type */
            ULONG  ulIndex;      /* index of resource */
            BYTE   cbType;       /* length of achType without trailing '\0' */
            BYTE   cbPlugin;     /* length of achPlugin without trailing '\0' */
            BYTE   cbMountPoint; /* length of achMountPoint without trailing '\0' */
            BYTE   cbRsrcPath;   /* length of achRsrcPath without trailing '\0' */
            CHAR   achType[1];   /* type name */
            CHAR   achPlugin[1]; /* plugin name */
            CHAR   achMountPoint[1]; /* mount point path */
            CHAR   achRsrcPath[1]; /* path after mount point */
        } ND_DATA_QUERYPATHINFO;
        typedef ND_DATA_QUERYPATHINFO *PND_DATA_QUERYPATHINFO;
        
        /* Values for ND_DATA_QUERYPATHINFO::bPathType */
        #define ND_PT_MOUNTPOINT 0
        #define ND_PT_INTERNAL   1
        #define ND_PT_EXTERNAL   2
        
        #define PATHINFO_SIZE(p) (sizeof (*p) + p->cbType + p->cbPlugin + p->cbMountPoint + p->cbRsrcPath)
        #define PATHINFO_TYPE(p) (&(p->achType[0]))
        #define PATHINFO_PLUGIN(p) (PATHINFO_TYPE(p) + sizeof (p->achType) + p->cbType)
        #define PATHINFO_MOUNTPOINT(p) (PATHINFO_PLUGIN(p) + sizeof (p->achPlugin) + p->cbPlugin)
        #define PATHINFO_RSRCPATH(p) (PATHINFO_MOUNTPOINT(p) + sizeof (p->achMountPoint) + p->cbMountPoint)
        
        
        /* ND_IOCTL_SETWRITEPROTECT:
         *   Sets write protect flag for resource with specified index in the
         *   specified mount point.
         */
        /* in */
        typedef struct _ND_PARAM_SETWRITEPROTECT
        {
            ULONG ulIndex;       /* index of resource */
            ULONG fWriteProtect; /* 1 - write protect, 0 - write enable */
        } ND_PARAM_SETWRITEPROTECT;
        /* no out */
        
        
        /* ND_IOCTL_QUERYPLUGINS:
         *   Queries list of loaded plugins.
         */
        /* no in */
        /* out */
        typedef struct _ND_DATA_PLUGINLIST
        {
            ULONG cbSize;   /* size of plugin list (with ND_DATA_PLUGINLIST) */
            ULONG ulCount;  /* number of plugins returned */
            /* ulCount ND_DATA_PLUGIN structures follow */
        } ND_DATA_PLUGINLIST;
        
        typedef struct _ND_DATA_PLUGIN
        {
            HMODULE hMod;       /* placeholder, always zero */
            BYTE    bExternal;  /* = 1 if external plugin; 0 if internal */
            BYTE    cbName;     /* length of the name without the trailing '\0' */
            CHAR    szName[1];  /* plugin dll: ndpftp.dll -> name == ndpftp */
        } ND_DATA_PLUGIN;
        
        
        /* ND_IOCTL_QUERYRSRCTYPES:
         *   Query type names for specified plugin.
         */
        /* in */
        typedef struct _ND_PARAM_QUERYRSRCTYPES
        {
            BYTE cbName;        /* length of the name without the trailing '\0' */
            char szName[1];     /* plugin name */
        } ND_PARAM_QUERYRSRCTYPES;
        /* out */
        typedef struct _ND_DATA_RSRCTYPELIST
        {
            ULONG cbSize;   /* size of type list (with ND_DATA_RSRCTYPELIST) */
            ULONG ulCount;  /* number of types returned */
            /* ulCount ND_DATA_RSRCTYPE structures follow */
        } ND_DATA_RSRCTYPELIST;
        
        typedef struct _ND_DATA_RSRCTYPE
        {
            BYTE    cbType;     /* length of the name without the trailing '\0' */
            CHAR    szType[1];  /* type: local, ftp ... */
        } ND_DATA_RSRCTYPE;
        
        
        /* ND_IOCTL_QUERYTYPEINFO:
         *   Query plugin name that supports the specified type.
         */
        /* in */
        typedef struct _ND_PARAM_NAME
        {
            BYTE cbName;      /* length of the name without the trailing '\0' */
            CHAR achName[1];  /* the type name */
        } ND_PARAM_NAME;
        /* out */
        typedef struct _ND_DATA_TYPEINFO
        {
            ULONG  ulType;       /* numeric value of resource type, index in types array */
            BYTE   cbType;       /* length of achType without trailing '\0' */
            BYTE   cbPlugin;     /* length of achPlugin without trailing '\0' */
            BYTE   bExternal;    /* = 1 if external plugin */
            CHAR   achType[1];   /* type name */
            CHAR   achPlugin[1]; /* plugin name */
        } ND_DATA_TYPEINFO;
        
        #define TYPEINFO_SIZE(p) (sizeof (*p) + p->cbType + p->cbPlugin)
        #define TYPEINFO_TYPE(p) (&(p->achType[0]))
        #define TYPEINFO_PLUGIN(p) (TYPEINFO_TYPE(p) + sizeof (p->achType) + p->cbType)
        
        #pragma pack()

#ifdef __cplusplus
    }
#endif

#endif /* __NDFS__H */
