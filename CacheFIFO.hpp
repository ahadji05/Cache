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
#include <deque>

template<
        typename KeyType = int64_t ,
        template <typename... Args> typename MapType = std::map
        >
class CacheFIFO : public Cache< KeyType, MapType > {

    private:
        using item_type = Item< KeyType >;

        // queue to keep track of the order of items in cache.
        std::deque< KeyType > _cacheOrder;

        item_type * searchPool      ( KeyType itemID );
        item_type * searchCache     ( KeyType itemID );
        void        loadItemFromPool( item_type &item );
        void        addItemToCache  ( item_type &item );
        void        touchItem       ( item_type & item );

    public:
        CacheFIFO( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems );
        ~CacheFIFO() noexcept;

        MapType< KeyType, item_type* > getCache() const;
        std::deque< KeyType > getCacheOrder() const;

        item_type * getItem( KeyType itemID ) override;
};

/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType, template <typename... Args> class MapType >
CacheFIFO<KeyType, MapType>::
CacheFIFO( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems )
{
    this->m_pool = pool;
    this->_cacheSizeInNumItems = cacheSizeInNumItems;
}

template< typename KeyType, template <typename... Args> class MapType >
CacheFIFO<KeyType, MapType>::~CacheFIFO() noexcept
{
}

template< typename KeyType, template <typename... Args> class MapType >
std::deque< KeyType >
CacheFIFO<KeyType, MapType>::getCacheOrder() const
{
    return _cacheOrder;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType< KeyType, typename CacheFIFO<KeyType, MapType>::item_type* >
CacheFIFO<KeyType, MapType>::getCache() const
{
    return this->_cache;
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheFIFO<KeyType, MapType>::item_type * 
CacheFIFO<KeyType, MapType>::getItem( KeyType itemID )
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
CacheFIFO<KeyType, MapType>::touchItem ( item_type & item )
{
    #ifdef DEBUG_CACHES
    std::cout << "touching item..." << item.getID() << std::endl;
    #endif
    // touch is a no-operation call for FIFO cache.
    ++this->_cacheHits;
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheFIFO<KeyType, MapType>::addItemToCache  ( item_type &item )
{
    KeyType itemID = item.getID();
    #ifdef DEBUG_CACHES
    std::cout << "adding item..." << itemID << std::endl;
    #endif
    this->_cache[ itemID ] = &item;
    _cacheOrder.push_back( itemID );
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheFIFO<KeyType, MapType>::loadItemFromPool( item_type &item )
{
    if ( this->_cache.size() >= this->_cacheSizeInNumItems ) {
        KeyType itemID = _cacheOrder.front();
        _cacheOrder.pop_front();
        #ifdef DEBUG_CACHES
        std::cout << "evicting item... " << itemID << std::endl;
        #endif
        this->_cache.at( itemID )->unload();
        this->_cache.erase( itemID );
        ++this->_cacheEvictions;
    }
    item.load();
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheFIFO<KeyType, MapType>::item_type *
CacheFIFO<KeyType, MapType>::searchCache ( KeyType itemID )
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
typename CacheFIFO<KeyType, MapType>::item_type *
CacheFIFO<KeyType, MapType>::searchPool ( KeyType itemID )
{
    return this->m_pool->getItem( itemID );
}
