#pragma once

#include "Cache.hpp"

class CacheLRU : public Cache {

    private:
        // List that keeps on the one end the next item to be evicted and on the other end the most recently used.
        std::list<Item*> _cacheOrder;
        std::map<int64_t, std::list<Item*>::const_iterator> _itemPositions;

        // Cache maximum size.
        size_t _cacheSizeInNumItems = 0;

        // Keep track of the number of cache hits.
        size_t _cacheHits = 0;
        size_t _cacheEvictions = 0;

        Item * searchPool ( int64_t itemID ) override {
            return this->m_pool->getItem( itemID );
        }

        Item * searchCache ( int64_t itemID ) override {
            #ifdef DEBUG_CACHES
            std::cout << "searching cache...\n";
            #endif
            auto it = this->_cache.find( itemID );
            if ( it == _cache.end() ) {
                #ifdef DEBUG_CACHES
                std::cout << "not found in cache...\n";
                #endif
                return nullptr;
            }

            touchItem( *it->second );
            return it->second;
        }

        void loadItemFromPool( Item &item ) override {
            if ( _cache.size() >= _cacheSizeInNumItems ) {
                int64_t itemID = _cacheOrder.front()->getID();
                #ifdef DEBUG_CACHES
                std::cout << "evicting item... " << itemID << std::endl;
                #endif
                _cache.erase( itemID );
                _cacheOrder.front()->unload();
                _cacheOrder.pop_front();
                ++_cacheEvictions;
            }
            item.load();
        }

        void addItemToCache  ( Item &item ) override {
            int64_t itemID = item.getID();
            #ifdef DEBUG_CACHES
            std::cout << "adding item..." << itemID << std::endl;
            #endif
            _cache[ itemID ] = &item;
            _cacheOrder.push_back( &item );
            _itemPositions.insert( { item.getID(), std::prev( _cacheOrder.end() ) } );
        }

        void touchItem ( Item & item ) override {
            #ifdef DEBUG_CACHES
            std::cout << "touching item..." << item.getID() << std::endl;
            #endif
            std::list<Item*>::const_iterator it = _itemPositions.at( item.getID() );
            _cacheOrder.erase( it );
            _cacheOrder.push_back( &item );
            _itemPositions[ item.getID() ] = std::prev( _cacheOrder.end() );
        }

    public:
        CacheLRU( Pool const * pool, size_t cacheSizeInNumItems ) : _cacheSizeInNumItems( cacheSizeInNumItems ) {
            this->m_pool = pool;
        }

        std::list<Item*> getCacheOrder() const {
            return _cacheOrder;
        }

        std::map<int64_t, Item*> getCache() const {
            return _cache;
        }

        Item * getItem( int64_t itemID ) override {

            // search the cache and return the item if it is already in.
            Item * item = searchCache( itemID );
            if ( item != nullptr ) {
                ++_cacheHits;
                return item;
            }

            // load from the pool.
            item = searchPool( itemID );
            if ( item == nullptr )
                throw std::runtime_error( "Not found item with ID "+std::to_string( itemID )+" in the pool!" );

            loadItemFromPool( *item );

            // update cache status.
            addItemToCache( *item );

            // return item reference.
            return item;
        }
};