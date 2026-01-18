#pragma once

#include "Cache.hpp"

template<
        typename KeyType = int64_t ,
        template <typename... Args> typename MapType = std::map
        >
class CacheLFU : public Cache< KeyType, MapType > {

    private:
        using item_type = Item< KeyType >;
        using usageCountIterator = typename MapType< item_type*, size_t >::iterator;

        // Frequency count per Item.
        MapType< item_type*, size_t > _usageCount;

        // Map of frequency count to map of Item pointers.
        MapType< size_t, MapType< KeyType, usageCountIterator > > _frequencyMap;

        // Keep track of the least frequency count in the cache.
        size_t _leastFrequencyCount = 0;

        item_type * searchPool      ( KeyType itemID );
        item_type * searchCache     ( KeyType itemID );
        void        loadItemFromPool( item_type &item );
        void        addItemToCache  ( item_type &item );
        void        touchItem       ( item_type & item );
        void        removeItemFromFrequencyMap( size_t frequency, KeyType key );

    public:
        CacheLFU( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems );
        ~CacheLFU() noexcept;

        MapType< KeyType, item_type* > getCache() const;
        MapType< item_type*, size_t >  getUsageCount() const;
        MapType< size_t, MapType< KeyType, usageCountIterator > > getFrequencyMap() const;
        size_t getLeastFrequencyCount() const;

        item_type * getItem( KeyType itemID ) override;
};

/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType, template <typename... Args> class MapType >
CacheLFU<KeyType, MapType>::
CacheLFU( Pool< KeyType, MapType > const * pool, size_t cacheSizeInNumItems )
{
    this->m_pool = pool;
    this->_cacheSizeInNumItems = cacheSizeInNumItems;
}

template< typename KeyType, template <typename... Args> class MapType >
CacheLFU<KeyType, MapType>::
~CacheLFU() noexcept
{
}

template< typename KeyType, template <typename... Args> class MapType >
size_t
CacheLFU<KeyType, MapType>::getLeastFrequencyCount() const
{
    return _leastFrequencyCount;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType< typename CacheLFU<KeyType, MapType>::item_type*, size_t >  
CacheLFU<KeyType, MapType>::getUsageCount() const
{
    return _usageCount;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType< size_t, MapType< KeyType, typename CacheLFU<KeyType, MapType>::usageCountIterator > >
CacheLFU<KeyType, MapType>::getFrequencyMap() const
{
    return _frequencyMap;
}

template< typename KeyType, template <typename... Args> class MapType >
MapType< KeyType, typename CacheLFU<KeyType, MapType>::item_type* >
CacheLFU<KeyType, MapType>::getCache() const
{
    return this->_cache;
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLFU<KeyType, MapType>::item_type * 
CacheLFU<KeyType, MapType>::getItem( KeyType itemID )
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
CacheLFU<KeyType, MapType>::touchItem ( item_type & item )
{
    KeyType itemID = item.getID();
    #ifdef DEBUG_CACHES
    std::cout << "touching item..." << itemID << std::endl;
    #endif
    // get iterator to the item in _usageCount map.
    usageCountIterator it = _usageCount.find( &item ) ;
    size_t &currentCount = it->second;

    // from the current usage count find the item in frequency-map and remove it.
    removeItemFromFrequencyMap( currentCount, itemID );

    // Increment least frequency count if this item was the last item in _frequencyMap that had
    // a usage count = _leastFrequencyCount
    if ( currentCount == _leastFrequencyCount && _frequencyMap.count( _leastFrequencyCount ) == 0 )
        ++_leastFrequencyCount;

    // update the count
    ++currentCount;

    // update the frequency map
    _frequencyMap[ currentCount][ itemID ] = it;

    ++this->_cacheHits;
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheLFU<KeyType, MapType>::addItemToCache  ( item_type &item )
{
    KeyType itemID = item.getID();
    #ifdef DEBUG_CACHES
    std::cout << "adding item..." << itemID << std::endl;
    #endif
    // Register a new Item object to the cache.
    this->_cache[ itemID ] = &item;
    _leastFrequencyCount = 1;
    _usageCount[ &item ] = 1;
    _frequencyMap[ _leastFrequencyCount ][ itemID ] = _usageCount.find( &item );
}

template< typename KeyType, template <typename... Args> class MapType >
void 
CacheLFU<KeyType, MapType>::removeItemFromFrequencyMap( size_t frequency,
    KeyType key )
{
    _frequencyMap[ frequency ].erase( key );
    if ( _frequencyMap[ frequency ].empty() )
        _frequencyMap.erase( frequency );
}

template< typename KeyType, template <typename... Args> class MapType >
void
CacheLFU<KeyType, MapType>::loadItemFromPool( item_type &item )
{
    if ( this->_cache.size() >= this->_cacheSizeInNumItems ) {

        // evict least frequently used item.
        auto const& it = _frequencyMap[ _leastFrequencyCount ].begin();
        usageCountIterator mapIterator = it->second;
        KeyType itemIDToEvict = it->first;
        #ifdef DEBUG_CACHES
        std::cout << "evicting item... " << itemIDToEvict << std::endl;
        #endif

        mapIterator->first->unload();
        _usageCount.erase( mapIterator );
        removeItemFromFrequencyMap( _leastFrequencyCount, itemIDToEvict );
        this->_cache.erase( itemIDToEvict );
        ++this->_cacheEvictions;
    }
    item.load();
}

template< typename KeyType, template <typename... Args> class MapType >
typename CacheLFU<KeyType, MapType>::item_type *
CacheLFU<KeyType, MapType>::searchCache ( KeyType itemID )
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
typename CacheLFU<KeyType, MapType>::item_type *
CacheLFU<KeyType, MapType>::searchPool ( KeyType itemID )
{
    return this->m_pool->getItem( itemID );
}
