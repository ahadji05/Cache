/*
* MIT License

* Copyright (c) 2025 Andreas

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once

#include "Pool.hpp"

template<
        typename KeyType = int64_t ,
        template <typename... Args> class MapType = std::map
        >
class Cache {

    protected:
        using item_type = Item< KeyType >;

        // Pointer to the associated pool (non-owning).
        Pool<KeyType, MapType> const * m_pool;

        // Map of items currently in cache.
        MapType<KeyType, item_type*> _cache;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Keep track of the number of cache hits.
        size_t _cacheHits = 0;
        size_t _cacheEvictions = 0;

    public:
        Cache();
        virtual ~Cache() {}

        Pool<KeyType, MapType> const * const getPool() const;
        MapType<KeyType, item_type*> const& getCache() const;
        size_t getCacheSizeInNumItems() const;
        size_t getCacheHits() const;
        size_t getCacheEvictions() const;

        virtual item_type *getItem( KeyType itemID ) = 0;
};

/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType, template <typename... Args> class MapType >
Cache<KeyType, MapType>::Cache()
{
}

template< typename KeyType, template <typename... Args> class MapType >
Pool<KeyType, MapType> const * const
Cache<KeyType, MapType>::getPool() const
{
    return m_pool;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType<KeyType, typename Cache<KeyType, MapType>::item_type*> const&
Cache<KeyType, MapType>::getCache() const
{
    return _cache;
}

template< typename KeyType, template <typename... Args> class MapType >
size_t
Cache<KeyType, MapType>::getCacheSizeInNumItems() const
{
    return _cacheSizeInNumItems;
}

template< typename KeyType, template <typename... Args> class MapType >
size_t
Cache<KeyType, MapType>::getCacheHits() const
{
    return _cacheHits;
}

template< typename KeyType, template <typename... Args> class MapType >
size_t
Cache<KeyType, MapType>::getCacheEvictions() const
{
    return _cacheEvictions;
}
