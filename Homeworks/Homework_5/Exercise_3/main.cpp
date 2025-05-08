#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

int mirror(std::ifstream &input, std::ofstream &output) {
    std::vector<std::string> header;
    uint32_t width = 0, height = 0, depth = 0;
    while (header.empty() or header.back().find("ENDHDR") == std::string::npos) {
        std::string string;
        std::getline(input, string);
        header.push_back(string);
        if (string.find("WIDTH") != std::string::npos) {
            width = std::stoi(string.substr(6));
        }
        if (string.find("HEIGHT") != std::string::npos) {
            height = std::stoi(string.substr(7));
        }
        if (string.find("DEPTH") != std::string::npos) {
            depth = std::stoi(string.substr(6));
        }
    }
    std::vector<std::vector<std::vector<char>>> lines;
    for (uint32_t line = 0; line < height; line++) {
        std::vector row(width, std::vector<char>(depth));
        for (uint32_t pixel = 0; pixel < width; pixel++) {
            input.read(row[pixel].data(), depth);
        }
        lines.push_back(row);
    }
    for (auto &element : header) {
        output << element << "\n";
    }
    for (auto &pixels : lines) {
        std::reverse(pixels.begin(), pixels.end());
        for (auto &pixel : pixels) {
            output.write(pixel.data(), depth);
        }
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
