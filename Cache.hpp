#pragma once

#include "Pool.hpp"

class Cache {

    protected:
        Pool const * m_pool;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Map of buffer ID to Buffer objects in cache.
        std::map<int64_t, Item*> _cache;

        virtual Item * searchPool      ( int64_t itemID ) = 0;
        virtual Item * searchCache     ( int64_t itemID ) = 0;
        virtual void loadItemFromPool( Item & item ) = 0;
        virtual void addItemToCache  ( Item & item ) = 0;
        virtual void touchItem       ( Item & item ) = 0;

    public:
        Cache() = default;

        virtual Item *getItem( int64_t itemID ) = 0;
};