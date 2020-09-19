#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define NDPL_LARGEFILES
#define INCL_LONGLONG
#include <ndextpl2.h>

#include "smbwrp.h"

extern PLUGINHELPERTABLE2L *ph;

/*
 * Static helpers.
 */
static int lockCreate (NDMUTEX *pMutex)
{
    return ph->fsphCreateMutex (pMutex);
}
        
static void lockDelete (NDMUTEX mutex)
{
    ph->fsphCloseMutex (mutex);
}
        
static int lockRequest (NDMUTEX mutex)
{
    return ph->fsphRequestMutex (mutex, 10000);
}

void lockRelease (NDMUTEX mutex)
{
    ph->fsphReleaseMutex (mutex);
}

static ULONG Hash (const char *pData, ULONG ulLen) 
{
    ULONG hash = 0ul, g;
    static ULONG ulHashModule = 30031ul;
    
    const char *p;
    int i;
    for( p = pData, i = 0; i < ulLen; i++, p++ ) 
    {
        hash = ( hash << 4 ) + (*p);
        g = hash & 0xf0000000ul;
        if ( g ) 
        {
            hash ^= ( g >> 24 );
            hash ^= g;
        }
    }
    return ( hash % ulHashModule );
}

static unsigned long timesec (void) 
{
    ULONG ul = 0;
    DosQuerySysInfo (QSV_TIME_LOW, QSV_TIME_LOW, &ul, sizeof (ul));
    return ul;
}

static unsigned long computehash (const char *s)
{
    return s? Hash ((char *)s, strlen (s)): 0;
}



/*
 * An entry in the directory cache contains one directory listing.
 */
typedef struct DirectoryCacheEntry
{
    struct DirectoryCacheEntry *pNext;
    struct DirectoryCacheEntry *pPrev;

    struct DirectoryCache *pdc;
    
    smbwrp_fileinfo *aInfos;
    int cInfos;
    int cInfosAllocated;

    char *pszPath;
    ULONG ulHash;
    ULONG ulLastUpdateTime;
    int fInvalid;
} DirectoryCacheEntry;

typedef struct DirectoryCache
{
    NDMUTEX mutex;

    DirectoryCacheEntry *pEntriesHead;
    DirectoryCacheEntry *pEntriesTail;
    int cEntries;

    int fEnabled;
    unsigned long ulExpirationTime;
    int cMaxEntries;
} DirectoryCache;

enum {
    CacheFault = 0,
    CacheOk = 1
};
        

