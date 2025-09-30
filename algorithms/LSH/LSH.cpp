// LSH Hash Table (for Locality-Sensitive Hashing)
template<typename T>
class LSHHashTable : public HashTable<T> {
private:
    size_t k;  // Number of hash functions in composite key
    
public:
    LSHHashTable(size_t numHashFunctions) : HashTable<T>(), k(numHashFunctions) {}
    
    // Create composite key from multiple hash values
    std::string createKey(const std::vector<int>& hashValues) const {
        if (hashValues.size() != k) {
            throw std::invalid_argument("Hash values size must match k");
        }
        
        std::string key;
        for (size_t i = 0; i < hashValues.size(); ++i) {
            key += std::to_string(hashValues[i]);
            if (i < hashValues.size() - 1) {
                key += "_";
            }
        }
        return key;
    }
    
    // Insert with hash values
    void insertWithHash(const std::vector<int>& hashValues, const T& item) {
        std::string key = createKey(hashValues);
        this->insert(key, item);
    }
    
    // Query with hash values
    std::vector<T> queryWithHash(const std::vector<int>& hashValues) const {
        std::string key = createKey(hashValues);
        return this->query(key);
    }
    
    size_t getK() const { return k; }
    
    void printStatistics() const override {
        std::cout << "LSH Hash Table Statistics (k=" << k << "):" << std::endl;
        HashTable<T>::printStatistics();
    }
};