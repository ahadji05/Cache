/*
* MIT License

* Copyright (c) 2025 Andreas

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
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
        list_type _cacheOrder;

        // Map that keeps track of the positions of items in the list for fast evictions and touches.
        MapType< KeyType, list_iterator_type > _itemPositions;

        item_type * searchPool      ( KeyType itemID );
        item_type * searchCache     ( KeyType itemID );
        void        loadItemFromPool( item_type &item );
        void        addItemToCache  ( item_type &item );
        void        touchItem       ( item_type & item );

    public:
        CacheLRU( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems );
        ~CacheLRU() noexcept;

        MapType< KeyType, item_type* > getCache() const;
        list_type getCacheOrder() const;

        item_type * getItem( KeyType itemID ) override;
};

/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType, template <typename... Args> class MapType >
CacheLRU<KeyType, MapType>::
CacheLRU( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems )
{
    this->m_pool = pool;
    this->_cacheSizeInNumItems = cacheSizeInNumItems;
}

template< typename KeyType, template <typename... Args> class MapType >
CacheLRU<KeyType, MapType>::~CacheLRU() noexcept
{
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLRU<KeyType, MapType>::list_type
CacheLRU<KeyType, MapType>::getCacheOrder() const
{
    return _cacheOrder;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType< KeyType, typename CacheLRU<KeyType, MapType>::item_type* >
CacheLRU<KeyType, MapType>::getCache() const
{
    return this->_cache;
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLRU<KeyType, MapType>::item_type * 
CacheLRU<KeyType, MapType>::getItem( KeyType itemID )
{
    // search the cache and return the item if it is already in.
    item_type * item = searchCache( itemID );
    if ( item != nullptr ) {
        touchItem( *item );
        return item;
    }

    // search pool for the item.
    item = searchPool( itemID );
    if ( item == nullptr )
        throw std::runtime_error( "Item not found in the pool!" );

    // load from the pool.
    loadItemFromPool( *item );

    // update cache status.
    addItemToCache( *item );

    // return item reference.
    return item;
}

/**********************************
 * PRIVATE INTERFACE IMPLEMENTATION
 *********************************/

template< typename KeyType, template <typename... Args> class MapType >
void
CacheLRU<KeyType, MapType>::touchItem ( item_type & item )
{
    #ifdef DEBUG_CACHES
    std::cout << "touching item..." << item.getID() << std::endl;
    #endif
    list_iterator_type it = _itemPositions.at( item.getID() );
    _cacheOrder.erase( it );
    _cacheOrder.push_back( &item );
    _itemPositions[ item.getID() ] = std::prev( _cacheOrder.end() );
    ++this->_cacheHits;
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheLRU<KeyType, MapType>::addItemToCache  ( item_type &item )
{
    KeyType itemID = item.getID();
    #ifdef DEBUG_CACHES
    std::cout << "adding item..." << itemID << std::endl;
    #endif
    this->_cache[ itemID ] = &item;
    _cacheOrder.push_back( &item );
    _itemPositions[ item.getID() ] = std::prev( _cacheOrder.end() );
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheLRU<KeyType, MapType>::loadItemFromPool( item_type &item )
{
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

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLRU<KeyType, MapType>::item_type *
CacheLRU<KeyType, MapType>::searchCache ( KeyType itemID )
{
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

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLRU<KeyType, MapType>::item_type *
CacheLRU<KeyType, MapType>::searchPool ( KeyType itemID )
{
    return this->m_pool->getItem( itemID );
}