static int dceCreate (DirectoryCacheEntry **ppdce, DirectoryCache *pdc, const char *path, int cFiles)
{
    DirectoryCacheEntry *pdce = (DirectoryCacheEntry *)malloc(sizeof(DirectoryCacheEntry));

    if (!pdce)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pdce->aInfos = (smbwrp_fileinfo *)malloc(cFiles * sizeof (smbwrp_fileinfo));
    if (!pdce->aInfos)
    {
        free (pdce);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pdce->cInfos = 0;
    pdce->cInfosAllocated = cFiles;
    pdce->ulHash = computehash (path);
    pdce->ulLastUpdateTime = 0;
    pdce->pdc = pdc;
    pdce->fInvalid = 0;

    int l = strlen (path);
    pdce->pszPath = (char *)malloc (l + 1);
    memcpy (pdce->pszPath, path, l + 1);
    
    pdce->pNext = NULL;
    pdce->pPrev = NULL;

    *ppdce = pdce;
    
    return NO_ERROR;
}

static void dceDelete (DirectoryCacheEntry *pdce)
{
    free(pdce->aInfos);
    free(pdce->pszPath);
    free(pdce);
}
                
static void dceClear (DirectoryCacheEntry *pdce)
{
    pdce->cInfos = 0;
}

static void dceWriteEntry (DirectoryCacheEntry *pdce, const smbwrp_fileinfo *finfo)
{
    if (pdce->cInfos >= pdce->cInfosAllocated)
    {
        /* 50% increase of the buffer, but at least 1 more. */
        int cNewAllocated = pdce->cInfosAllocated + pdce->cInfosAllocated / 2 + 1;

        smbwrp_fileinfo *pNewInfos = (smbwrp_fileinfo *)realloc(pdce->aInfos,
                                                                cNewAllocated * sizeof (smbwrp_fileinfo));

        debuglocal(9, "dceWriteEntry: [%s] realloc %d -> %d\n", pdce->pszPath, pdce->cInfosAllocated, cNewAllocated);
        if (pNewInfos)
        {
            pdce->cInfosAllocated = cNewAllocated;
            pdce->aInfos = pNewInfos;
        }
        else
        {
            /* Mark the entry as invalid. The entry will be deleted in dircache_write_end */
            pdce->fInvalid = 1;
            return;
        }
    }

    pdce->aInfos[pdce->cInfos] = *finfo;
    pdce->cInfos++;
}

static void dceAdjust (DirectoryCacheEntry *pdce)
{
    /* If the entry has too many preallocated info structures, adjust the memory allocation */
    int cFree = pdce->cInfosAllocated - pdce->cInfos;

    if (cFree > pdce->cInfos / 2)
    {
        /* More than 50% is free. Make the free space 2 times smaller. */
        int cNewAllocated = pdce->cInfos + cFree / 2;

        smbwrp_fileinfo *pNewInfos = (smbwrp_fileinfo *)realloc(pdce->aInfos,
                                                                cNewAllocated * sizeof (smbwrp_fileinfo));

        debuglocal(9, "dceAdjust: [%s] realloc %d -> %d\n", pdce->pszPath, pdce->cInfosAllocated, cNewAllocated);
        if (pNewInfos)
        {
            pdce->cInfosAllocated = cNewAllocated;
            pdce->aInfos = pNewInfos;
        }
        else
        {
            /* Ignore. The old buffer remains. */
        }
    }
}

static int dcePathEqualsTo (DirectoryCacheEntry *pdce, const char *path)
{
    debuglocal(9, "dcePathEqualTo: [%s] [%s]\n", path, pdce->pszPath);
    return ph->fsphStrICmp (path, pdce->pszPath) == 0; 
}

static int dcePathPrefixedWith (DirectoryCacheEntry *pdce, const char *path)
{ 
    return ph->fsphStrNICmp (path, pdce->pszPath, strlen (path)) == 0; 
}

static int dceExpired (DirectoryCacheEntry *pdce, unsigned long ulExpirationTime)
{
    return (timesec () - pdce->ulLastUpdateTime >= ulExpirationTime);
}
                
static struct DirectoryCacheEntry *dcFindDirCache (struct DirectoryCache *pdc, const char *path, int fPrefix)
{
    unsigned long hash = computehash (path);

    DirectoryCacheEntry *iter = pdc->pEntriesHead;

    debuglocal(9, "findDirCache: [%s]\n", path);

    while (iter != NULL)
    {
        debuglocal(9, "findDirCache: entry [%s]\n", iter->pszPath);
        if (fPrefix)
        {
            if (dcePathPrefixedWith (iter, path))
            {
                break;
            }
        }
        else
        {
            if (    iter->ulHash == hash
                 && dcePathEqualsTo (iter, path)
               )
            {
                break;
            }
        }
        iter = iter->pNext;
    }

    debuglocal(9, "findDirCache: %p\n", iter);

    return iter;
}

static int dcCreate (struct DirectoryCache **ppdc)
{
    int rc;

    DirectoryCache *pdc = (DirectoryCache *)malloc(sizeof (DirectoryCache));
    
    if (!pdc)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    rc = lockCreate (&pdc->mutex);

    if (rc != NO_ERROR)
    {
        free(pdc);
        return rc;
    }

    pdc->pEntriesHead = NULL;
    pdc->pEntriesTail = NULL;
    pdc->cEntries = 0;

    pdc->fEnabled = 0;
    pdc->ulExpirationTime = -1;
    
    *ppdc = pdc;
    
    return NO_ERROR;
}

static void dcDelete (struct DirectoryCache *pdc)
{
    DirectoryCacheEntry *iter = pdc->pEntriesHead;

    while (iter != NULL)
    {
        DirectoryCacheEntry *next = iter->pNext;

        dceDelete (iter);
        
        iter = next;
    }

    lockDelete (pdc->mutex);
    free (pdc);
}

static void dcRemoveEntry (DirectoryCacheEntry *pdce)
{
    DirectoryCache *pdc = pdce->pdc;

    if (pdce->pNext)
    {
        pdce->pNext->pPrev = pdce->pPrev;
    }
    else
    {
        pdc->pEntriesTail = pdce->pPrev;
    }

    if (pdce->pPrev)
    {
        pdce->pPrev->pNext = pdce->pNext;
    }
    else
    {
        pdc->pEntriesHead = pdce->pNext;
    }

    pdce->pNext = NULL;
    pdce->pPrev = NULL;
   
    pdc->cEntries--;
}

static void dcInsertEntry (DirectoryCacheEntry *pdce)
{
    DirectoryCache *pdc = pdce->pdc;

    pdce->pNext = pdc->pEntriesHead;

    if (pdc->pEntriesHead)
    {
        pdc->pEntriesHead->pPrev = pdce;
    }
    else
    {
        pdc->pEntriesTail = pdce;
    }
    
    pdc->pEntriesHead = pdce;
    pdc->cEntries++;
}

        
static void dcEnable (DirectoryCache *pdc, int fEnable, unsigned long ulExpirationTime, int cMaxEntries)
{
    pdc->fEnabled = fEnable;
    pdc->ulExpirationTime = ulExpirationTime;
    pdc->cMaxEntries = cMaxEntries;
};

static int dcExistsInCache (DirectoryCache *pdc, const char *path)
{
    int rc = CacheFault;
        
    if (pdc->fEnabled)
    {
        if (dcFindDirCache (pdc, path, 0) != NULL)
        {
            rc = CacheOk;
        }
        
        debuglocal(9, "ExistsInCache: [%s], %d\n", path, rc);
    }
    
    return rc;
}
        
static int dcRead (DirectoryCache *pdc, const char *path,
                   PFNADDDIRENTRY fn,
                   filelist_state *state,
                   int *ptotal_received, int fForceCache)
{
    int i;
    
    int rc = CacheFault;
        
    if (pdc->fEnabled)
    {
        DirectoryCacheEntry *pdce;

        pdce = dcFindDirCache (pdc, path, 0);
        
        if (pdce)
        {
            debuglocal(9, "CacheRead: entry %p found for [%s]\n", pdce, path);
            
            if (fForceCache)
            {
                for (i = 0; i < pdce->cInfos; i++)
                {
                    fn ("", &pdce->aInfos[i], path, state);
                }

                rc = CacheOk;
            }
            else
            {
                if (dceExpired (pdce, pdc->ulExpirationTime))
                {
                    debuglocal(9, "CacheRead: expired\n");
                    dcRemoveEntry (pdce);
                    dceDelete (pdce);
                }
                else
                {
                    for (i = 0; i < pdce->cInfos; i++)
                    {
                        fn ("", &pdce->aInfos[i], path, state);
                    }

                    rc = CacheOk;
                }
            }
            
            if (rc == CacheOk)
            {
                *ptotal_received = (int)pdce->cInfos;
            }
        }
        
        debuglocal(9, "CacheRead: [%s], %d\n", path, rc);
    }
    
    return rc;
}

static DirectoryCacheEntry *dcWriteBegin (DirectoryCache *pdc, const char *path, int cFiles)
{
    DirectoryCacheEntry *pdce = NULL;

    if (pdc->fEnabled)
    {
        pdce = dcFindDirCache (pdc, path, 0);
        
        if (!pdce)
        {
            /* Does not exist in the cache yet. */
            dceCreate (&pdce, pdc, path, cFiles);
        }
        else
        {
            /* Discard the listing. */
            dceClear (pdce);
            /* Remove the entry from list. It will be added again in dircache_write_end. */
            dcRemoveEntry (pdce);
        }

        if (pdce)
        {
            pdce->ulLastUpdateTime = timesec ();
        }
        
        debuglocal(9, "CacheWriteBegin: %s\n", path);
    }
    
    return pdce;
}


static void dcInvalidate (DirectoryCache *pdc, const char *path, int fPrefix)
{
    DirectoryCacheEntry *pdce;
    
    debuglocal(9, "CacheInvalidate: [%s]\n", path);

    pdce = dcFindDirCache (pdc, path, fPrefix);

    while (pdce)
    {
        dcRemoveEntry (pdce);
        dceDelete (pdce);
        
        pdce = dcFindDirCache (pdc, path, fPrefix);
    }
}

static int dcFindPath (DirectoryCache *pdc,
                       const char *path,
                       const char *parent,
                       const char *name,
                       smbwrp_fileinfo *finfo,
                       unsigned long *pulAge)
{
    DirectoryCacheEntry *pdce = pdc->pEntriesHead;

    unsigned long hash = computehash (parent);

    debuglocal(9, "dcFindPath: [%s][%s]\n", parent, name);

    while (pdce != NULL)
    {
        debuglocal(9, "dcFindPath: entry [%s]\n", pdce->pszPath);

        if (   pdce->ulHash == hash
            && dcePathEqualsTo (pdce, parent))
        {
            /* This entry should contain the path. */
            int i;
            for (i = 0; i < pdce->cInfos; i++)
            {
                if (ph->fsphStrICmp (pdce->aInfos[i].fname, name) == 0)
                {
                    *finfo = pdce->aInfos[i];
                    *pulAge = timesec () - pdce->ulLastUpdateTime;
                    debuglocal(9, "dircache: FindPath %s found, age %d\n", path, *pulAge);
                    return 1;
                }
            }
        }

        pdce = pdce->pNext;
    }

    debuglocal(9, "dircache: FindPath %s not found\n", path);
    return 0;
}

static int dcCreateDirPath(char *path, int cbPath, filelist_state *state)
{
    /* State contains the original path passed to the plugin (fullpath) and
     * the actual filename mask (dir_mask), which should be used to filter
     * appropriate filenames from the full listing.
     * So the directory name can be constructed by removing the mask from the fullpath.
     */
    int cbDir;
    int cbMask = strlen(state->dir_mask) + 1;
    if (cbMask > cbPath)
    {
        /* This actually should never happen, because mask is a part of path.
         * But still return a failure, better no dircache than a crash.
         */
        return 0;
    }
    cbDir = cbPath - cbMask;
    if (cbDir > 0)
    {
       cbDir--; /* Exclude the slash. */
       memcpy(path, state->fullpath, cbDir);
    }
    path[cbDir] = 0;
    return 1;
}


/*
 *  Public API.
 */

int dircache_create(DirectoryCache **ppdc, unsigned long ulExpirationTime, int cMaxEntries)
{
    int rc;

    debuglocal(9, "dircache_create: %u seconds, %d entries\n", ulExpirationTime, cMaxEntries);

    rc = dcCreate(ppdc);

    if (rc == NO_ERROR)
    {
        dcEnable (*ppdc, 1, ulExpirationTime, cMaxEntries);
    }

    debuglocal(9, "dircache_create: %p, rc = %d\n", *ppdc, rc);
    return rc;
}

void dircache_delete(DirectoryCache *pdc)
{
    debuglocal(9, "dircache_delete: %p\n", pdc);

    if (pdc)
    {
        dcDelete(pdc);
    }
}


int dircache_list_files(PFNADDDIRENTRY fn,
                        filelist_state *state,
                        int *ptotal_received)
{
    int rc;
    DirectoryCache *pdc = state->pConn->pRes->pdc;

    int cbPath = strlen(state->fullpath) + 1;
    char *path = alloca(cbPath);
    if (!dcCreateDirPath(path, cbPath, state))
    {
        return 0;
    }

    debuglocal(9, "dircache_list_files [%s]\n", path);

    if (!pdc)
    {
        return 0;
    }

    lockRequest (pdc->mutex);

    rc = dcRead (pdc, path, fn, state, ptotal_received, 0);

    lockRelease (pdc->mutex);

    return (rc == CacheOk);
}

void *dircache_write_begin(filelist_state *state,
                           int cFiles)
{
    DirectoryCache *pdc = state->pConn->pRes->pdc;
    DirectoryCacheEntry *pdce = NULL;

    int cbPath = strlen(state->fullpath) + 1;
    char *path = alloca(cbPath);
    if (!dcCreateDirPath(path, cbPath, state))
    {
        return NULL;
    }

    debuglocal(9, "dircache_write_begin pdc %p path [%s]\n", pdc, path);

    if (!pdc)
    {
        return NULL;
    }

    lockRequest (pdc->mutex);
    
    if (pdc->cEntries >= pdc->cMaxEntries)
    {
        /* Remove oldest entry. */
        pdce = pdc->pEntriesTail;
        dcRemoveEntry (pdce);
        dceDelete (pdce);
        pdce = NULL;
    }
    
    pdce = dcWriteBegin (pdc, path, cFiles);
    
    lockRelease (pdc->mutex);

    debuglocal(9, "dircache_write_begin returning ctx %p\n", pdce);
    return (void *)pdce;
}

void dircache_write_entry(void *dircachectx, const smbwrp_fileinfo *finfo)
{
    DirectoryCacheEntry *pdce = (DirectoryCacheEntry *)dircachectx;

    debuglocal(9, "dircache_write_entry %p\n", pdce);
    
    if (!pdce)
    {
        return;
    }
    
    lockRequest (pdce->pdc->mutex);
    
    dceWriteEntry (pdce, finfo);
    
    lockRelease (pdce->pdc->mutex);
}

void dircache_write_end(void *dircachectx)
{
    DirectoryCacheEntry *pdce = (DirectoryCacheEntry *)dircachectx;
    DirectoryCache *pdc;

    debuglocal(9, "dircache_write_end: pdce %p cInfos %u\n", pdce,
pdce->cInfos);

    if (!pdce)
    {
        return;
    }
    
    pdc = pdce->pdc;
    
    lockRequest (pdc->mutex);
    
    if (pdce->fInvalid)
    {
        /* Something happened during writing to the entry. Delete it. */
        dceDelete (pdce);
    }
    else
    {
        dceAdjust (pdce);
        dcInsertEntry (pdce);
    }
    
    lockRelease (pdc->mutex);
}

void dircache_invalidate(const char *path,
                         struct DirectoryCache *pdc,
                         int fParent)
{
    debuglocal(9, "dircache_invalidate [%s], parent %d\n", path, fParent);

    if (!pdc)
    {
        return;
    }

    lockRequest (pdc->mutex);
    
    if (fParent)
    {
        int cb = strlen (path) + 1;
        char *p = (char *)alloca(cb);
        memcpy(p, path, cb);
        char *lastSlash = ph->fsphStrRChr(p, '\\');
        if (lastSlash)
        {
            *lastSlash = 0;
            dcInvalidate (pdc, p, 0);
        }
        else
        {
            dcInvalidate (pdc, "", 0);
        }
    }
    else
    {
        dcInvalidate (pdc, path, 0);
    }
    
    lockRelease (pdc->mutex);

    return;
}

int dircache_find_path(struct DirectoryCache *pdc,
                       const char *path,
                       smbwrp_fileinfo *finfo,
                       unsigned long *pulAge)
{
    int cb;
    char *p;
    char *lastSlash;
    int fFound = 0;

    debuglocal(9, "dircache_find_path [%s]\n", path);

    if (!pdc)
    {
        return 0;
    }
    
    if (ph->fsphStrChr(path, '*') || ph->fsphStrChr(path, '?'))
    {
        /* Wildcards are not allowed in the input path. */
        return 0;
    }

    lockRequest (pdc->mutex);

    /* Prepare the parent path. */
    cb = strlen (path) + 1;
    p = (char *)alloca(cb);
    memcpy(p, path, cb);
    lastSlash = ph->fsphStrRChr(p, '\\');
    if (lastSlash)
    {
        *lastSlash = 0;
        fFound = dcFindPath (pdc, path, p, lastSlash + 1, finfo, pulAge);
    }
    else
    {
        /* Find in the root directory. p is the name. */
        fFound = dcFindPath (pdc, path, "", p, finfo, pulAge);
    }
    
    lockRelease (pdc->mutex);

    return fFound;
}
