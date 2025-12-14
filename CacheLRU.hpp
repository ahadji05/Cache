#pragma once

#include "Cache.hpp"

template<
        typename KeyType = int64_t ,
        template <typename... Args> typename MapType = std::map
        >
class CacheLRU : public Cache< KeyType, MapType > {

    private:
        using item_type          = Item< KeyType >;
        using list_type          = std::list< item_type* >;
        using list_iterator_type = typename list_type::const_iterator;

        // List that keeps on the one end the next item to be evicted and on the other end the most recently used.
        list_type                              _cacheOrder;
        MapType< KeyType, list_iterator_type > _itemPositions;

        item_type * searchPool ( KeyType itemID ) override {
            return this->m_pool->getItem( itemID );
        }

        item_type * searchCache ( KeyType itemID ) override {
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
            return it->second;
        }

        void loadItemFromPool( item_type &item ) override {
            if ( this->_cache.size() >= this->_cacheSizeInNumItems ) {
                KeyType itemID = _cacheOrder.front()->getID();
                #ifdef DEBUG_CACHES
                std::cout << "evicting item... " << itemID << std::endl;
                #endif
                _cacheOrder.front()->unload();
                _cacheOrder.pop_front();
                this->_cache.erase( itemID );
                ++this->_cacheEvictions;
            }
            item.load();
        }

        void addItemToCache  ( item_type &item ) override {
            KeyType itemID = item.getID();
            #ifdef DEBUG_CACHES
            std::cout << "adding item..." << itemID << std::endl;
            #endif
            this->_cache[ itemID ] = &item;
            _cacheOrder.push_back( &item );
            _itemPositions.insert( { item.getID(), std::prev( _cacheOrder.end() ) } );
        }

        void touchItem ( item_type & item ) override {
            #ifdef DEBUG_CACHES
            std::cout << "touching item..." << item.getID() << std::endl;
            #endif
            list_iterator_type it = _itemPositions.at( item.getID() );
            _cacheOrder.erase( it );
            _cacheOrder.push_back( &item );
            _itemPositions[ item.getID() ] = std::prev( _cacheOrder.end() );
            ++this->_cacheHits;
        }

    public:
        CacheLRU( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems ) {
            this->m_pool = pool;
            this->_cacheSizeInNumItems = cacheSizeInNumItems;
        }

        list_type getCacheOrder() const {
            return _cacheOrder;
        }

        MapType< KeyType, item_type* > getCache() const {
            return this->_cache;
        }

        item_type * getItem( KeyType itemID ) override {

            // search the cache and return the item if it is already in.
            item_type * item = searchCache( itemID );
            if ( item != nullptr ) {
                touchItem( *item );
                return item;
            }

            // search pool for the item.
            item = searchPool( itemID );
            if ( item == nullptr )
                throw std::runtime_error( "Not found item with ID "+std::to_string( itemID )+" in the pool!" );

            // load from the pool.
            loadItemFromPool( *item );

            // update cache status.
            addItemToCache( *item );

            // return item reference.
            return item;
        }
};