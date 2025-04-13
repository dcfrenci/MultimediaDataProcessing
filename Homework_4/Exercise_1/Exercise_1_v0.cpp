#include <fstream>
#include <iosfwd>
#include <string>

void packBitPrintCompress(std::ofstream &output, const std::string &characters, const bool similar) {
    const unsigned long long size = similar ? 257 - characters.size() : characters.size() - 1;
    const std::string string = similar ? std::string(1, characters.at(0)) : std::string().append(characters);
    output << static_cast<unsigned char>(size) << string;
}

void packBitPrintDecompress(std::ofstream &output, const std::string &characters) {
    output << characters;
}

void compress(std::ifstream &input, std::ofstream &output) {
    std::string binary;
    bool similar = false;
    char c;
    while (input.read(&c, 1)) {
        if (binary.size() < 128) {
            if (binary.back() == c) {
                if (binary.size() == 1) {
                    similar = true;
                } else {
                    if (!similar) {
                        binary.pop_back();
                        packBitPrintCompress(output, binary, similar);
                        binary.clear();
                        similar = true;
                        binary.push_back(c);
                    }
                }
                binary.push_back(c);
            } else {
                if (!similar) {
                    binary.push_back(c);
                } else {
                    packBitPrintCompress(output, binary, similar);
                    binary.clear();
                    binary.push_back(c);
                    similar = false;
                }
            }
        } else {
            packBitPrintCompress(output, binary, similar);
            binary.clear();
            binary.push_back(c);
            similar = false;
        }
    }
    if (!binary.empty()) {
        packBitPrintCompress(output, binary, similar);
    }
    output << static_cast<unsigned char>(128);
}

void decompress(std::ifstream &input, std::ofstream &output) {
    while (input.good()) {
        char n;
        input.read(&n, 1);
        int number = static_cast<unsigned char>(n);
        if (number < 128) {
            std::string string(number + 1, '\0');
            input.read(string.data(), number + 1);
            packBitPrintDecompress(output, string);
        } else if (number > 128) {
            char c;
            input.read(&c, 1);
            packBitPrintDecompress(output, std::string(257 - number, c));
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
