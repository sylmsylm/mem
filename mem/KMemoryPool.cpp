#include <malloc.h>

#include "KMemoryPool.h"

KMemoryPool::KMemoryPool()
{
    m_uHeap     = 0;
    m_pEnd      = NULL;
    m_pStart    = NULL;

    for (int i = 0; i < MEMORY_POOL_LISTS_COUNT; ++i)
    {
        m_apObjectList[i] = NULL;
    }
}

KMemoryPool::~KMemoryPool() {}

void* KMemoryPool::Alloc(size_t uDataLen)
{
    void*                   pResult         = NULL;
    OBJECT_LIST* volatile*  ppListIndex     = NULL;

    if (uDataLen > MEMORY_POOL_MAX_BYTES)
    {
        return malloc(uDataLen);
    }

    ppListIndex = m_apObjectList + _ListIndex(uDataLen);
    if (!(*ppListIndex))
    {
        size_t uRoundDataLen = _RoundUp(uDataLen);
        return _ReFill(uRoundDataLen);
    }

    pResult = *ppListIndex;
    *ppListIndex = ((OBJECT_LIST*)pResult)->pNextObject;

    return pResult;
}

void KMemoryPool::DeAlloc(void* pData, size_t uDataLen)
{
    OBJECT_LIST*            pObject         = NULL;
    OBJECT_LIST* volatile*  ppListIndex     = NULL;

    if (!pData)
    {
        return;
    }

    if (uDataLen > MEMORY_POOL_MAX_BYTES)
    {
        free(pData);
        pData = NULL;
        return;
    }

    ppListIndex = m_apObjectList + _ListIndex(uDataLen);

    pObject = (OBJECT_LIST*)pData;
    pObject->pNextObject = *ppListIndex;
    *ppListIndex = pObject;

    return;
}

void* KMemoryPool::ReAlloc(void* pData, size_t uOldDataLen, size_t uNewDataLen)
{
    void* pResult = NULL;

    if (!pData)
    {
        return Alloc(uNewDataLen);
    }

    if (uOldDataLen > MEMORY_POOL_MAX_BYTES && uNewDataLen > MEMORY_POOL_MAX_BYTES)
    {
        return realloc(pData, uNewDataLen);
    }

    DeAlloc(pData, uOldDataLen);

    return Alloc(uNewDataLen);
}

size_t KMemoryPool::_ListIndex(size_t uDataLen)
{
    return ((uDataLen + MEMORY_POOL_ALIGN - 1) / MEMORY_POOL_ALIGN - 1);
}

size_t KMemoryPool::_RoundUp(size_t uDataLen)
{
    return ((uDataLen + MEMORY_POOL_ALIGN - 1) & (~(MEMORY_POOL_ALIGN - 1)));
}

void* KMemoryPool::_ReFill(size_t uDataLen)
{
    char*                   pRetChunk       = NULL;
    int                     nObjectCount    = MEMORY_POOL_OBJECTS_COUNT;
    OBJECT_LIST*            pCurrentObject  = NULL;
    OBJECT_LIST*            pNextObject     = NULL;
    OBJECT_LIST* volatile*  ppListIndex     = NULL;

    pRetChunk = _ChunkAlloc(uDataLen, nObjectCount);
    if (!pRetChunk)
    {
        return NULL;
    }

    if (nObjectCount == 1)
    {
        return (void*)pRetChunk;
    }

    ppListIndex = m_apObjectList + _ListIndex(uDataLen);

    pNextObject = (OBJECT_LIST*)(pRetChunk + uDataLen);
    *ppListIndex = pNextObject;

    for (int i = 1; ; ++i)
    {
        pCurrentObject = pNextObject;
        pNextObject = (OBJECT_LIST*)((char*)pNextObject + uDataLen);

        if (i == nObjectCount - 1)
        {
            pCurrentObject->pNextObject = NULL;
            break;
        }
        pCurrentObject->pNextObject = pNextObject;
    }

    return (void*)pRetChunk;
}

char* KMemoryPool::_ChunkAlloc(size_t uDataLen, int& nObjectCount)
{
    char*                   pResult         = NULL;
    size_t                  uDataNeed       = uDataLen * nObjectCount;
    size_t                  uDataLeft       = m_pEnd - m_pStart;
    size_t                  uDataNewMalloc  = 2 * uDataNeed + _RoundUp(m_uHeap >> 4);
    OBJECT_LIST*            pTempObject     = NULL;
    OBJECT_LIST* volatile*  ppListIndex     = NULL;

    if (uDataLeft >= uDataNeed)
    {
        pResult = m_pStart;
        m_pStart += uDataNeed;
        return pResult;
    }

    if (uDataLeft >= uDataLen)
    {
        nObjectCount = (int)(uDataLeft / uDataLen);
        pResult = m_pStart;
        m_pStart += nObjectCount * uDataLen;
        return pResult;
    }

    if (uDataLeft > 0)
    {
        ppListIndex = m_apObjectList + _ListIndex(uDataLeft);
        ((OBJECT_LIST*)m_pStart)->pNextObject = *ppListIndex;
        *ppListIndex = (OBJECT_LIST*)m_pStart;
    }

    m_pStart = (char*)malloc(uDataNewMalloc);
    if (!m_pStart)
    {
        for (int i = (int)uDataLen; i <= MEMORY_POOL_MAX_BYTES; i += MEMORY_POOL_ALIGN)
        {
            ppListIndex = m_apObjectList + _ListIndex(i);
            pTempObject = *ppListIndex;
            if (pTempObject)
            {
                *ppListIndex = pTempObject->pNextObject;
                m_pStart = (char*)pTempObject;
                m_pEnd = m_pStart + i;
                return _ChunkAlloc(uDataLen, nObjectCount);
            }
        }

        m_pEnd = NULL;
        nObjectCount = 0;
        return NULL;
    }

    m_uHeap += uDataNewMalloc;
    m_pEnd = m_pStart + uDataNewMalloc;

    return _ChunkAlloc(uDataLen, nObjectCount);
}