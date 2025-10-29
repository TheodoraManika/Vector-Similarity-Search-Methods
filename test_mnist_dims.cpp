#include "dataprocessing/MNIST/MNIST.h"
#include <iostream>

int main() {
    auto dataset = MNIST::parseMNISTImages("dataset/train-images.idx3-ubyte", 1);
    
    std::cout << "Number of images: " << dataset.size() << std::endl;
    std::cout << "Dimension of first image: " << dataset[0].size() << std::endl;
    std::cout << "Expected: 28 x 28 = " << (28 * 28) << std::endl;
    
    return 0;
}
