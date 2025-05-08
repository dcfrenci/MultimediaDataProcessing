#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <math.h>
#include <vector>

int main(const int argc, const char *argv[]) {
    if (argc < 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[2], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    std::ofstream output(argv[3], std::ios::binary);
    if (!output) {
        return EXIT_FAILURE;
    }
    if (argv[1][0] == 'c') {    // Compression
        std::vector<unsigned int> mapping;
        std::vector<int> numbers{
            std::istream_iterator<int>(input),
            std::istream_iterator<int>()
        };
        for (auto &value: numbers) {
            if (value == 0) {
                mapping.push_back(1);
            } else if (value > 0) {
                mapping.push_back(2 * value + 1);
            } else {
                mapping.push_back(-2 * value);
            }
        }
        int characters = 0;
        for (const auto &x: mapping) {
            std::string zero(std::floor(log2(x)), '0');
            std::string binary = std::bitset<32>(x).to_string().substr(32 - static_cast<int>(std::floor(log2(x))) - 1);
            output << zero << binary;
            characters += zero.size() + binary.size();
        }
        output << std::string((8 - characters % 8) % 8, '0');
    } else {    // Decompression
        bool reading = false;
        int counter = 0;
        std::string binary;
        while (input.good()) {
            char value = input.get();
            if (!reading) {
                if (value == '0') {
                    counter++;
                    continue;
                }
                binary.push_back(value);
                reading = true;
            } else {
                binary.push_back(value);
            }
            if (binary.size() > counter) {
                int number = std::bitset<32>(std::string(32 - binary.size(), '0') + binary).to_ulong();
                reading = false;
                counter = 0;
                binary.clear();
                if (number == 1) {
                    output << '0' << '\n';
                } else if (number % 2 == 0) {
                    output << -(number / 2) << '\n';
                } else {
                    output << (number - 1) / 2 << '\n';
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
