#include <iostream>
#include <vector>

#include "KMemoryPool.h"


int main() {
    KMemoryPool Pool;
    std::vector<std::pair<void*, size_t>>v;
    v.push_back(std::make_pair(Pool.Alloc(129), 129));

    for (int i = 0; i < 21; ++i)
    {
        v.push_back(std::make_pair(Pool.Alloc(12), 12));
    }

    while (!v.empty())
    {
        Pool.DeAlloc(v.back().first, v.back().second);
        v.pop_back();
    }

    for (int i = 0; i < 21; ++i)
    {
        v.push_back(std::make_pair(Pool.Alloc(7), 7));
    }

    for (int i = 0; i < 10; ++i)
    {
        v.push_back(std::make_pair(Pool.Alloc(16), 16));
    }

    while (!v.empty())
    {
        Pool.DeAlloc(v.back().first, v.back().second);
        v.pop_back();
    }

    return 0;
}