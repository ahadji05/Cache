#pragma once

#include "Pool.hpp"

template<
        typename KeyType = int64_t ,
        template <typename... Args> class MapType = std::map
        >
class Cache {

    protected:
        using item_type = Item< KeyType >;

        // Pointer to the associated pool.
        Pool<KeyType, MapType> const * m_pool;

        // Map of items currently in cache.
        MapType<KeyType, item_type*> _cache;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Keep track of the number of cache hits.
        size_t _cacheHits = 0;
        size_t _cacheEvictions = 0;

        virtual item_type * searchPool      ( KeyType itemID ) = 0;
        virtual item_type * searchCache     ( KeyType itemID ) = 0;
        virtual void loadItemFromPool( item_type & item ) = 0;
        virtual void addItemToCache  ( item_type & item ) = 0;
        virtual void touchItem       ( item_type & item ) = 0;

    public:
        Cache() = default;

        Pool<KeyType, MapType> const * const getPool() const {
            return m_pool;
        }

        MapType<KeyType, item_type*> const& getCache() const {
            return _cache;
        }

        size_t getCacheSizeInNumItems() const {
            return _cacheSizeInNumItems;
        }

        size_t getCacheHits() const {
            return _cacheHits;
        }

        size_t getCacheEvictions() const {
            return _cacheEvictions;
        }

        virtual item_type *getItem( KeyType itemID ) = 0;
};