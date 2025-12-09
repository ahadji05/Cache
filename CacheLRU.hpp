#pragma once

#include "Cache.hpp"

template< typename KeyType >
class CacheLRU : public Cache< KeyType > {

    private:
        // List that keeps on the one end the next item to be evicted and on the other end the most recently used.
        std::list<Item<KeyType>*> _cacheOrder;
        std::map<KeyType, typename std::list<Item<KeyType>*>::const_iterator> _itemPositions;

        // Cache maximum size.
        size_t _cacheSizeInNumItems = 0;

        // Keep track of the number of cache hits.
        size_t _cacheHits = 0;
        size_t _cacheEvictions = 0;

        Item<KeyType> * searchPool ( KeyType itemID ) override {
            return this->m_pool->getItem( itemID );
        }

        Item<KeyType> * searchCache ( KeyType itemID ) override {
            #ifdef DEBUG_CACHES
            std::cout << "searching cache...\n";
            #endif
            auto it = this->_cache.find( itemID );
            if ( it == this->_cache.end() ) {
                #ifdef DEBUG_CACHES
                std::cout << "not found in cache...\n";
                #endif
                return nullptr;
            }

            touchItem( *it->second );
            return it->second;
        }

        void loadItemFromPool( Item<KeyType> &item ) override {
            if ( this->_cache.size() >= _cacheSizeInNumItems ) {
                KeyType itemID = _cacheOrder.front()->getID();
                #ifdef DEBUG_CACHES
                std::cout << "evicting item... " << itemID << std::endl;
                #endif
                this->_cache.erase( itemID );
                _cacheOrder.front()->unload();
                _cacheOrder.pop_front();
                ++_cacheEvictions;
            }
            item.load();
        }

        void addItemToCache  ( Item<KeyType> &item ) override {
            KeyType itemID = item.getID();
            #ifdef DEBUG_CACHES
            std::cout << "adding item..." << itemID << std::endl;
            #endif
            this->_cache[ itemID ] = &item;
            _cacheOrder.push_back( &item );
            _itemPositions.insert( { item.getID(), std::prev( _cacheOrder.end() ) } );
        }

        void touchItem ( Item<KeyType> & item ) override {
            #ifdef DEBUG_CACHES
            std::cout << "touching item..." << item.getID() << std::endl;
            #endif
            typename std::list<Item<KeyType>*>::const_iterator it = _itemPositions.at( item.getID() );
            _cacheOrder.erase( it );
            _cacheOrder.push_back( &item );
            _itemPositions[ item.getID() ] = std::prev( _cacheOrder.end() );
        }

    public:
        CacheLRU( Pool<KeyType> const * pool, size_t cacheSizeInNumItems ) : _cacheSizeInNumItems( cacheSizeInNumItems ) {
            this->m_pool = pool;
        }

        std::list<Item<KeyType>*> getCacheOrder() const {
            return _cacheOrder;
        }

        std::map<KeyType, Item<KeyType>*> getCache() const {
            return this->_cache;
        }

        Item<KeyType> * getItem( KeyType itemID ) override {

            // search the cache and return the item if it is already in.
            Item<KeyType> * item = searchCache( itemID );
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