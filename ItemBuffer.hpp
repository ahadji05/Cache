#pragma once

#include "Item.hpp"

template< typename KeyType >
class ItemBuffer : public Item< KeyType > {

    private:
        void const * const m_sourceBuffer = nullptr;
        void       *       m_itemBuffer   = nullptr;
        size_t m_size = 0;

    public:
        ~ItemBuffer(){
            #ifdef DEBUG_CACHES
            std::cout << "Freeing...\n";
            #endif
            free( m_itemBuffer );
            m_itemBuffer = nullptr;
            m_size = 0;
        }

        ItemBuffer( ItemBuffer const& ) = delete;
        ItemBuffer & operator=( ItemBuffer const& ) = delete;

        ItemBuffer( KeyType itemID, void * sourceBuffer, size_t size ) : m_sourceBuffer( sourceBuffer ) {
            #ifdef DEBUG_CACHES
            std::cout << "Allocating...\n";
            #endif
            this->m_itemID = itemID;
            this->m_kind = "ItemBuffer";
            m_size = size;
            m_itemBuffer = malloc( this->m_size );
        }

        void load() override {
            memcpy( m_itemBuffer, m_sourceBuffer, m_size );
        }

        void * get() const {
            return m_itemBuffer;
        }

        void unload() override {}
};