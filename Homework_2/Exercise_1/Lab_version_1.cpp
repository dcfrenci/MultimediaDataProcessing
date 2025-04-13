#include <fstream>
#include <iomanip>
#include <array>
#include <iostream>

int main(const int argc, const char** argv) {
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

    std::array<size_t, 256> count{};
    while (true) {
        int c = input.get();
        if (c == EOF) {
            break;
        }
        ++count[c];
    }
    for (size_t i = 0; i < 256; ++i) {
        output << std::hex << std::setw(2) << std::setfill('0') <<
            i << "\t" << std::dec << count[i] << "\n";
    }
    return EXIT_SUCCESS;
}
