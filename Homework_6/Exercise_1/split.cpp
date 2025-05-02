#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

int split(std::ifstream &input, const std::string &filename) {
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
    const std::vector rgb{'R', 'G', 'B'};
    for (uint32_t pixel_number = 0; pixel_number < depth; pixel_number++) {
        std::ofstream output(filename.substr(0, filename.size() - 4) + "_" + rgb[pixel_number] + ".pam", std::ios::binary);
        for (auto &element : header) {
            output << element << "\n";
        }
        for (auto &pixels : lines) {
            for (auto &pixel : pixels) {
                auto zero = std::string(depth, '0');
                output.write(zero.c_str(), pixel_number);
                output.write(&pixel.data()[pixel_number], 1);
                output.write(zero.c_str(), depth - pixel_number - 1);
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(const int argc, const char** argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    return split(input, argv[1]);
}
