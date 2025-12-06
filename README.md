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

