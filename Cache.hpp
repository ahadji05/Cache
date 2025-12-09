#pragma once

#include "Pool.hpp"

template<
        typename KeyType = int64_t ,
        template <typename... Args> class MapType = std::map
        >
class Cache {

    protected:
        using item_type = Item< KeyType >;

        Pool<KeyType, MapType> const * m_pool;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Map of ...
        MapType<KeyType, item_type*> _cache;

        virtual item_type * searchPool      ( KeyType itemID ) = 0;
        virtual item_type * searchCache     ( KeyType itemID ) = 0;
        virtual void loadItemFromPool( item_type & item ) = 0;
        virtual void addItemToCache  ( item_type & item ) = 0;
        virtual void touchItem       ( item_type & item ) = 0;

    public:
        Cache() = default;

        virtual item_type *getItem( KeyType itemID ) = 0;
};