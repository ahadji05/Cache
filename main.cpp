
#include "ItemBuffer.hpp"
#include "CacheLRU.hpp"
#include "Pool.hpp"
#include <cassert>
#include <chrono>

int main() {
try
{
    Pool pool;

    std::vector<int> basicData(10);
    int value = 0;
    for ( auto & c : basicData ) {
        std::cout << "Value: " << value << std::endl;
        c = value;
        value = 2 * value + 1;
    }

    for ( int i = 0 ; i < 10 ; ++i )
        pool.addItem( std::make_unique<ItemBuffer>( i, &basicData[i], sizeof( int ) ) );

    CacheLRU cache( &pool, 3 );

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

    std::cout << *reinterpret_cast<int*>( item->get() ) << std::endl;

    std::cout << cache.getCache().size() << std::endl;
    std::cout << cache.getCacheOrder().size() << std::endl;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << '\n';
}
    return 0;
}