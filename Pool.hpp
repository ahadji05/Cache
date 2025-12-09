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

        MapType< KeyType, unique_ptr_type > m_pool;

    public:
        Pool(){}
        ~Pool(){}

        size_t getPoolSize() const {
            return m_pool.size();
        }

        void addItem( unique_ptr_type item ){
            KeyType id = item->getID();

            if ( m_pool.count( id ) )
                throw std::runtime_error( "The item with ID "+std::to_string(id)+" already exists in the pool!" );

            pair_type p = { id, std::move( item ) };

            m_pool.insert( std::move( p ) );
        }

        item_type * getItem( KeyType id ) const {
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
};