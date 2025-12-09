#pragma once

#include "Pool.hpp"

template< typename KeyType >
class Cache {

    protected:
        Pool<KeyType> const * m_pool;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Map of buffer ID to Buffer objects in cache.
        std::map<KeyType, Item<KeyType>*> _cache;

        virtual Item<KeyType> * searchPool      ( KeyType itemID ) = 0;
        virtual Item<KeyType> * searchCache     ( KeyType itemID ) = 0;
        virtual void loadItemFromPool( Item<KeyType> & item ) = 0;
        virtual void addItemToCache  ( Item<KeyType> & item ) = 0;
        virtual void touchItem       ( Item<KeyType> & item ) = 0;

    public:
        Cache() = default;

        virtual Item<KeyType> *getItem( KeyType itemID ) = 0;
};