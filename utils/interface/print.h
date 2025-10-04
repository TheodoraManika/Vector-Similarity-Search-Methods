#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <list>

/**
Print functions
**/
using namespace std;

namespace print {
    template <typename T> 
    // print vector
    inline void vector_print(std::vector<T> vec);
    
    template <typename T> 
    // print vector in file
    inline void vector_print_infile(std::vector<T> vec, ofstream* out);
    
    // print vector and iterator
    template <typename T>
    inline void vector_it_print(std::vector<T> vec);

    //print list of pairs
    template <typename T, typename M>
    inline void list_of_pairs_print(std::list<std::pair<T,M>> result);

    //print list of pairs same line
    template <typename T, typename M>
    inline void same_line_list_of_pairs_print(std::list<std::pair<T,M>> result);
};