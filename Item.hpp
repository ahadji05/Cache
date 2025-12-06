#pragma once

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <unordered_map>

class Item {

    protected:
        int64_t m_itemID;
        void * m_data = nullptr;
        size_t m_size = 0;

    public:
        ~Item(){}
        Item(){}
        Item( int64_t id ) : m_itemID(id) {}
        int64_t getID() const             { return m_itemID; }
        void setID( int64_t const& id ){ m_itemID = id;   }

        // virtual public interface
        virtual void load()                          = 0;
        virtual void clean()                         = 0;
        virtual void serialize( void ** data ) const = 0;
        virtual void deserialize( void * data )      = 0;
};

class ItemBuffer : public Item {

    ItemBuffer(){}
    ~ItemBuffer(){}
    ItemBuffer( int64_t id ) {
        this->m_itemID = id;
    }

    void serialize( void ** data ) const override {
        if ( m_data != nullptr ) {

            size_t sz = sizeof( size_t ) + m_size;

            *data = malloc( sz );

            char * p = ( char * ) *data;

            std::memcpy(p, &m_size, sizeof( size_t ) );

            std::memcpy( p + 4, m_data, m_size );
        }
    }

    void deserialize( void * serializedData ) override {
        if ( serializedData != nullptr ){

            char * p = ( char * ) serializedData;

            std::memcpy( &m_size, p, sizeof( size_t ) );

            std::memcpy( m_data, p + 4, m_size );
        }
    }
};

class Pool {

    private:
        std::map<int64_t, std::unique_ptr<Item>> m_pool;

    public:
        Pool(){}
        ~Pool(){}

        size_t getPoolSize() const {
            return m_pool.size();
        }

        void addItem( std::unique_ptr<Item> &const item ){

            int64_t id = item->getID();

            if ( m_pool.count( id ) )
                throw std::runtime_error( "The item with ID "+std::to_string(id)+" already exists in the pool!" );

            std::pair<int64_t, std::unique_ptr<Item>> p = { id, std::move( item ) };

            m_pool.insert( std::move( p ) );
        }

        Item &getItem( int64_t id ) const {
            if ( !m_pool.count( id ) )
                throw std::runtime_error( "The item with ID "+std::to_string(id)+" is not in the pool!" );

            return *m_pool.at(id).get();
        }
};

class Cache {

    protected:
        Pool &const m_pool;

        // Cache data with specified capacity (owned memory).
        size_t _cacheSizeInNumItems = 0;

        // Map of buffer ID to Buffer objects in cache.
        std::unordered_map<int64_t, std::unique_ptr<Item>> _cache;

        virtual void searchPool       ( Item const &const item ) = 0;
        virtual void searchCache      ( Item const &const item ) = 0;
        virtual void fetchItemFromPool( Item const &const item ) = 0;
        virtual void addItem          ( Item const &const item ) = 0;
        virtual void updateCache      ( Item const &const item ) = 0;

    public:
        Cache( Pool &const pool ) : m_pool( pool ) {}

        virtual Item &getItem( int64_t itemID ) = 0;
};
