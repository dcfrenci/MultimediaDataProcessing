#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <vector>

bool replace_range(const char c) {
    return c < 32 or c > 126;
}

std::string read_string(std::ifstream &input, const char c) {
    std::string dim;
    std::getline(input, dim, ':');
    std::string string(std::stoi(std::string(1, c) + dim), '\0');
    input.read(string.data(), std::stoi(std::string(1, c) + dim));
    std::replace_if(string.begin(), string.end(), replace_range, '.');
    return string;
}

std::string read_int(std::ifstream &input) {
    std::string string;
    std::getline(input, string, 'e');
    return string;
}

void recursive(std::ifstream &input, const uint32_t level, const bool dictionary, bool key, bool pieces) { // NOLINT(*-no-recursion)
    char c;
    input.read(&c, 1);
    if (input.eof() or c == 'e') {
        return;
    }
    const std::string string = key? " => " : input.tellg() == 1? std::string(level, '\t') : "\n" + std::string(level, '\t');
    std::cout << string;
    if (dictionary) {
        key = !key;
    }
    if (c == 'i') {
        std::cout << read_int(input);
    } else if (c == 'l') {
        std::cout << "[";
        recursive(input, level + 1, false, key, false);
        std::cout << "\n" + std::string(level, '\t') + "]";
    } else if (c == 'd') {
        std::cout << "{";
        recursive(input, level + 1, true, key, false);
        std::cout << "\n" + std::string(level, '\t') + "}";
    } else {
        if (pieces) {
            std::vector<uint8_t> hex(20);
            std::string dim;
            std::getline(input, dim, ':');
            const uint32_t max = std::stoi(std::string(1, c) + dim);
            for (uint32_t i = 0; i < max; i += 20) {
                input.read(reinterpret_cast<std::istream::char_type *>(hex.data()), 20);
                std::cout << '\n' + std::string(level + 1, '\t');
                std::for_each(hex.begin(), hex.end(), [](const uint32_t value) {value <= 15? std::cout << '0' << std::hex << value : std::cout << std::hex << value;});
                }
            pieces = false;
        } else {
            const std::string str = read_string(input, c);
            if (str == "pieces" and key) {
                pieces = true;
            }
            std::cout << "\"" + str + "\"";
        }
    }
    recursive(input, level, dictionary, key, pieces);
}

int dump(std::ifstream &input) {
    recursive(input, 0, false, false, false);
    return EXIT_SUCCESS;
}

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    return dump(input);
}
