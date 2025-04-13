#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

int main(const int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return EXIT_FAILURE;
    }
    // Open files
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Error opening input file\n";
        return EXIT_FAILURE;
    }
    std::ofstream output(argv[2], std::ios::binary);
    if (!output) {
        std::cerr << "Error opening output file\n";
        return EXIT_FAILURE;
    }

    // Read and count each character
    input.unsetf(std::ios::skipws);                     //Extractor by default skip whitespaces -> Set to read them
    std::multiset<unsigned char> characters{
        std::istream_iterator<unsigned char>(input),
        std::istream_iterator<unsigned char>()
    };

    std::map<unsigned char, int> map;
    for (const auto& character : characters) {
        map[character]++;
    }

    for (const auto&[first, second] : map) {
        output << std::hex << std::setw(2) << std::setfill('0') <<
            static_cast<int>(first) << "\t" << second << "\n";
    }
    return EXIT_SUCCESS;
}
