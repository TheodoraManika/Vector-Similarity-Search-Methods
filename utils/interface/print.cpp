#include "print.h"

namespace print {
    
    template <typename T>
    inline void vector_print(std::vector<T> vec) {
        std::cout << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << vec[i];
            if (i < vec.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
    
    template <typename T>
    inline void vector_print_infile(std::vector<T> vec, std::ofstream* out) {
        if (!out || !out->is_open()) {
            std::cerr << "Error: Output file is not open" << std::endl;
            return;
        }
        
        *out << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            *out << vec[i];
            if (i < vec.size() - 1) {
                *out << ", ";
            }
        }
        *out << "]" << std::endl;
    }
    
    template <typename T>
    inline void vector_it_print(std::vector<T> vec) {
        std::cout << "Vector contents with iterators:" << std::endl;
        auto it = vec.begin();
        int index = 0;
        for (; it != vec.end(); ++it, ++index) {
            std::cout << "Index " << index << ": " << *it << std::endl;
        }
    }
    
    template <typename T, typename M>
    inline void list_of_pairs_print(std::list<std::pair<T, M>> result) {
        std::cout << "List of pairs:" << std::endl;
        for (const auto& pair : result) {
            std::cout << "(" << pair.first << ", " << pair.second << ")" << std::endl;
        }
    }
    
    template <typename T, typename M>
    inline void same_line_list_of_pairs_print(std::list<std::pair<T, M>> result) {
        std::cout << "[";
        bool first = true;
        for (const auto& pair : result) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << "(" << pair.first << ", " << pair.second << ")";
            first = false;
        }
        std::cout << "]" << std::endl;
    }
    
    // Explicit template instantiations for common types
    template void vector_print<int>(std::vector<int>);
    template void vector_print<float>(std::vector<float>);
    template void vector_print<double>(std::vector<double>);
    template void vector_print<std::string>(std::vector<std::string>);
    
    template void vector_print_infile<int>(std::vector<int>, std::ofstream*);
    template void vector_print_infile<float>(std::vector<float>, std::ofstream*);
    template void vector_print_infile<double>(std::vector<double>, std::ofstream*);
    
    template void vector_it_print<int>(std::vector<int>);
    template void vector_it_print<float>(std::vector<float>);
    template void vector_it_print<double>(std::vector<double>);
    
    template void list_of_pairs_print<int, float>(std::list<std::pair<int, float>>);
    template void list_of_pairs_print<int, double>(std::list<std::pair<int, double>>);
    
    template void same_line_list_of_pairs_print<int, float>(std::list<std::pair<int, float>>);
    template void same_line_list_of_pairs_print<int, double>(std::list<std::pair<int, double>>);
}
