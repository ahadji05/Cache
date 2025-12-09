
#include "ItemBuffer.hpp"
#include "CacheLRU.hpp"
#include "Pool.hpp"
#include <cassert>
#include <chrono>

using key_type = int64_t;

using item_type = ItemBuffer< key_type >;

template< 
        typename Key, 
        typename Value,
        typename Hash = std::hash<Key>,
        typename Pred = std::equal_to<Key>
        >
using map_type = std::unordered_map< Key, Value, Hash, Pred >;

int main() {
try
{
    Pool<key_type, map_type> pool;
    CacheLRU<key_type, map_type> cache( &pool, 3 );

    // Populate the pool with some items.
    std::vector<int> basicData(10);
    int value = 0;
    for ( auto & c : basicData ) {
        std::cout << "Value: " << value << std::endl;
        c = value;
        value = 2 * value + 1;
    }
    for ( int i = 0 ; i < 10 ; ++i )
        pool.addItem( std::make_unique<item_type>( i, &basicData[i], sizeof( int ) ) );


    // Benchmark cache performance by accessing some items.
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    auto item = cache.getItem( 6 );
    item = cache.getItem( 5 );
    item = cache.getItem( 4 );
    item = cache.getItem( 5 );
    item = cache.getItem( 5 );
    item = cache.getItem( 5 );
    item = cache.getItem( 4 );
    item = cache.getItem( 5 );
    item = cache.getItem( 4 );
    item = cache.getItem( 6 );
    item = cache.getItem( 7 );
    item = cache.getItem( 4 );
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << "Benchmark completed in " << elapsedSeconds.count() << " s" << std::endl;

    std::cout << "kind: " << item->getKind() << std::endl;
    if ( item->getKind() == "ItemBuffer" ) {
        item_type * buffer = dynamic_cast<item_type*>( item );
        int * data = static_cast<int*>( buffer->get() );
        std::cout << "data: " << *data << std::endl;
    }

    std::cout << cache.getCache().size() << std::endl;
    std::cout << cache.getCacheOrder().size() << std::endl;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << '\n';
}
    return 0;
}