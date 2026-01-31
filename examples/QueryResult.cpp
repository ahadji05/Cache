#include "kash/Item.hpp"
#include "kash/CacheLRU.hpp"
#include "kash/Pool.hpp"

#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace kash;

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
        
        std::cout << "Executing query: " << m_query << std::endl;
        
        // Actually fetch data from the simulated data source
        // This demonstrates loading data from "somewhere" (simulated DB)
        m_data = executeQuery(m_query);
        
        std::cout << "   Loaded " << m_data.size() << " rows" << std::endl;
        m_loaded = true;
    }

    // Unload: Free the cached data from memory
    void unload() override {
        if (!m_loaded) return;  // Already unloaded
        
        std::cout << "Unloading query results: " << m_query << std::endl;
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

    // Accessing the same query again will hit the cache
    result1 = queryCache.getItem("SELECT * FROM users");

    std::cout << "Cache hits: " << queryCache.getCacheHits() << "\n";
    std::cout << "Cache evictions: " << queryCache.getCacheEvictions() << "\n";

    return 0;
}