#pragma once

#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <list>
#include <map>

template< typename KeyType >
class Item {

    protected:
        KeyType m_itemID;

    public:
        using key_type = KeyType;

        virtual ~Item(){}
        Item(){}
        Item( KeyType id ) : m_itemID( id ) {}
        KeyType getID() const { return m_itemID; }

        // virtual public interface
        virtual void load()   = 0; // load item from pool to cache.
        virtual void unload() = 0; // unload from cache.
};
