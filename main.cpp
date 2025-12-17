
#include "Item.hpp"
#include "CacheLRU.hpp"
#include "Pool.hpp"
#include <cassert>
#include <chrono>

/**
 * A dummy item that throws an exception if accessed before being loaded.
 * Used for testing the cache and pool functionality.
 */
class DummyItem : public Item< int64_t > {

    private:
        int  m_value;
        bool m_loaded;

    public:
        using key_type = typename Item< int64_t >::key_type;

        ~DummyItem(){}
        DummyItem( key_type itemID, int value ) : m_value( value ), m_loaded( false ) {
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

using key_type = typename DummyItem::key_type;
using item_type = DummyItem;

template< 
        typename Key, 
        typename Value,
        typename Hash = std::hash<Key>,
        typename Pred = std::equal_to<Key>
        >
using map_type = std::unordered_map< Key, Value, Hash, Pred >;

int test_for_CacheLRU( Pool<key_type, map_type> *const pool );

int main() {

    Pool<key_type, map_type> pool;
    test_for_CacheLRU( &pool );

    return 0;
}

int test_for_CacheLRU( Pool<key_type, map_type> *const pool ) {

    // create a cache with capacity of 3 items.
    CacheLRU<key_type, map_type> cache( pool, 3 );
    assert( cache.getCacheSizeInNumItems() == 3 );
    assert( cache.getPool() == pool );
    assert( cache.getCache().size() == 0 );
    assert( cache.getCacheHits() == 0 );
    assert( cache.getCacheEvictions() == 0 );

    // create some test data.
    std::vector<int> testData( 10 );
    int value = 0;
    for ( auto & c : testData ) {
        std::cout << "Value: " << value << std::endl;
        c = value;
        value = 2 * value + 1;
    }

    // populate the pool with some items.
    for ( int i = 0 ; i < 10 ; ++i )
        pool->addItem( std::make_unique<item_type>( i, testData[i] ) );

    // benchmark cache performance by accessing some items.
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    // add new item with ID 6.
    auto item = cache.getItem( 6 );
    auto cacheOrder = cache.getCacheOrder();
    auto cacheMap = cache.getCache();
    assert( cacheOrder.size() == 1 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( dynamic_cast<DummyItem*>( cacheOrder.front() )->get() == testData[ 6 ] );

    // add new item with ID 5.
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( cacheMap.size() == 2 );
    assert( cacheOrder.size() == 2 );
    assert( cacheOrder.back()->getID() == 5 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( dynamic_cast<DummyItem*>( cacheOrder.back() )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheOrder.front() )->get() == testData[ 6 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );

    // add new item with ID 4.
    item = cache.getItem( 4 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( cacheMap.size() == 3 );
    assert( cacheOrder.size() == 3 );
    assert( cacheOrder.back()->getID() == 4 );
    assert( cacheOrder.front()->getID() == 6 );
    auto it = cacheOrder.begin();
    assert( dynamic_cast<DummyItem*>( *it )->get() == testData[ 6 ] );
    it = std::next( it );
    assert( dynamic_cast<DummyItem*>( *it )->get() == testData[ 5 ] );
    it = std::next( it );
    assert( dynamic_cast<DummyItem*>( *it )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 0 );

    // touch item 5.
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 1 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 5 );

    // touch item 5 (again).
    item = cache.getItem( 5 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 2 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 5 );

    // touch item 4.
    item = cache.getItem( 4 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 6 ) )->get() == testData[ 6 ] );
    assert( cache.getCacheEvictions() == 0 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 6 );
    assert( cacheOrder.back()->getID() == 4 );

    // add new item 7; should evict item 6.
    item = cache.getItem( 7 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 5 ) )->get() == testData[ 5 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 7 ) )->get() == testData[ 7 ] );
    assert( cache.getCacheEvictions() == 1 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 5 );
    assert( cacheOrder.back()->getID() == 7 );

    // add new item 9; should evict item 5.
    item = cache.getItem( 9 );
    cacheOrder = cache.getCacheOrder();
    cacheMap = cache.getCache();
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 4 ) )->get() == testData[ 4 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 7 ) )->get() == testData[ 7 ] );
    assert( dynamic_cast<DummyItem*>( cacheMap.at( 9 ) )->get() == testData[ 9 ] );
    assert( cache.getCacheEvictions() == 2 );
    assert( cache.getCacheHits() == 3 );
    assert( cacheOrder.front()->getID() == 4 );
    assert( cacheOrder.back()->getID() == 9 );

    // try to get an item that does not exist in the pool.
    std::string errorMsg;
    try {
        item = cache.getItem( 11 );
    } catch( std::exception const& e ) {
        errorMsg = e.what();
    }
    assert( errorMsg == "Not found item with ID 11 in the pool!" );

    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << "Benchmark completed in " << elapsedSeconds.count() << " s" << std::endl;

    return 0;
}