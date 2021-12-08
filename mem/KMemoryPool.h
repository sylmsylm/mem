#ifndef _KMEMORY_POOL_H_
#define _KMEMORY_POOL_H_

#define MEMORY_POOL_ALIGN           (8)
#define MEMORY_POOL_MIN_BYTES       (8)
#define MEMORY_POOL_MAX_BYTES       (128)
#define MEMORY_POOL_LISTS_COUNT     ((MEMORY_POOL_MAX_BYTES) / (MEMORY_POOL_ALIGN))
#define MEMORY_POOL_OBJECTS_COUNT   (20)

class KMemoryPool
{
public:
    KMemoryPool();
    ~KMemoryPool();

    void*   Alloc(size_t uDataLen);
    void    DeAlloc(void* pData, size_t uDataLen);
    void*   ReAlloc(void* pData, size_t uOldDataLen, size_t uNewDataLen);

private:
    size_t  _ListIndex(size_t uDataLen);
    size_t  _RoundUp(size_t uDataLen);

    void*   _ReFill(size_t uDataLen);
    char*   _ChunkAlloc(size_t uDataLen, int& nObject);

    struct OBJECT_LIST
    {
        OBJECT_LIST*    pNextObject;
    };

    char*   m_pStart;
    char*   m_pEnd;
    size_t  m_uHeap;

    OBJECT_LIST* volatile m_apObjectList[MEMORY_POOL_LISTS_COUNT];
};

#endif
