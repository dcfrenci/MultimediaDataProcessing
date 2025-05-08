#include <fstream>
#include <iomanip>
#include <array>
#include <cstdint>
#include <iostream>
#include <map>

int main(const int argc, const char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1]);
    if (!input) {
        return EXIT_FAILURE;
    }
    std::ofstream output(argv[2]);
    if (!output) {
        return EXIT_FAILURE;
    }

    std::map<uint8_t, size_t> count;
    char c;
    while (input.get(c)) {
        uint8_t u = c;
        count[u] += 1;
    }

    for (const auto &[first, second]: count) {
        output << std::hex << std::setw(2) << std::setfill('0') <<
                static_cast<int>(first) << "\t" << std::dec << second << "\n";
    }
    return EXIT_SUCCESS;
}
