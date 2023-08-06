/*
Copyright 2023 AsyncFor
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include "vectorclass.h"
#include <vector>
#include <chrono>
struct Buffer {
    std::uint8_t* ptr;
    size_t length;
};

class Signature {
    public:
        std::vector<std::uint8_t> pattern;
        std::vector<std::uint8_t> mask;
        Signature(std::vector<std::int16_t> pattern);
        std::vector<std::uint32_t> get_pattern_u32();
};

Signature::Signature(std::vector<std::int16_t> pattern) {
    if (pattern.size() > 16) {
        throw std::invalid_argument("pattern size must be less than 16 - for now");
    }

    std::vector<std::uint8_t> mask;
    for (auto i : pattern) {
        if (i == -1) {
            mask.push_back(0xff);
        } else {
            mask.push_back(0x00);
        }
    }

    std::vector<std::uint8_t> pattern_u8;
    for (auto i : pattern) {
        pattern_u8.push_back(i & 0xff);
    }

    this->pattern = pattern_u8;
    this->mask = mask;
}

int search(Buffer buffer, Signature signature) {
    Vec64c pattern_vector = Vec64c().load_partial(signature.pattern.size(), signature.pattern.data());
    Vec64c mask_vector = Vec64c().load_partial(signature.mask.size(), signature.mask.data());

    mask_vector = ~mask_vector;
    pattern_vector = pattern_vector & mask_vector;

    for (int i = 0; i < buffer.length; i++) {
        Vec64c current_search_buffer = Vec64c().load_partial(signature.pattern.size(), buffer.ptr + i);
        current_search_buffer = current_search_buffer & mask_vector;
        Vec64c result = current_search_buffer ^ pattern_vector;
        if (horizontal_or(result) == 0) {
            return i;
        }
    }
    return -1;
}

int main() {
    constexpr char* filename = "test.txt";
    std::vector<std::int16_t> pattern = {0xff, 0xff, 0xff, -1, 0xff}; // change it to pattern, -1 is wildcard '?'

    // -- load file into buffer --
    std::ifstream file(filename, std::ios::binary);
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    std::cout << "length of file: " << length << std::endl;
    std::vector<char> buffer(length);
    file.read(buffer.data(), length);
    
    Signature signature(pattern);
    Buffer file_data = {reinterpret_cast<std::uint8_t*>(buffer.data()), length};
    
    std::chrono::high_resolution_clock clock;
    
    auto start = clock.now();
    int result = search(file_data, signature);
    auto end = clock.now();
    
    std::cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
    std::cout << "result: " << result << std::endl;
}