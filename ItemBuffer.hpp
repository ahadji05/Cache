#pragma once

#include "Item.hpp"

class ItemBuffer : public Item {

    private:
        void const * const m_sourceBuffer = nullptr;
        void       *       m_itemBuffer   = nullptr;
        size_t m_size = 0;

    public:
        ItemBuffer(){}

        ~ItemBuffer(){
            std::cout << "Freeing...\n";
            free( m_itemBuffer );
            m_itemBuffer = nullptr;
            m_size = 0;
        }

        ItemBuffer( ItemBuffer const& ) = delete;
        ItemBuffer & operator=( ItemBuffer const& ) = delete;

        ItemBuffer( int64_t itemID, void * sourceBuffer, size_t size ) : m_sourceBuffer( sourceBuffer ) {
            std::cout << "Allocating...\n";
            this->m_itemID = itemID;
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
        void * serialize() override {
            return m_itemBuffer;
        }
        void   deserialize( void * ) override {}
};