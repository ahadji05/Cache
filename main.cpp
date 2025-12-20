
#include "CacheFIFO.hpp"
#include "CacheLRU.hpp"
#include "CacheLFU.hpp"
#include "Pool.hpp"
#include "Item.hpp"
#include <cassert>
#include <chrono>

/**
 * A dummy item that throws an exception if accessed before being loaded.
 * Used for testing the cache and pool functionality.
 */
class TestItem : public Item< int64_t > {

    private:
        int  m_value;
        bool m_loaded;

    public:
        using key_type = typename Item< int64_t >::key_type;

        ~TestItem(){}
        TestItem( key_type itemID, int value ) : m_value( value ), m_loaded( false ) {
            this->m_itemID = itemID;
        }

        void load() override {
            if ( !m_loaded )
                m_loaded = true;
        }

        int get() const {
            if ( !m_loaded )
                throw std::runtime_error( "ITEM_NOT_LOADED" );
            return m_value;
        }

        void unload() override {
            if ( m_loaded )
                m_loaded = false;
        }
};

using key_type = typename TestItem::key_type;
using item_type = TestItem;

template< 
        typename Key, 
        typename Value,
        typename Hash = std::hash<Key>,
        typename Pred = std::equal_to<Key>
        >
using map_type = std::unordered_map< Key, Value, Hash, Pred >;

int test_for_CacheLRU( Pool<key_type, map_type> const& pool, std::vector<int> const& testData );
int test_for_CacheLFU( Pool<key_type, map_type> const& pool, std::vector<int> const& testData );
int test_for_CacheFIFO( Pool<key_type, map_type> const& pool, std::vector<int> const& testData );

int main() {

    // create some test data.
    std::vector<int> testData( 10 );
    int value = 0;
    for ( auto & c : testData ) {
        std::cout << "Value: " << value << std::endl;
        c = value;
        value = 2 * value + 1;
    }

    // populate the pool with some items.
    Pool<key_type, map_type> pool;
    for ( int i = 0 ; i < 10 ; ++i )
        pool.addItem( std::make_unique<item_type>( i, testData[i] ) );

    test_for_CacheLRU( pool, testData );

    test_for_CacheLFU( pool, testData );

    test_for_CacheFIFO( pool, testData );

    return 0;
}

int test_for_CacheLRU( Pool<key_type, map_type> const& pool, std::vector<int> const& testData ) {

    // Create a cache with capacity of 3 items.
    CacheLRU<key_type, map_type> cache( &pool, 3 );
    assert( cache.getCacheSizeInNumItems() == 3 );
    assert( cache.getPool() == &pool );
    assert( cache.getCache().size() == 0 );
    assert( cache.getCacheHits() == 0 );
    assert( cache.getCacheEvictions() == 0 );

    // Add new item with ID 6.
    auto item = cache.getItem( 6 );
    auto cacheOrder = cache.getCacheOrder();
    auto cacheMap = cache.getCache();
    assert( cacheOrder.size() == 1 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( dynamic_cast<TestItem*>( cacheOrder.front() )->get() == testData[ 6 ] );

    // Add new item with ID 5.
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( cacheMap.size() == 2 );
    assert( cacheOrder.size() == 2 );
    assert( cacheOrder.back()->getID() == 5 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( dynamic_cast<TestItem*>( cacheOrder.back() )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheOrder.front() )->get() == testData[ 6 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );

    // Add new item with ID 4.
    item = cache.getItem( 4 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( cacheMap.size() == 3 );
    assert( cacheOrder.size() == 3 );
    assert( cacheOrder.back()->getID() == 4 );
    assert( cacheOrder.front()->getID() == 6 );
    auto it = cacheOrder.begin();
    assert( dynamic_cast<TestItem*>( *it )->get() == testData[ 6 ] );
    it = std::next( it );
    assert( dynamic_cast<TestItem*>( *it )->get() == testData[ 5 ] );
    it = std::next( it );
    assert( dynamic_cast<TestItem*>( *it )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 0 );

    // Touch item 5.
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 1 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 5 );

    // Touch item 5 (again).
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 2 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 5 );

    // Touch item 4.
    item = cache.getItem( 4 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 4 );

    // Add new item 7; should evict item 6.
    item = cache.getItem( 7 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 7 ) )->get() == testData[ 7 ] );
    assert( cache.getCacheEvictions() == 1 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 5 );
    assert( cacheOrder.back()->getID() == 7 );

    // Add new item 9; should evict item 5.
    item = cache.getItem( 9 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<TestItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 7 ) )->get() == testData[ 7 ] );
    assert( dynamic_cast<TestItem*>( cacheMap.at( 9 ) )->get() == testData[ 9 ] );
    assert( cache.getCacheEvictions() == 2 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 4 );
    assert( cacheOrder.back()->getID() == 9 );

    // Try to get an item that does not exist in the pool.
    std::string errorMsg;
    try {
        item = cache.getItem( 11 );
    } catch( std::exception const& e ) {
        errorMsg = e.what();
    }
    assert( errorMsg == "Not found item with ID 11 in the pool!" );

    return 0;
}

