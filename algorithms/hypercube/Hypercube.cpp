// Hypercube Hash Table (for Random Projection Hypercube)
template<typename T>
class HypercubeHashTable : public HashTable<T> {
private:
    size_t d;  // Dimension of hypercube (number of bits)
    
    // Generate all binary strings within Hamming distance
    void generateHammingNeighborsRecursive(std::string current, size_t remaining, 
                                           size_t pos, std::vector<std::string>& results) const {
        if (remaining == 0) {
            return;
        }
        
        for (size_t i = pos; i < current.length(); ++i) {
            // Flip bit at position i
            std::string flipped = current;
            flipped[i] = (flipped[i] == '0') ? '1' : '0';
            
            results.push_back(flipped);
            
            // Recursively flip more bits
            if (remaining > 1) {
                generateHammingNeighborsRecursive(flipped, remaining - 1, i + 1, results);
            }
        }
    }
    
    std::vector<std::string> generateHammingNeighbors(const std::string& hashKey, size_t radius) const {
        std::vector<std::string> neighbors;
        neighbors.push_back(hashKey);
        
        if (radius == 0) {
            return neighbors;
        }
        
        generateHammingNeighborsRecursive(hashKey, radius, 0, neighbors);
        
        return neighbors;
    }
    
public:
    HypercubeHashTable(size_t hypercubeDimension) : HashTable<T>(), d(hypercubeDimension) {}
    
    // Create binary key from binary hash values
    std::string createKey(const std::vector<int>& binaryValues) const {
        if (binaryValues.size() != d) {
            throw std::invalid_argument("Binary values size must match d");
        }
        
        std::string key;
        for (int val : binaryValues) {
            key += (val == 0) ? '0' : '1';
        }
        return key;
    }
    
    // Insert with binary hash values
    void insertWithHash(const std::vector<int>& binaryValues, const T& item) {
        std::string key = createKey(binaryValues);
        this->insert(key, item);
    }
    
    // Query with binary hash values (exact match)
    std::vector<T> queryWithHash(const std::vector<int>& binaryValues) const {
        std::string key = createKey(binaryValues);
        return this->query(key);
    }
    
    // Query with Hamming distance
    std::vector<T> queryHamming(const std::string& hashKey, size_t hammingRadius) const {
        std::vector<T> results;
        
        // Generate all keys within Hamming distance
        std::vector<std::string> neighborKeys = generateHammingNeighbors(hashKey, hammingRadius);
        
        for (const auto& key : neighborKeys) {
            auto items = this->query(key);
            results.insert(results.end(), items.begin(), items.end());
        }
        
        return results;
    }
    
    // Query with Hamming distance using binary values
    std::vector<T> queryHammingWithHash(const std::vector<int>& binaryValues, size_t hammingRadius) const {
        std::string key = createKey(binaryValues);
        return queryHamming(key, hammingRadius);
    }
    
    // Calculate Hamming distance between two keys
    static size_t hammingDistance(const std::string& key1, const std::string& key2) {
        if (key1.length() != key2.length()) {
            throw std::invalid_argument("Keys must have same length");
        }
        
        size_t distance = 0;
        for (size_t i = 0; i < key1.length(); ++i) {
            if (key1[i] != key2[i]) {
                distance++;
            }
        }
        return distance;
    }
    
    size_t getD() const { return d; }
    
    void printStatistics() const override {
        std::cout << "Hypercube Hash Table Statistics (d=" << d << "):" << std::endl;
        HashTable<T>::printStatistics();
        std::cout << "  Max possible buckets: " << (1ULL << d) << std::endl;
    }
};