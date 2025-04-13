#include <fstream>
#include <iosfwd>
#include <string>

std::string packBitPrintCompress(std::ofstream &output, const std::string &characters, const bool similar, const char c) {
    const unsigned long long size = similar ? 257 - characters.size() : characters.size() - 1;
    const std::string string = similar ? std::string(1, characters.at(0)) : std::string().append(characters);
    output << static_cast<unsigned char>(size) << string;
    return {1, c};
}

void compress(std::ifstream &input, std::ofstream &output) {
    std::string binary;
    bool similar = false;
    char c;
    while (input.read(&c, 1)) {
        if (binary.size() < 128) {
            if ((binary.back() == c && similar) || (binary.back() == c && binary.size() == 1)) {
                similar = true;
                binary.push_back(c);
            } else {
                if (!similar) {
                    if (binary.back() == c) {
                        binary.pop_back();
                        binary = packBitPrintCompress(output, binary, similar, c);
                        similar = true;
                    }
                    binary.push_back(c);
                } else {
                    binary = packBitPrintCompress(output, binary, similar, c);
                    similar = false;
                }
            }
        } else {
            binary = packBitPrintCompress(output, binary, similar, c);
            similar = false;
        }
    }
    if (!binary.empty()) {
        packBitPrintCompress(output, binary, similar, '\0');
    }
    output << static_cast<unsigned char>(128);
}

void decompress(std::ifstream &input, std::ofstream &output) {
    char n;
    while (input.read(&n, 1)) {
        int number = static_cast<unsigned char>(n);
        if (number < 128) {
            std::string string(number + 1, '\0');
            input.read(string.data(), number + 1);
            output << string;
        } else if (number > 128) {
            char c;
            input.read(&c, 1);
            output << std::string(257 - number, c);
        } else {
            break;
        }
    }
}

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
    if (std::string(argv[1]) == "c") {
        compress(input, output);
        return EXIT_SUCCESS;
    }
    if (std::string(argv[1]) == "d") {
        decompress(input, output);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
