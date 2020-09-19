/* 
 * NetDrive for OS/2 External Plugin's level 2 header file.
 *
 * (C) COPYRIGHT Blueprint Software Works, Inc. 1999-2004.
 * ALL RIGHTS RESERVED. 
 */

#ifndef __EXTPL2__H
#define __EXTPL2__H

#include <ndfs.h>

#ifdef __cplusplus
    extern "C" {
#endif

#pragma pack(1)

/* Plugin attributes */
#define NDPA_REUSE_FILE_CONNECTIONS (0x1)

#ifdef __EMX__
#define DEFINE_HANDLE(a) struct _##a; typedef struct _##a *a;
#else
#define DEFINE_HANDLE(a) struct _##a; typedef struct _##a *##a;
#endif

#ifndef __EXTPL__H
DEFINE_HANDLE(HRESOURCE)
DEFINE_HANDLE(HCONNECTION)
#endif

DEFINE_HANDLE(NDPROPERTYHANDLE)
DEFINE_HANDLE(NDMUTEX)
DEFINE_HANDLE(NDFILEHANDLE)

#define DOSATTR_READONLY	0x01
#define DOSATTR_HIDDEN		0x02
#define DOSATTR_SYSTEM		0x04
#define DOSATTR_VOLLABEL        0x08
#define DOSATTR_DIRECTORY	0x10
#define DOSATTR_ARCHIVED	0x20
#define DOSATTR_NON83		0x40

#define DOSATTR_NEEDEA		0x80

// flags for NDPLFILEINFO::ulFlags
#define NDFILE_PARENT  (0x1)
#define NDFILE_ROOT    (0x2)

typedef struct _NDFILEINFO
{
    FILESTATUS3 stat;    // standard OS/2 file information
    ULONG ulFlags;       // see NDFILE_PARENT, NDFILE_ROOT
    UCHAR cchName;       // length of the file name
    CHAR  *pszName;      // address of the name
    CHAR  *pszUpperName; // address of the uppercased name
} NDFILEINFO;

typedef struct _NDFILEINFOL
{
    FILESTATUS3L stat;   // standard OS/2 file information
    ULONG ulFlags;       // see NDFILE_PARENT, NDFILE_ROOT
    UCHAR cchName;       // length of the file name
    CHAR  *pszName;      // address of the name
    CHAR  *pszUpperName; // address of the uppercased name
} NDFILEINFOL;

typedef struct _NDDATABUF
{
    ULONG ulSize;
    void  *pData;
} NDDATABUF;


typedef struct _NDFSALLOCATE 
{
    ULONG   cSectorUnit;
    ULONG   cUnit;
    ULONG   cUnitAvail;
    USHORT  cbSector;
} NDFSALLOCATE;

enum {
    ND_PROP_STRING = 0,
    ND_PROP_ULONG  = 1
};

typedef struct _NDPROPERTYINFO
{
    ULONG ulType;
    ULONG ulReserved;
    CHAR *szName;
    CHAR *szDefaultValue;
} NDPROPERTYINFO;

/* defines for fsphWildMatch's ignorecase parameter */
#define ND_CASE_SENSITIVE 0
#define ND_IGNORE_CASE    1

typedef struct _NDPATHELEMENT
{
    ULONG length;
    CHAR  *name;
    CHAR  *upperName;
} NDPATHELEMENT;

typedef struct _NDTIMEDATE
{
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
} NDTIMEDATE;

DEFINE_HANDLE(NDTIMEHANDLE)

/*
 * Logging levels
 */
#define ND_LL_DEBUG (5)
#define ND_LL_INFO  (4)
#define ND_LL_WARN  (3)
#define ND_LL_ERROR (2)
#define ND_LL_FATAL (1)
#define ND_LL_NONE  (0)

DEFINE_HANDLE(NDLOGHANDLE)

#define NDMSGID(a) ((char *)(a & 0xFFFFul))

typedef struct _PLUGINHELPERTABLE2
{
    ULONG cb;

    int (PNDENTRY fsphQueryStringProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const CHAR **pvalue, ULONG *plen);
    int (PNDENTRY fsphQueryUlongProperty) (NDPROPERTYHANDLE handle, const CHAR *name, ULONG *pvalue);
    int (PNDENTRY fsphSetProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const CHAR *value);
    int (PNDENTRY fsphParseProperty) (NDPROPERTYHANDLE handle, const CHAR *s);
    
    int (PNDENTRY fsphAddFileFind16) (void *plist, FILEFINDBUF *pFindBuf, int cEntries, void *data, ULONG dataLength, ULONG flags);
    int (PNDENTRY fsphAddFileFind32) (void *plist, FILEFINDBUF3 *pFindBuf, void *data, ULONG dataLength, ULONG flags);
    void (PNDENTRY fsphInitFileFindBuf3) (FILEFINDBUF3 *pbuf, FILESTATUS3 *pstat, char *name);

    int (PNDENTRY fsphWildMatch) (const CHAR *pat, const CHAR *str, int ignorecase);
    int (PNDENTRY fsphAttrMatch) (ULONG pat, ULONG attr);

    int (PNDENTRY fsphSetResourceData) (void *plist, void *data, ULONG dataLength);
    int (PNDENTRY fsphGetResourceData) (NDFILEINFO *pfi, NDDATABUF *pdata, int expectedSize);
    int (PNDENTRY fsphGetFileInfoData) (NDFILEINFO *pfi, NDDATABUF *pdata, int expectedSize);

    NDPATHELEMENT * (PNDENTRY fsphPathElemNext) (int index, NDPATHELEMENT *pel);
    NDPATHELEMENT * (PNDENTRY fsphPathElemPrev) (int index, NDPATHELEMENT *pel);
    NDPATHELEMENT * (PNDENTRY fsphNameElem) (int index);
    int (PNDENTRY fsphIsLastElem) (int index, NDPATHELEMENT *pel);

    int (PNDENTRY fsphUpperCase) (CHAR *str);
    int (PNDENTRY fsphUpperCase2) (CHAR *str, int len);

    int (PNDENTRY fsphGEA16to32) (GEALIST *fpGEAList, GEA2LIST *fpGEA2List);
    int (PNDENTRY fsphGEA32to16) (GEA2LIST *fpGEA2List, GEALIST *fpGEAList);
    int (PNDENTRY fsphFEA16to32) (FEALIST *fpFEAList, FEA2LIST *fpFEA2List);
    int (PNDENTRY fsphFEA32to16) (FEA2LIST *fpFEA2List, FEALIST *fpFEAList);

    int (PNDENTRY fsphGetCurrentDateTime) (FDATE *pDate, FTIME *pTime);
    void (PNDENTRY fsphUnixTimeToDosDate) (ULONG time, FDATE *pd, FTIME *pt);
    void (PNDENTRY fsphDosDateToUnixTime) (FDATE d, FTIME t, ULONG *ptime);
    void (PNDENTRY fsphTimeUnix2TimeDate) (ULONG time, NDTIMEDATE *ptd);
    void (PNDENTRY fsphTimeTimeDate2Unix) (NDTIMEDATE *ptd, ULONG *ptime);
    int (PNDENTRY fsphTimeCreateHandle) (NDTIMEHANDLE *phandle, const CHAR *pTZ);
    void (PNDENTRY fsphTimeDeleteHandle) (NDTIMEHANDLE handle);
    void (PNDENTRY fsphTimeLocal2GMT) (NDTIMEHANDLE handle, NDTIMEDATE *plocal, NDTIMEDATE *pgmt);
    void (PNDENTRY fsphTimeGMT2Local) (NDTIMEHANDLE handle, NDTIMEDATE *pgmt, NDTIMEDATE *plocal);
    
    INT    (PNDENTRY fsphCharIsDBCS) (CHAR c);
    USHORT (PNDENTRY fsphLoadChar) (CHAR **pps);
    CHAR * (PNDENTRY fsphCopyChar) (CHAR **pdst, CHAR *src);
    CHAR * (PNDENTRY fsphStrChr) (const CHAR *pStr, CHAR c);
    CHAR * (PNDENTRY fsphStrRChr) (const CHAR *pStr, CHAR c);
    INT    (PNDENTRY fsphStrCmp) (const CHAR *s1, const CHAR *s2);
    INT    (PNDENTRY fsphStrNCmp) (const CHAR *s1, const CHAR *s2, INT n);
    INT    (PNDENTRY fsphStrICmp) (const CHAR *s1, const CHAR *s2);
    INT    (PNDENTRY fsphStrNICmp) (const CHAR *s1, const CHAR *s2, INT n);
    
    INT (PNDENTRY fsphQueryFilePtr) (ULONG *pulFilePtr);
    INT (PNDENTRY fsphQueryFileSize) (ULONG *pulFileSize);
    INT (PNDENTRY fsphSetFilePtr) (ULONG ulFileSize, ULONG ulCurrent, LONG lOffset, ULONG ulMethod, ULONG *pulActual);
    
    INT (PNDENTRY fsphSetAttrPattern) (void *plist, ULONG ulAttribute);
    
    ULONG (PNDENTRY fsphQueryCurrentCp) (void);
    
    CHAR (PNDENTRY fsphCpTranslateChar) (ULONG idCpSource, CHAR cSource, ULONG idCpDest);
    BOOL (PNDENTRY fsphCpTranslateString) (ULONG idCpSource, const CHAR *pszSource, ULONG idCpDest, ULONG cbLenDest, CHAR *pszDest);
    
    BOOL (PNDENTRY fsphAsciiToUnicode) (ULONG idCpSource, PSZ pszSource, ULONG cbLenDest, USHORT *pDest);
    BOOL (PNDENTRY fsphUnicodeToAscii) (USHORT *pSource, ULONG idCpDest, ULONG cbLenDest, CHAR *pDest);
    
    INT (PNDENTRY fsphLogOpen) (NDLOGHANDLE *phandle, ULONG loglevel, const char *logfile, const char *msgfile);
    void (PNDENTRY fsphLogMsg) (NDLOGHANDLE handle, ULONG ll, CHAR *msg, INT argc, ...);
    void (PNDENTRY fsphLogClose) (NDLOGHANDLE handle);

    INT (PNDENTRY fsphSetUlongProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const ULONG value);

    void (PNDENTRY fsphLogMsg2) (NDLOGHANDLE handle, ULONG ll, CHAR *msg, INT argc, CHAR **args);

    void * (PNDENTRY fsphAllocMem) (ULONG size);
    void * (PNDENTRY fsphReallocMem) (void *p, ULONG size);
    void (PNDENTRY fsphFreeMem) (void *p);

    void (PNDENTRY fsphUlong2String) (ULONG number, CHAR *string, INT n, INT base);
    void (PNDENTRY fsphLong2String) (LONG number, CHAR *string, INT n, INT base);
    INT (PNDENTRY fsphString2Ulong) (const CHAR *string, CHAR **pstring2, ULONG *pvalue, INT base);
    INT (PNDENTRY fsph_vsnprintf) (CHAR *buf, INT n, const CHAR *fmt, CHAR **args);
    INT (PNDENTRY fsph_snprintf) (CHAR *buf, INT n, const CHAR *fmt, ...);
    
    INT (PNDENTRY fsphCreateMutex) (NDMUTEX *pmutex);
    INT (PNDENTRY fsphRequestMutex) (NDMUTEX mutex, ULONG ulTimeout);
    INT (PNDENTRY fsphReleaseMutex) (NDMUTEX mutex);
    void (PNDENTRY fsphCloseMutex) (NDMUTEX mutex);

    INT (PNDENTRY fsphQueryFileStatus) (FILESTATUS3 *pstat3);
 
    INT (PNDENTRY fsphUniStrLen) (USHORT *str);
    
    INT (PNDENTRY fsphQueryFileOpenMode) (ULONG *pulOpenMode);
} PLUGINHELPERTABLE2;

// large files support (> 2Gb)
typedef struct _PLUGINHELPERTABLE2L
{
    ULONG cb;

    int (PNDENTRY fsphQueryStringProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const CHAR **pvalue, ULONG *plen);
    int (PNDENTRY fsphQueryUlongProperty) (NDPROPERTYHANDLE handle, const CHAR *name, ULONG *pvalue);
    int (PNDENTRY fsphSetProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const CHAR *value);
    int (PNDENTRY fsphParseProperty) (NDPROPERTYHANDLE handle, const CHAR *s);
    
    int (PNDENTRY fsphAddFileFind16) (void *plist, FILEFINDBUF *pFindBuf, int cEntries, void *data, ULONG dataLength, ULONG flags);
    int (PNDENTRY fsphAddFileFind32) (void *plist, FILEFINDBUF3 *pFindBuf, void *data, ULONG dataLength, ULONG flags);
    void (PNDENTRY fsphInitFileFindBuf3) (FILEFINDBUF3 *pbuf, FILESTATUS3 *pstat, char *name);

    int (PNDENTRY fsphWildMatch) (const CHAR *pat, const CHAR *str, int ignorecase);
    int (PNDENTRY fsphAttrMatch) (ULONG pat, ULONG attr);

    int (PNDENTRY fsphSetResourceData) (void *plist, void *data, ULONG dataLength);
    int (PNDENTRY fsphGetResourceData) (NDFILEINFOL *pfi, NDDATABUF *pdata, int expectedSize);
    int (PNDENTRY fsphGetFileInfoData) (NDFILEINFOL *pfi, NDDATABUF *pdata, int expectedSize);

    NDPATHELEMENT * (PNDENTRY fsphPathElemNext) (int index, NDPATHELEMENT *pel);
    NDPATHELEMENT * (PNDENTRY fsphPathElemPrev) (int index, NDPATHELEMENT *pel);
    NDPATHELEMENT * (PNDENTRY fsphNameElem) (int index);
    int (PNDENTRY fsphIsLastElem) (int index, NDPATHELEMENT *pel);

    int (PNDENTRY fsphUpperCase) (CHAR *str);
    int (PNDENTRY fsphUpperCase2) (CHAR *str, int len);

    int (PNDENTRY fsphGEA16to32) (GEALIST *fpGEAList, GEA2LIST *fpGEA2List);
    int (PNDENTRY fsphGEA32to16) (GEA2LIST *fpGEA2List, GEALIST *fpGEAList);
    int (PNDENTRY fsphFEA16to32) (FEALIST *fpFEAList, FEA2LIST *fpFEA2List);
    int (PNDENTRY fsphFEA32to16) (FEA2LIST *fpFEA2List, FEALIST *fpFEAList);

    int (PNDENTRY fsphGetCurrentDateTime) (FDATE *pDate, FTIME *pTime);
    void (PNDENTRY fsphUnixTimeToDosDate) (ULONG time, FDATE *pd, FTIME *pt);
    void (PNDENTRY fsphDosDateToUnixTime) (FDATE d, FTIME t, ULONG *ptime);
    void (PNDENTRY fsphTimeUnix2TimeDate) (ULONG time, NDTIMEDATE *ptd);
    void (PNDENTRY fsphTimeTimeDate2Unix) (NDTIMEDATE *ptd, ULONG *ptime);
    int (PNDENTRY fsphTimeCreateHandle) (NDTIMEHANDLE *phandle, const CHAR *pTZ);
    void (PNDENTRY fsphTimeDeleteHandle) (NDTIMEHANDLE handle);
    void (PNDENTRY fsphTimeLocal2GMT) (NDTIMEHANDLE handle, NDTIMEDATE *plocal, NDTIMEDATE *pgmt);
    void (PNDENTRY fsphTimeGMT2Local) (NDTIMEHANDLE handle, NDTIMEDATE *pgmt, NDTIMEDATE *plocal);
    
    INT    (PNDENTRY fsphCharIsDBCS) (CHAR c);
    USHORT (PNDENTRY fsphLoadChar) (CHAR **pps);
    CHAR * (PNDENTRY fsphCopyChar) (CHAR **pdst, CHAR *src);
    CHAR * (PNDENTRY fsphStrChr) (const CHAR *pStr, CHAR c);
    CHAR * (PNDENTRY fsphStrRChr) (const CHAR *pStr, CHAR c);
    INT    (PNDENTRY fsphStrCmp) (const CHAR *s1, const CHAR *s2);
    INT    (PNDENTRY fsphStrNCmp) (const CHAR *s1, const CHAR *s2, INT n);
    INT    (PNDENTRY fsphStrICmp) (const CHAR *s1, const CHAR *s2);
    INT    (PNDENTRY fsphStrNICmp) (const CHAR *s1, const CHAR *s2, INT n);
    
    INT (PNDENTRY fsphQueryFilePtr) (LONGLONG *pllFilePtr);
    INT (PNDENTRY fsphQueryFileSize) (LONGLONG *pllFileSize);
    INT (PNDENTRY fsphSetFilePtr) (LONGLONG llFileSize, LONGLONG llCurrent, LONGLONG lOffset, ULONG ulMethod, LONGLONG *pllActual);
    
    INT (PNDENTRY fsphSetAttrPattern) (void *plist, ULONG ulAttribute);
    
    ULONG (PNDENTRY fsphQueryCurrentCp) (void);
    
    CHAR (PNDENTRY fsphCpTranslateChar) (ULONG idCpSource, CHAR cSource, ULONG idCpDest);
    BOOL (PNDENTRY fsphCpTranslateString) (ULONG idCpSource, const CHAR *pszSource, ULONG idCpDest, ULONG cbLenDest, CHAR *pszDest);
    
    BOOL (PNDENTRY fsphAsciiToUnicode) (ULONG idCpSource, PSZ pszSource, ULONG cbLenDest, USHORT *pDest);
    BOOL (PNDENTRY fsphUnicodeToAscii) (USHORT *pSource, ULONG idCpDest, ULONG cbLenDest, CHAR *pDest);
    
    INT (PNDENTRY fsphLogOpen) (NDLOGHANDLE *phandle, ULONG loglevel, const char *logfile, const char *msgfile);
    void (PNDENTRY fsphLogMsg) (NDLOGHANDLE handle, ULONG ll, CHAR *msg, INT argc, ...);
    void (PNDENTRY fsphLogClose) (NDLOGHANDLE handle);

    INT (PNDENTRY fsphSetUlongProperty) (NDPROPERTYHANDLE handle, const CHAR *name, const ULONG value);

    void (PNDENTRY fsphLogMsg2) (NDLOGHANDLE handle, ULONG ll, CHAR *msg, INT argc, CHAR **args);

    void * (PNDENTRY fsphAllocMem) (ULONG size);
    void * (PNDENTRY fsphReallocMem) (void *p, ULONG size);
    void (PNDENTRY fsphFreeMem) (void *p);

    void (PNDENTRY fsphUlong2String) (ULONG number, CHAR *string, INT n, INT base);
    void (PNDENTRY fsphLong2String) (LONG number, CHAR *string, INT n, INT base);
    INT (PNDENTRY fsphString2Ulong) (const CHAR *string, CHAR **pstring2, ULONG *pvalue, INT base);
    INT (PNDENTRY fsph_vsnprintf) (CHAR *buf, INT n, const CHAR *fmt, CHAR **args);
    INT (PNDENTRY fsph_snprintf) (CHAR *buf, INT n, const CHAR *fmt, ...);
    
    INT (PNDENTRY fsphCreateMutex) (NDMUTEX *pmutex);
    INT (PNDENTRY fsphRequestMutex) (NDMUTEX mutex, ULONG ulTimeout);
    INT (PNDENTRY fsphReleaseMutex) (NDMUTEX mutex);
    void (PNDENTRY fsphCloseMutex) (NDMUTEX mutex);

    INT (PNDENTRY fsphQueryFileStatus) (FILESTATUS3L *pstat);
 
    INT (PNDENTRY fsphUniStrLen) (USHORT *str);
    
    INT (PNDENTRY fsphQueryFileOpenMode) (ULONG *pulOpenMode);
    
    int (PNDENTRY fsphAddFile32L) (void *plist, FILESTATUS3L *pstat, char *name, ULONG namelen, void *data, ULONG dataLength, ULONG flags);
    
    USHORT (PNDENTRY fsphGetChar) (const char *p);
    const char * (PNDENTRY fsphNextChar) (const char *p);
    
    INT (PNDENTRY fsphString2longlong) (const CHAR *string, CHAR **pstring2, long long *pvalue, INT base);
    
    int (PNDENTRY fsphTimeDateToUnixTime) (const NDTIMEDATE *ptimedate, unsigned long *ptimelow, unsigned long *ptimehigh, int isgmt, int togmt);

    int (PNDENTRY fsphGetCurrentDateTime2) (NDTIMEDATE *ptimedate);
} PLUGINHELPERTABLE2L;

#pragma pack()

// flags for NdpRsrcQueryInfo pulFlags parameter
#define ND_RSRC_AVAILABLE (1)

// return codes for NdpRsrcCompare
#define ND_RSRC_EQUAL     (0)
#define ND_RSRC_DIFFERENT (1)


// Plugin Entry points
#ifdef NDPL_LARGEFILES
#define PLUGINHELPERTABLE2 PLUGINHELPERTABLE2L
#define NDFILEINFO NDFILEINFOL
#endif

extern const char *NdpTypes[];
extern const NDPROPERTYINFO *NdpPropertiesInfo[];
extern const unsigned long NdpAttribute;
int NDENTRY NdpPluginLoad (PLUGINHELPERTABLE2 *pPHT2);
int NDENTRY NdpPluginFree (void);
            
/* resource functions */
int NDENTRY NdpMountResource (HRESOURCE *presource, int type, NDPROPERTYHANDLE properties);
int NDENTRY NdpFreeResource (HRESOURCE resource);
int NDENTRY NdpCreateConnection (HRESOURCE resource, HCONNECTION *pconn);
int NDENTRY NdpFreeConnection (HCONNECTION conn);

int NDENTRY NdpRsrcCompare (HRESOURCE resource, HRESOURCE resource2);
int NDENTRY NdpRsrcUpdate (HRESOURCE resource, HRESOURCE resource2);

int NDENTRY NdpRsrcQueryInfo (HRESOURCE resource, ULONG *pulFlags, void *pdata, ULONG insize, ULONG *poutlen);
int NDENTRY NdpRsrcQueryFSAttach (HRESOURCE resource, void *in, ULONG insize, PULONG poutlen);
int NDENTRY NdpRsrcQueryFSAllocate (HRESOURCE resource, NDFSALLOCATE *pfsa);
      
/* path functions */
int NDENTRY NdpQueryPathInfo (HCONNECTION conn, void *plist, char *szPath);
int NDENTRY NdpDeletePathInfo (HRESOURCE resource, NDFILEINFO *pfi);
int NDENTRY NdpDiscardResourceData (HRESOURCE resource, NDDATABUF *pdatabuf);
int NDENTRY NdpRefresh (HCONNECTION conn, char *path, int tree);
int NDENTRY NdpSetPathInfo (HCONNECTION conn, NDFILEINFO *pfi, char *szPathName);
int NDENTRY NdpEAQuery (HCONNECTION conn, GEALIST *pGEAList, NDFILEINFO *pfi, FEALIST *pFEAList);
int NDENTRY NdpEASet (HCONNECTION conn, FEALIST *pFEAList, NDFILEINFO *pfi);
int NDENTRY NdpEASize (HCONNECTION conn, NDFILEINFO *pfi, ULONG *pulEASize);
int NDENTRY NdpSetCurrentDir (HCONNECTION conn, NDFILEINFO *pfi, char *szPath);
int NDENTRY NdpCopy (HCONNECTION conn, NDFILEINFO *pfiDst, char *szDst, NDFILEINFO *pfiSrc, char *szSrc, ULONG ulOption);
int NDENTRY NdpCopy2 (HCONNECTION conn, HRESOURCE resDst, NDFILEINFO *pfiDst, char *szDst, NDFILEINFO *pfiSrc, char *szSrc, ULONG ulOption);
int NDENTRY NdpForceDelete (HCONNECTION conn, NDFILEINFO *pfi, char *szFile);
int NDENTRY NdpCreateDir (HCONNECTION conn, NDFILEINFO *pfiparent, char *szDirName, FEALIST *pFEAList);
int NDENTRY NdpFindStart (HCONNECTION conn, void *pList, NDFILEINFO *pfiparent, char *szPath, ULONG ulAttribute);
int NDENTRY NdpMove (HCONNECTION conn, NDFILEINFO *pfiDst, char *szDst, NDFILEINFO *pfiSrc, char *szSrc);
int NDENTRY NdpMove2 (HCONNECTION conn, HRESOURCE resDst, NDFILEINFO *pfiDst, char *szDst, NDFILEINFO *pfiSrc, char *szSrc);
int NDENTRY NdpChangeCase (HCONNECTION conn, char *szDst, NDFILEINFO *pfiSrc, char *szSrc, char *szNewName, ULONG ulNameLen);
int NDENTRY NdpRename (HCONNECTION conn, char *szDst, NDFILEINFO *pfiSrc, char *szSrc, char *szNewName, ULONG ulNameLen);
int NDENTRY NdpOpenExisting (HCONNECTION conn, NDFILEINFO *pfi, NDFILEHANDLE *phandle, char *szFileName, ULONG ulOpenMode, USHORT *pfNeedEA);
int NDENTRY NdpOpenReplace (HCONNECTION conn, NDFILEINFO *pfi, NDFILEHANDLE *phandle, char *szFileName, ULONG ulSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList);
int NDENTRY NdpOpenCreate (HCONNECTION conn, NDFILEINFO *pfiparent, NDFILEHANDLE *phandle, char *szFileName, ULONG ulSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList);
int NDENTRY NdpDeleteDir (HCONNECTION conn, NDFILEINFO *pfi, char *szDir);
int NDENTRY NdpFlush (HRESOURCE resource);
int NDENTRY NdpSetFileAttribute (HCONNECTION conn, NDFILEINFO *pfi, char *szFileName, USHORT usAttr);
int NDENTRY NdpIOCTL (int type, HRESOURCE resource, char *path, int function,
                       void *in, ULONG insize, PULONG poutlen);
                                      
/* file functions */
int NDENTRY NdpFileQueryInfo (HCONNECTION conn, NDFILEHANDLE handle, void *plist);
int NDENTRY NdpFileEAQuery (HCONNECTION conn, NDFILEHANDLE handle, GEALIST *pGEAList, FEALIST *pFEAList);
int NDENTRY NdpFileEASet (HCONNECTION conn, NDFILEHANDLE handle, FEALIST *pFEAList);
int NDENTRY NdpFileEASize (HCONNECTION conn, NDFILEHANDLE handle, ULONG *pulEASize);
int NDENTRY NdpFileSetInfo (HCONNECTION conn, NDFILEHANDLE handle, NDFILEINFO *pfi);
int NDENTRY NdpFileSetFilePtr (HCONNECTION conn, NDFILEHANDLE handle, LONG lOffset, ULONG ulMethod, ULONG *pulActual);
int NDENTRY NdpFileClose (HCONNECTION conn, NDFILEHANDLE handle);
int NDENTRY NdpFileCommit (HCONNECTION conn, NDFILEHANDLE handle);
int NDENTRY NdpFileNewSize (HCONNECTION conn, NDFILEHANDLE handle, ULONG ulLen);
int NDENTRY NdpFileRead (HCONNECTION conn, NDFILEHANDLE handle, void *pBuffer, ULONG ulRead, ULONG *pulActual);
int NDENTRY NdpFileWrite (HCONNECTION conn, NDFILEHANDLE handle, void *pBuffer, ULONG ulWrite, ULONG *pulActual);

// large file support (>2Gb) 
#ifdef NDPL_LARGEFILES
int NDENTRY NdpOpenReplaceL(HCONNECTION conn, NDFILEINFO *pfi, NDFILEHANDLE *phandle, char *szFileName, LONGLONG llSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList);
int NDENTRY NdpOpenCreateL(HCONNECTION conn, NDFILEINFO *pfiparent, NDFILEHANDLE *phandle, char *szFileName, LONGLONG llSize, ULONG ulOpenMode, ULONG ulAttribute, FEALIST *pFEAList);
int NDENTRY NdpFileSetFilePtrL(HCONNECTION conn, NDFILEHANDLE handle, LONGLONG llOffset, ULONG ulMethod, LONGLONG *pllActual);
int NDENTRY NdpFileNewSizeL(HCONNECTION conn, NDFILEHANDLE handle, LONGLONG llLen);
#endif

#ifdef __cplusplus
    }
#endif

#endif /* __EXTPL2__H */
