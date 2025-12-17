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
        KeyType     m_itemID = UINT64_MAX;
        std::string m_kind   = "UDEF";

    public:
        using key_type = KeyType;

        virtual ~Item(){}
        Item(){}
        Item( KeyType id, std::string kind ) : m_itemID( id ), m_kind( kind ) {}

        KeyType getID() const {
            if ( m_itemID == UINT64_MAX )
                throw std::runtime_error( "UNDEFINED: m_itemID" );
            return m_itemID;
        }

        std::string getKind() const {
            if ( m_kind.empty() || m_kind == "UDEF" )
                throw std::runtime_error( "UNDEFINED: m_kind" );
            return m_kind;
        }

        void setID( KeyType const& id ){ m_itemID = id;   }
        void setKind( std::string const& kind ){ m_kind = kind; }

        // virtual public interface
        virtual void load()   = 0; // load item from pool to cache.
        virtual void unload() = 0; // unload from cache.
};
