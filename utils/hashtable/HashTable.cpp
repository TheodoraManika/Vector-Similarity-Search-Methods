#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <algorithm>

// Base Hash Table class
template<typename T>
class HashTable {
protected:
    // Maps hash keys to vectors of items
    std::unordered_map<std::string, std::vector<T>> buckets;
    size_t numElements;
    
public:
    HashTable() : numElements(0) {}
    
    virtual ~HashTable() = default;
    
    // Insert an item with a given hash key
    virtual void insert(const std::string& hashKey, const T& item) {
        buckets[hashKey].push_back(item);
        numElements++;
    }
    
    // Query items in a specific bucket
    virtual std::vector<T> query(const std::string& hashKey) const {
        auto it = buckets.find(hashKey);
        if (it != buckets.end()) {
            return it->second;
        }
        return std::vector<T>();
    }
    
    // Get all items in a specific bucket (returns pointer, no copy)
    const std::vector<T>* getBucket(const std::string& hashKey) const {
        auto it = buckets.find(hashKey);
        if (it != buckets.end()) {
            return &(it->second);
        }
        return nullptr;
    }
    
    // Check if a bucket exists
    bool hasBucket(const std::string& hashKey) const {
        return buckets.find(hashKey) != buckets.end();
    }
    
    // Get bucket size
    size_t bucketSize(const std::string& hashKey) const {
        auto it = buckets.find(hashKey);
        if (it != buckets.end()) {
            return it->second.size();
        }
        return 0;
    }
    
    // Clear all buckets
    virtual void clear() {
        buckets.clear();
        numElements = 0;
    }
    
    // Get total number of elements
    size_t size() const {
        return numElements;
    }
    
    // Get number of buckets
    size_t bucketCount() const {
        return buckets.size();
    }
    
    // Check if empty
    bool empty() const {
        return numElements == 0;
    }
    
    // Get all hash keys
    std::vector<std::string> getAllKeys() const {
        std::vector<std::string> keys;
        keys.reserve(buckets.size());
        for (const auto& pair : buckets) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    // Print statistics
    virtual void printStatistics() const {
        std::cout << "Hash Table Statistics:" << std::endl;
        std::cout << "  Total elements: " << numElements << std::endl;
        std::cout << "  Number of buckets: " << buckets.size() << std::endl;
        
        if (!buckets.empty()) {
            size_t maxSize = 0;
            size_t minSize = SIZE_MAX;
            double avgSize = 0;
            
            for (const auto& pair : buckets) {
                size_t size = pair.second.size();
                maxSize = std::max(maxSize, size);
                minSize = std::min(minSize, size);
                avgSize += size;
            }
            
            avgSize /= buckets.size();
            
            std::cout << "  Min bucket size: " << minSize << std::endl;
            std::cout << "  Max bucket size: " << maxSize << std::endl;
            std::cout << "  Avg bucket size: " << avgSize << std::endl;
        }
    }
    
    // Iterator support
    typename std::unordered_map<std::string, std::vector<T>>::const_iterator begin() const {
        return buckets.begin();
    }
    
    typename std::unordered_map<std::string, std::vector<T>>::const_iterator end() const {
        return buckets.end();
    }
};