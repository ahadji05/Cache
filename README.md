# ItemCache
A generic library for effiicient querying of cachable items across different memory spaces.

the Item should contain in void * m_data the cached data. 
The virtual function load() is specific to the use case such 
that it loads the data from some source address/file/buffer 
into m_data.

Using this design i can implement caching on cpu from disk data,
or caching on gpu from cpu data, 
or caching in one data structure from another,
etc.

Each Item specilization should provide enough information in the 
object so that the overiden load() function can load the data in the 
cache.

The following Cache methods should not be virtual, they
should insteade be implemented in the Base class so that
not derived class will have to reimplement them.

Item * searchPool      ( int64_t itemID );
        
Item * searchCache     ( int64_t itemID );

The methods:

void loadItemFromPool( Item & item );

void addItemToCache( Item & item );

void touchItem( Item & item );

should be normal functiona implemented in the body
of each derived class.

The only virtual method in a Cache subclass should be 
Item * getItem( int64_t itemID ) = 0;