int test_for_CacheLFU( Pool<key_type, map_type> const& pool, std::vector<int> const& testData )
{
    CacheLFU<key_type, map_type> lfuCache( &pool, 3 );

    // Add item with ID 5.
    lfuCache.getItem( 5 );
    auto cache = lfuCache.getCache();
    auto usageCount = lfuCache.getUsageCount();
    auto frequencyMap = lfuCache.getFrequencyMap();
    assert( cache.size() == 1 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( frequencyMap.size() == 1 );
    assert( frequencyMap.at( 1 ).size() == 1 );
    assert( usageCount.at( cache.at( 5 ) ) == 1 );

    // Add buffer with ID 9.
    lfuCache.getItem( 9 );
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 2 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( frequencyMap.size() == 1 );
    assert( frequencyMap.at(1).size() == 2 );
    assert( usageCount.at( cache.at( 5 ) ) == 1 );
    assert( usageCount.at( cache.at( 9 ) ) == 1 );

    // Add item with ID 5 (touch).
    lfuCache.getItem( 5 );
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 2 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( frequencyMap.size() == 2 );
    assert( frequencyMap.at(1).size() == 1 );
    assert( frequencyMap.at(2).size() == 1 );
    assert( usageCount.at( cache.at( 5 ) ) == 2 );
    assert( usageCount.at( cache.at( 9 ) ) == 1 );

    // Add item with ID 4.
    lfuCache.getItem( 4 );
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 3 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( frequencyMap.size() == 2 );
    assert( frequencyMap.at(1).size() == 2 );
    assert( frequencyMap.at(2).size() == 1 );
    assert( usageCount.at( cache.at( 5 ) ) == 2 );
    assert( usageCount.at( cache.at( 9 ) ) == 1 );
    assert( usageCount.at( cache.at( 4 ) ) == 1 );

    // Add item with ID 9 (touch).
    lfuCache.getItem( 9 );
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 3 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( frequencyMap.size() == 2 );
    assert( frequencyMap.at(1).size() == 1 );
    assert( frequencyMap.at(2).size() == 2 );
    assert( usageCount.at( cache.at( 5 ) ) == 2 );
    assert( usageCount.at( cache.at( 9 ) ) == 2 );
    assert( usageCount.at( cache.at( 4 ) ) == 1 );

    // Add item with ID 7; should evict item with ID 4.
    lfuCache.getItem( 7 );
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 3 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( cache.at( 7 )->getID() == 7 );
    assert( dynamic_cast<TestItem*>(cache.at( 7 ))->get() == testData[ 7 ] );
    assert( frequencyMap.size() == 2 );
    assert( frequencyMap.at(1).size() == 1 );
    assert( frequencyMap.at(2).size() == 2 );
    assert( usageCount.at( cache.at( 5 ) ) == 2 );
    assert( usageCount.at( cache.at( 9 ) ) == 2 );
    assert( usageCount.at( cache.at( 7 ) ) == 1 );

    // Add item with ID 7 two time so it becomes the most frequently used, then add item with ID 5 to
    // cause eviction of item with ID 9 from item with ID 4.
    lfuCache.getItem( 7 );
    lfuCache.getItem( 7 ); // 7 becomes most frequently used.
    lfuCache.getItem( 5 ); // now 5 catches up with 7.
    lfuCache.getItem( 4 ); // should evict item with ID 9.
    cache = lfuCache.getCache();
    usageCount = lfuCache.getUsageCount();
    frequencyMap = lfuCache.getFrequencyMap(); 
    assert( cache.size() == 3 );
    assert( cache.at( 5 )->getID() == 5 );
    assert( dynamic_cast<TestItem*>(cache.at( 5 ))->get() == testData[ 5 ] );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( cache.at( 7 )->getID() == 7 );
    assert( dynamic_cast<TestItem*>(cache.at( 7 ))->get() == testData[ 7 ] );
    assert( frequencyMap.size() == 2 );
    assert( frequencyMap.at(1).size() == 1 );
    assert( frequencyMap.at(3).size() == 2 );
    assert( usageCount.at( cache.at( 5 ) ) == 3 );
    assert( usageCount.at( cache.at( 4 ) ) == 1 );
    assert( usageCount.at( cache.at( 7 ) ) == 3 );

    // Try to get an item that does not exist in the pool.
    std::string errorMsg;
    try {
        lfuCache.getItem( 11 );
    } catch( std::exception const& e ) {
        errorMsg = e.what();
    }
    assert( errorMsg == "Not found item with ID 11 in the pool!" );

    // Check cache hits and evictions.
    assert( lfuCache.getCacheHits() == 5 );
    assert( lfuCache.getCacheEvictions() == 2 );

    // Print usage count.
    for ( auto const& it : usageCount )
        std::cout << "Item ID: " << it.first->getID() << " | Usage count: " << it.second << std::endl;

    return 0;
}

int test_for_CacheFIFO( Pool<key_type, map_type> const& pool, std::vector<int> const& testData )
{
    CacheFIFO<key_type, map_type> fifoCache( &pool, 3 );

    // Add item with ID 2.
    fifoCache.getItem( 2 );
    auto cache = fifoCache.getCache();
    auto cacheOrder = fifoCache.getCacheOrder();
    assert( cache.size() == 1 );
    assert( cache.at( 2 )->getID() == 2 );
    assert( dynamic_cast<TestItem*>(cache.at( 2 ))->get() == testData[ 2 ] );
    assert( cacheOrder.size() == 1 );
    assert( cacheOrder.front() == 2 );

    // Add item with ID 4.
    fifoCache.getItem( 4 );
    cache = fifoCache.getCache();
    cacheOrder = fifoCache.getCacheOrder();
    assert( cache.size() == 2 );
    assert( cache.at( 2 )->getID() == 2 );
    assert( dynamic_cast<TestItem*>(cache.at( 2 ))->get() == testData[ 2 ] );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( cacheOrder.size() == 2 );
    assert( cacheOrder.front() == 2 );
    assert( cacheOrder.back() == 4 );

    // Add item with ID 6.
    fifoCache.getItem( 6 );
    cache = fifoCache.getCache();
    cacheOrder = fifoCache.getCacheOrder();
    assert( cache.size() == 3 );
    assert( cache.at( 2 )->getID() == 2 );
    assert( dynamic_cast<TestItem*>(cache.at( 2 ))->get() == testData[ 2 ] );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( cache.at( 6 )->getID() == 6 );
    assert( dynamic_cast<TestItem*>(cache.at( 6 ))->get() == testData[ 6 ] );
    assert( cacheOrder.size() == 3 );
    assert( cacheOrder.front() == 2 );
    assert( cacheOrder.back() == 6 );

    // Add item with ID 8; should evict item with ID 2.
    fifoCache.getItem( 8 );
    cache = fifoCache.getCache();
    cacheOrder = fifoCache.getCacheOrder();
    assert( cache.size() == 3 );
    assert( cache.at( 4 )->getID() == 4 );
    assert( dynamic_cast<TestItem*>(cache.at( 4 ))->get() == testData[ 4 ] );
    assert( cache.at( 6 )->getID() == 6 );
    assert( dynamic_cast<TestItem*>(cache.at( 6 ))->get() == testData[ 6 ] );
    assert( cache.at( 8 )->getID()  == 8 );
    assert( dynamic_cast<TestItem*>(cache.at( 8 ))->get() == testData[ 8 ] );
    assert( cacheOrder.size() == 3 );
    assert( cacheOrder.front() == 4 );
    assert( cacheOrder.back() == 8 );

    // Add item with ID 4 (touch).
    fifoCache.getItem( 4 );

    // Add item with ID 10; should evict item with ID 4 because the order is not affected with touch.
    fifoCache.getItem( 9 );
    cache = fifoCache.getCache();
    cacheOrder = fifoCache.getCacheOrder();
    assert( cache.size() == 3 );
    assert( cache.at( 6 )->getID() == 6 );
    assert( dynamic_cast<TestItem*>(cache.at( 6 ))->get() == testData[ 6 ] );
    assert( cache.at( 8 )->getID() == 8 );
    assert( dynamic_cast<TestItem*>(cache.at( 8 ))->get() == testData[ 8 ] );
    assert( cache.at( 9 )->getID() == 9 );
    assert( dynamic_cast<TestItem*>(cache.at( 9 ))->get() == testData[ 9 ] );
    assert( cacheOrder.size() == 3 );
    assert( cacheOrder.front() == 6 );
    assert( cacheOrder.back() == 9 );

    // Try to get an item that does not exist in the pool.
    std::string errorMsg;
    try {
        fifoCache.getItem( 11 );
    } catch( std::exception const& e ) {
        errorMsg = e.what();
    }
    assert( errorMsg == "Not found item with ID 11 in the pool!" );

    // Check cache hits and evictions.
    assert( fifoCache.getCacheHits() == 1 );
    assert( fifoCache.getCacheEvictions() == 2 );

    // Print usage count.
    std::cout << "Cache order (from oldest to newest):" << std::endl;
    for ( auto const& e : cacheOrder )
        std::cout << "Item ID: " << e << std::endl;

    return 0;
}