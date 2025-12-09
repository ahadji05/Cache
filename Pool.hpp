#pragma once

#include "Item.hpp"
#include <memory>
#include <map>

class Pool {

    private:
        std::map<int64_t, std::unique_ptr<Item>> m_pool;

    public:
        Pool(){}
        ~Pool(){}

        size_t getPoolSize() const {
            return m_pool.size();
        }

        void addItem( std::unique_ptr<Item> item ){
            int64_t id = item->getID();

            if ( m_pool.count( id ) )
                throw std::runtime_error( "The item with ID "+std::to_string(id)+" already exists in the pool!" );

            std::pair<int64_t, std::unique_ptr<Item>> p = { id, std::move( item ) };

            m_pool.insert( std::move( p ) );
        }

        Item * getItem( int64_t id ) const {
            if ( !m_pool.count( id ) ) {
                std::cout << "not found in pool...\n";
                return nullptr;
            }

            std::cout << "found in pool...\n";
            return m_pool.at(id).get();
        }
};