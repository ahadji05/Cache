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
        std::string m_kind;

    public:
        virtual ~Item(){}
        Item(){}
        Item( KeyType id, std::string kind ) : m_itemID( id ), m_kind( kind ) {}
        KeyType getID() const             { return m_itemID; }
        void    setID( KeyType const& id ){ m_itemID = id;   }
        std::string getKind() const       { return m_kind;  }
        void        setKind( std::string const& kind ){ m_kind = kind; }

        // virtual public interface
        virtual void   load()   = 0;               // load item from pool to cache.
        virtual void   unload() = 0;               // unload from cache.
        virtual void * get() const = 0;            // get read-only access.
};
