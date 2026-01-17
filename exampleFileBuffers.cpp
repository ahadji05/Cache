
/**
 * In this example, we demonstrate how to make use of a Pool-backed Cache to load on 
 * demand buffers from a file on disk and keep as many buffers in memory as allowed 
 * by the Cache's capacity.
 *
 * For algorithms that require to read data from files, and loading the entire file
 * into memory is not feasible, using a Pool-backed Cache to load file buffers on 
 * demand can be an effective solution because it allows to keep frequently accessed
 * buffers in memory while evicting less frequently used ones when the Cache reaches
 * its capacity.
 *
 * Example:
 *
 * In this example we assume we have a large binary file "data.bin" that contains
 * a sequence of float numbers. We want to read this file in chunks (buffers) of
 * a fixed size (e.g., 11 bytes) and process each buffer. We will use a Pool-backed
 * Cache to manage these buffers efficiently. 
 *
 * The first step to implement this, is to derive from class Item a custom class FileBuffer
 * that keeps the position of a unique buffer in the file and loads it when load() is called.
 * Load essentially allocates memory for the buffer and reads the corresponding data from 
 * the file into memory. Unloading the buffer simply frees the allocated memory. The latter
 * is done when unload() is called.
 *
 * Then, a Pool is created that contains all the FileBuffer items, each corresponding to a
 * specific buffer in the file. A Cache is then created on top of this Pool with a specified
 * capacity (e.g., 50 buffers).
 *
 * For testing purposes, we have a setup() function that creates a simple binary file: "data.bin" 
 * with chars that store numbers between 0 and 255. The first buffer will contain chars '0', the
 * second '1', and so on. The file contains 1000 buffers, each one of size 2000 bytes.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "Item.hpp"
#include "Pool.hpp"
#include "CacheLRU.hpp"
#include "CacheLFU.hpp"
#include "CacheFIFO.hpp"
#include <chrono>
#include <cassert>

// Custom item that loads and unloads a buffer from a file-stream.
class FileBuffer : public Item< size_t > {

    private:
        std::string   m_fileName;
        size_t        m_bufferOffsetInFile; // Offset of the buffer in the file.
        size_t        m_bufferSizeInBytes;  // Size of the buffer in bytes.
        char *        m_buffer;             // Buffer to hold the loaded data.

    public:
        // Delete the copy constructor and assignment operator to prevent copying.
        FileBuffer( const FileBuffer& ) = delete;
        FileBuffer& operator=( const FileBuffer& ) = delete;

        // Constructor.
        FileBuffer( const std::string& filename, size_t offsetInFile, size_t sizeInBytes ) 
            : m_fileName( filename ), m_bufferOffsetInFile( offsetInFile ), m_bufferSizeInBytes( sizeInBytes ), m_buffer( nullptr ) {

            // Use buffer index as ID.
            this->m_itemID = offsetInFile / sizeInBytes;
        }

        // Destructor.
        ~FileBuffer() noexcept {
            unload();
        }

        // Load the buffer from the file.
        void load() override {

            // Already loaded; early return.
            if ( m_buffer )
                return;

            // Allocate memory for the buffer.
            m_buffer = ( char * ) std::malloc( m_bufferSizeInBytes );

            std::ifstream m_fileStream;
            m_fileStream.open( m_fileName, std::ios::binary );
            if ( !m_fileStream )
                throw std::runtime_error( "Failed to open file: " + m_fileName );

            // Check file-stream access.
            if ( !m_fileStream )
                throw std::runtime_error( "The file-stream is not open." );

            // Move to the specified offset in the file.
            m_fileStream.seekg( m_bufferOffsetInFile, std::ios::beg );
            if ( !m_fileStream )
                throw std::runtime_error( "Failed to seek to offset: " + std::to_string( m_bufferOffsetInFile ) );

            // Read the buffer.
            m_fileStream.read( m_buffer, m_bufferSizeInBytes );
            if ( !m_fileStream )
                throw std::runtime_error( "Failed to read data from the file-stream." );

            m_fileStream.close();
        }

        // Unload the buffer (clear it).
        void unload() override {
            if ( m_buffer != nullptr ) {
                std::free( m_buffer );
                m_buffer = nullptr;
            }
        }

        // Accessor for the buffer.
        const char * getBuffer() const {
            return m_buffer;
        }
};

// Function to setup a test binary file with sample data.
void setup( const std::string& filename, size_t totalBuffers, size_t bufferSizeInBytes ) {

    std::ofstream outFile( filename, std::ios::binary );
    if ( !outFile )
        throw std::runtime_error( "Failed to create test file: " + filename );

    for ( size_t i = 0; i < totalBuffers; ++i ) {
        int value = i % 100;
        std::vector<char> buffer( bufferSizeInBytes, static_cast<char>( value ) );
        outFile.write( buffer.data(), bufferSizeInBytes );
    }
    outFile.close();
}

// Example parameters.
const std::string filename = "data.bin";
const size_t      bufferSizeInBytes = 1000;    // Size of each buffer.
const size_t      totalBuffers      = 5000000; // Total number of buffers in the file.
const size_t      cacheCapacity     = 200000;  // Capacity of the Cache (number of buffers).

int main() {

    try {
        // Setup test file; roughly 5 GB file with 5 million buffers, each one of size 1000 bytes.
        setup( filename, totalBuffers, bufferSizeInBytes );

        // Create a Pool of FileBuffer items using size_t as KeyType.
        Pool< size_t, std::unordered_map > fileBufferPool;

        for ( size_t i = 0; i < totalBuffers; ++i ) {
            size_t offset = i * bufferSizeInBytes;
            fileBufferPool.addItem( std::make_unique< FileBuffer >( filename, offset, bufferSizeInBytes ) );
        }

        // Create a Cache on top of the Pool with the same KeyType == size_t.
        CacheLRU< size_t, std::unordered_map > fileBufferCache( &fileBufferPool, cacheCapacity );

        // Use current time as seed for random generator.
        std::srand( std::time( {} ) );

        // Access and process buffers using the Cache.
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        for ( size_t i = 0; i < 300000; ++i ) {

            // Randomly access buffers.
            size_t id = std::rand() % totalBuffers;

            const char * bufferData = dynamic_cast< FileBuffer* >( fileBufferCache.getItem( id ) )->getBuffer();

            // For testing purposes, we assert that the first byte of the buffer matches the expected value.
            // The expected value is (id % 100) as per the setup function.
            assert( static_cast<int>( bufferData[ 0 ] ) == ( id % 100 ) );
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time = end - begin ;

        std::cout << "Completed buffer accesses using Pool-backed Cache." << std::endl;
        std::cout << "Final Cache Hits: " << fileBufferCache.getCacheHits() << std::endl;
        std::cout << "Final Cache Evictions: " << fileBufferCache.getCacheEvictions() << std::endl;
        std::cout << "Time taken: " << ( double ) ( std::chrono::duration_cast<std::chrono::milliseconds>( time ).count() ) / 1e3 << " seconds" << std::endl;

    } catch ( const std::exception& e ) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}