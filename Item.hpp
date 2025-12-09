#pragma once

#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <list>
#include <map>

class Item {

    protected:
        int64_t m_itemID;

    public:
        Item(){}
        Item( int64_t id ) : m_itemID( id ) {}
        int64_t getID() const             { return m_itemID; }
        void    setID( int64_t const& id ){ m_itemID = id;   }
        virtual ~Item(){}

        // virtual public interface
        virtual void   load()   = 0;               // load item from pool to cache.
        virtual void   unload() = 0;               // unload from cache.
        virtual void * get() const = 0;            // get read-only access.
        virtual void * serialize() = 0;            // packs the item in a byte-array so it can be returned with get().
        virtual void   deserialize( void * ) = 0;  // unpacks whatever serialize() packs.
};
