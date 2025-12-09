#pragma once

#include "Item.hpp"
#include <memory>
#include <map>

template< typename KeyType >
class Pool {

    private:
        std::map<KeyType, std::unique_ptr<Item<KeyType>>> m_pool;

    public:
        Pool(){}
        ~Pool(){}

        size_t getPoolSize() const {
            return m_pool.size();
        }

        void addItem( std::unique_ptr<Item<KeyType>> item ){
            KeyType id = item->getID();

            if ( m_pool.count( id ) )
                throw std::runtime_error( "The item with ID "+std::to_string(id)+" already exists in the pool!" );

            std::pair<KeyType, std::unique_ptr<Item<KeyType>>> p = { id, std::move( item ) };

            m_pool.insert( std::move( p ) );
        }

        Item<KeyType> * getItem( KeyType id ) const {
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