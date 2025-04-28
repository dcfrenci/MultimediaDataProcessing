#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

int mirror(std::ifstream &input, std::ofstream &output) {
    std::vector<std::string> header;
    uint32_t width = 0, height = 0;
    for (uint8_t i = 0; i < 7; i++) {
        char c = 0;
        std::string string;
        while (c != '\n') {
            input.read(&c, 1);
            string.push_back(c);
        }
        if (string.find("WIDTH") != std::string::npos) {
            width = std::stoi(string.substr(6));
        }
        if (string.find("HEIGHT") != std::string::npos) {
            height = std::stoi(string.substr(7));
        }
        header.push_back(string);
    }
    std::vector<std::vector<char>> lines;
    for (uint32_t line = 0; line < height; line++) {
        std::vector<char> vector(width);
        input.read(vector.data(), width);
        lines.push_back(vector);
    }
    for (auto &element : header) {
        output << element;
    }
    for (auto &element : lines) {
        std::reverse(element.begin(), element.end());
        output.write(element.data(), height);
    }
    return 0;
}

int main(const int argc, const char **argv) {
    if (argc != 3) {
        return 1;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return 1;
    }
    std::ofstream output(argv[2], std::ios::binary);
    if (!output) {
        return 1;
    }
    return mirror(input, output);
}
