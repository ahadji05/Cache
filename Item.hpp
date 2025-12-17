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

        // ID of the item.
        KeyType m_itemID;

    public:
        using key_type = KeyType;

        Item();
        virtual ~Item() {}

        Item( KeyType id );
        KeyType getID() const;

        virtual void load()   = 0; // load to cache.
        virtual void unload() = 0; // unload from cache.
};


/*********************************
 * PUBLIC INTERFACE IMPLEMENTATION
 ********************************/

template< typename KeyType >
Item<KeyType>::Item()
{
}

template< typename KeyType >
Item<KeyType>::Item( KeyType id ) : m_itemID( id )
{
}

template< typename KeyType >
KeyType
Item<KeyType>::getID() const
{
    return m_itemID;
}
