#pragma once

#include "Item.hpp"
#include <memory>
#include <map>

template<
        typename KeyType = int64_t ,
        template <typename... Args> class MapType = std::map
        >
class Pool {

    private:
        using item_type       = Item< KeyType >;
        using unique_ptr_type = std::unique_ptr< item_type >;
        using pair_type       = std::pair< KeyType, unique_ptr_type >;

        // Map of pool-items.
        MapType< KeyType, unique_ptr_type > m_pool;

    public:
        Pool();
        ~Pool() noexcept;

        size_t getPoolSize() const;
        void addItem( unique_ptr_type item );
        item_type * getItem( KeyType id ) const;
};


/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType, template <typename... Args> class MapType >
Pool<KeyType, MapType>::Pool()
{
}

template< typename KeyType, template <typename... Args> class MapType >
Pool<KeyType, MapType>::~Pool() noexcept
{
}

template< typename KeyType, template <typename... Args> class MapType >
size_t
Pool<KeyType, MapType>::getPoolSize() const
{
    return m_pool.size();
}

template< typename KeyType, template <typename... Args> class MapType >
void
Pool<KeyType, MapType>::addItem( unique_ptr_type item )
{
    KeyType id = item->getID();

    if ( m_pool.count( id ) )
        throw std::runtime_error( "Item already exists in the pool!" );

    pair_type p = { id, std::move( item ) };

    m_pool.insert( std::move( p ) );
}

template< typename KeyType, template <typename... Args> class MapType >
typename Pool<KeyType, MapType>::item_type *
Pool<KeyType, MapType>::getItem( KeyType id ) const
{
    if ( !m_pool.count( id ) ) {
        #ifdef DEBUG_CACHES
        std::cout << "not found in pool...\n";
        #endif
        return nullptr;
    }

    #ifdef DEBUG_CACHES
    std::cout << "found in pool...\n";
    #endif
    return m_pool.at(id).get();
}