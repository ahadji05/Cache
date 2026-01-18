# ItemCache
A flexible, header-only C++ library providing **Pool-backed caches** with multiple eviction policies. This library is designed to be **generic with respect to Key-type and Map-type**, enabling efficient caching solutions for a wide range of applications.

## Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [Implemented Cache Algorithms](#implemented-cache-algorithms)
- [Core Concepts](#core-concepts)
- [Getting Started: Creating a Custom Item](#getting-started-creating-a-custom-item)
- [Switching Between Map Types](#switching-between-map-types)
- [Building and Testing](#building-and-testing)
- [License](#license)

## Overview

This library provides a complete caching infrastructure where:
- **Pool** manages the entire collection of items that can potentially be cached
- **Cache** implementations (LRU, LFU, FIFO) maintain a subset of items from the Pool in memory
- **Item** is the base class you extend to create custom cacheable objects with `load()` and `unload()` logic

The architecture separates **storage management** (Pool) from **caching policy** (Cache implementations), allowing you to focus on defining what it means to load/unload your specific data.

## Key Features

✨ **Generic Design**: Works with any Key type (integers, strings, custom types) and any Map type (`std::map`, `std::unordered_map`, or custom implementations)

🎯 **Multiple Eviction Policies**: Choose the caching strategy that best fits your use case
- **LRU** (Least Recently Used): Best for temporal locality patterns
- **LFU** (Least Frequently Used): Best for frequency-based access patterns  
- **FIFO** (First In First Out): Simple and predictable eviction

🔌 **Extensible**: Create custom Item types by implementing just two methods: `load()` and `unload()`

⚡ **Header-Only**: Easy integration into your projects

🎨 **Template-Based**: Zero-cost abstractions with compile-time polymorphism

## Implemented Cache Algorithms

| Algorithm | Description | Best Use Case |
|-----------|-------------|---------------|
| **CacheLRU** | Evicts the least recently accessed item | Time-sensitive data with recent access patterns |
| **CacheLFU** | Evicts the least frequently accessed item | Frequency-based access patterns |
| **CacheFIFO** | Evicts the oldest item regardless of access | Simple, predictable eviction |

### Cache Algorithms Under Development
- Random Replacement (CacheRR)

## Core Concepts

### 1. Item (Item.hpp)
The base class for all cacheable objects. Any custom item must inherit from `Item<KeyType>` and implement:
```cpp
virtual void load()   = 0;  // Load item into memory/cache
virtual void unload() = 0;  // Unload item from memory/cache
```

### 2. Pool (Pool.hpp)
A container that holds **all** available items (both cached and uncached). The Pool:
- Stores items as `unique_ptr` (owns the items)
- Provides fast lookup by Key
- Is generic over KeyType and MapType

### 3. Cache (Cache.hpp)
Abstract base class for cache implementations. Each cache:
- Maintains a **non-owning** pointer to a Pool
- Keeps a subset of Pool items in the cache
- Implements a specific eviction policy
- Tracks cache hits and evictions

### 4. Cache Implementations
- **CacheLRU**: Uses a list to track access order, moving items to the back on access
- **CacheLFU**: Uses frequency maps to track access counts, evicting least frequent items
- **CacheFIFO**: Uses a deque to track insertion order, evicting oldest items

## Getting Started: Creating a Custom Item

Let's create a practical example: **caching database query results**. This is useful when you have expensive database queries that you want to cache in memory.

### Step 1: Define Your Custom Item Class

Create a class that inherits from `Item<KeyType>` and implements `load()` and `unload()`:

```cpp
#include "Item.hpp"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// Represents a database query result that can be cached
class QueryResult : public Item<std::string> {
private:
    std::string m_query;              // The SQL query
    std::vector<std::string> m_data;  // Cached result data
    bool m_loaded;                    // Track if data is loaded

    // Simulate database query execution by reading from a mock data source
    std::vector<std::string> executeQuery(const std::string& query) {
        std::vector<std::string> results;
        
        // Simulate network/disk I/O delay (expensive operation)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Parse query to determine what data to return
        // In real code, this would connect to a database
        if (query.find("users") != std::string::npos) {
            results.push_back("id=1,name=Alice,email=alice@example.com");
            results.push_back("id=2,name=Bob,email=bob@example.com");
            results.push_back("id=3,name=Charlie,email=charlie@example.com");
        } else if (query.find("products") != std::string::npos) {
            results.push_back("id=101,name=Laptop,price=999.99");
            results.push_back("id=102,name=Mouse,price=29.99");
            results.push_back("id=103,name=Keyboard,price=79.99");
        } else if (query.find("COUNT") != std::string::npos) {
            results.push_back("count=42");
        } else {
            results.push_back("Generic result row 1");
            results.push_back("Generic result row 2");
        }
        
        return results;
    }

public:
    // Constructor: takes query string as ID
    QueryResult(const std::string& query) 
        : m_query(query), m_loaded(false) {
        this->m_itemID = query;  // Use query as the Key
    }

    // Load: Execute the database query and store results in memory
    void load() override {
        if (m_loaded) return;  // Already loaded
        
        std::cout << "📥 Executing query: " << m_query << std::endl;
        
        // Actually fetch data from the simulated data source
        // This demonstrates loading data from "somewhere" (simulated DB)
        m_data = executeQuery(m_query);
        
        std::cout << "   Loaded " << m_data.size() << " rows" << std::endl;
        m_loaded = true;
    }

    // Unload: Free the cached data from memory
    void unload() override {
        if (!m_loaded) return;  // Already unloaded
        
        std::cout << "📤 Unloading query results: " << m_query << std::endl;
        m_data.clear();  // Free memory
        m_loaded = false;
    }

    // Accessor to get the cached results
    const std::vector<std::string>& getData() const {
        if (!m_loaded) {
            throw std::runtime_error("Cannot access data before loading!");
        }
        return m_data;
    }
    
    // Get number of result rows
    size_t getRowCount() const {
        if (!m_loaded) {
            throw std::runtime_error("Cannot access data before loading!");
        }
        return m_data.size();
    }
};
```

**Key Points:**
- ✅ **Must inherit** from `Item<KeyType>` (here `KeyType = std::string`)
- ✅ **Must set** `this->m_itemID` in the constructor
- ✅ **Must implement** `load()`: allocate resources, fetch data, etc.
- ✅ **Must implement** `unload()`: free resources, clear data, etc.
- ✅ **Should be safe** to call `load()` and `unload()` multiple times

### Step 2: Create a Pool and Add Items

```cpp
#include "Pool.hpp"
#include <memory>

int main() {
    // Create a Pool to hold all possible query results
    // Using std::string as Key and std::map as the underlying map
    Pool<std::string, std::map> queryPool;
    
    // Add query items to the pool
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM users"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM products"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM orders"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT COUNT(*) FROM users"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT AVG(price) FROM products"));
    
    std::cout << "Pool contains " << queryPool.getPoolSize() << " queries\n";
    
    // ... continue to Step 3
}
```

### Step 3: Create a Cache with Your Preferred Policy

```cpp
#include "CacheLRU.hpp"
// or #include "CacheLFU.hpp"
// or #include "CacheFIFO.hpp"

int main() {
    // ... (Pool creation from Step 2)
    
    // Create an LRU cache that holds at most 2 queries in memory
    CacheLRU<std::string, std::map> queryCache(&queryPool, 2);
    
    // Access queries through the cache and get the data
    auto* result1 = queryCache.getItem("SELECT * FROM users");
    QueryResult* queryResult = dynamic_cast<QueryResult*>(result1);
    
    // Access the loaded data
    std::cout << "First result row: " << queryResult->getData()[0] << "\n";
    std::cout << "Total rows: " << queryResult->getRowCount() << "\n";
    
    // This query is now loaded and cached
    auto* result2 = queryCache.getItem("SELECT * FROM products");
    QueryResult* productResult = dynamic_cast<QueryResult*>(result2);
    std::cout << "Product data: " << productResult->getData()[0] << "\n";
    
    // Cache is full (2 items), next query will evict the LRU item
    auto* result3 = queryCache.getItem("SELECT * FROM orders");
    // "SELECT * FROM users" was evicted (least recently used)
    
    // Accessing the first query again will load it back
    result1 = queryCache.getItem("SELECT * FROM users");
    // "SELECT * FROM products" was evicted this time
    
    std::cout << "Cache hits: " << queryCache.getCacheHits() << "\n";
    std::cout << "Cache evictions: " << queryCache.getCacheEvictions() << "\n";
    
    return 0;
}
```

### Step 4: Understanding the Flow

1. **First access** to `"SELECT * FROM users"`:
   - Cache misses → searches Pool → finds item
   - Calls `item->load()` → executes query, stores results
   - Adds item to cache → returns pointer

2. **Subsequent accesses** to same query:
   - Cache hits → returns cached pointer immediately
   - No database query executed!
   - Updates eviction policy metadata (e.g., moves to back in LRU)

3. **Cache full** + new query:
   - Cache must evict according to policy (LRU/LFU/FIFO)
   - Calls `item->unload()` on evicted item → frees memory
   - Loads and caches the new item

### Complete Working Example

Here's a full program you can compile and run:

```cpp
#include "Item.hpp"
#include "Pool.hpp"
#include "CacheLRU.hpp"
#include <iostream>
#include <vector>
#include <string>

class QueryResult : public Item<std::string> {
private:
    std::string m_query;
    std::vector<std::string> m_data;
    bool m_loaded;

public:
    QueryResult(const std::string& query) : m_query(query), m_loaded(false) {
        this->m_itemID = query;
    }

    void load() override {
        if (m_loaded) return;
        std::cout << "📥 Loading: " << m_query << std::endl;
        m_data = {"Row1", "Row2", "Row3"};  // Simulate query execution
        m_loaded = true;
    }

    void unload() override {
        if (!m_loaded) return;
        std::cout << "📤 Unloading: " << m_query << std::endl;
        m_data.clear();
        m_loaded = false;
    }

    const std::vector<std::string>& getData() const {
        if (!m_loaded) throw std::runtime_error("Data not loaded!");
        return m_data;
    }
};

int main() {
    Pool<std::string, std::map> queryPool;
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM users"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM products"));
    queryPool.addItem(std::make_unique<QueryResult>("SELECT * FROM orders"));

    CacheLRU<std::string, std::map> cache(&queryPool, 2);

    std::cout << "\n=== Accessing 3 queries with cache size 2 ===\n";
    cache.getItem("SELECT * FROM users");
    cache.getItem("SELECT * FROM products");
    cache.getItem("SELECT * FROM orders");  // Will evict "users"

    std::cout << "\n=== Re-accessing first query ===\n";
    cache.getItem("SELECT * FROM users");  // Will evict "products"

    std::cout << "\n📊 Cache Statistics:\n";
    std::cout << "   Hits: " << cache.getCacheHits() << "\n";
    std::cout << "   Evictions: " << cache.getCacheEvictions() << "\n";

    return 0;
}
```

**Expected Output:**
```
=== Accessing 3 queries with cache size 2 ===
📥 Loading: SELECT * FROM users
📥 Loading: SELECT * FROM products
📤 Unloading: SELECT * FROM users
📥 Loading: SELECT * FROM orders

=== Re-accessing first query ===
📤 Unloading: SELECT * FROM products
📥 Loading: SELECT * FROM users

📊 Cache Statistics:
   Hits: 0
   Evictions: 2
```

## Switching Between Map Types

One of the most powerful features of this library is the ability to easily switch the underlying map implementation. This is achieved through template parameters.

### Using std::map (Ordered Map)

```cpp
Pool<int, std::map> pool;
CacheLRU<int, std::map> cache(&pool, 100);
```

**Characteristics:**
- ✅ Keys are sorted
- ✅ O(log n) lookup, insertion, deletion
- ✅ Lower memory overhead
- ❌ Slower than unordered_map for large datasets

### Using std::unordered_map (Hash Map)

```cpp
#include <unordered_map>

Pool<int, std::unordered_map> pool;
CacheLRU<int, std::unordered_map> cache(&pool, 100);
```

**Characteristics:**
- ✅ O(1) average-case lookup, insertion, deletion
- ✅ Faster for large datasets
- ❌ Keys are not sorted
- ❌ Higher memory overhead

### Switching is a One-Line Change!

The beauty of this design is that switching between map types requires **only changing the template parameter**:

```cpp
// Original code using std::map
Pool<std::string, std::map> pool;
CacheLRU<std::string, std::map> cache(&pool, 50);

// Switch to std::unordered_map by changing ONE word:
Pool<std::string, std::unordered_map> pool;
CacheLRU<std::string, std::unordered_map> cache(&pool, 50);
```

No other code changes required! Your custom Item class works with both.

### Using Custom Map Types

You can even provide your own map implementation, as long as it supports the same interface as `std::map`:
- `size()`, `count()`, `insert()`, `erase()`, `find()`, `at()`, `begin()`, `end()`

```cpp
// Example with a custom map type
template<typename K, typename V>
class MyCustomMap {
    // Your implementation with std::map-compatible interface
};

Pool<int, MyCustomMap> pool;
CacheLRU<int, MyCustomMap> cache(&pool, 100);
```

## Building and Testing

### Compile the Tests

```bash
make tests.exe
./tests.exe
```

This runs comprehensive tests for all three cache implementations (LRU, LFU, FIFO).

### Compile the File Buffer Example

```bash
make exampleFileBuffers.exe
./exampleFileBuffers.exe
```

This demonstrates caching file buffers loaded from disk - a practical example of using Pool-backed caches for I/O operations.

### Clean Build Artifacts

```bash
make clean
```

## License

See [LICENSE](LICENSE) file for details.
