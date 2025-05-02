#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

std::vector<std::vector<std::vector<char>>> content(std::istream &input) {
    uint32_t width = 0, height = 0, depth = 0;
    std::string string;
    while (true) {
        std::getline(input, string);
        if (string.find("WIDTH") != std::string::npos) {
            width = std::stoi(string.substr(6));
        }
        if (string.find("HEIGHT") != std::string::npos) {
            height = std::stoi(string.substr(7));
        }
        if (string.find("DEPTH") != std::string::npos) {
            depth = std::stoi(string.substr(6));
        }
        if (string.find("ENDHDR") != std::string::npos) {
            break;
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
    return lines;
}

int combine(std::istream &r_input, std::istream &g_input, std::istream &b_input, const std::string &filename) {
    const auto r_pixel = content(r_input);
    const auto g_pixel = content(g_input);
    const auto b_pixel = content(b_input);
    std::ofstream output(filename.substr(0, filename.size() - 5) + "reconstructed.pam", std::ios::binary);
    if (!output) {
        return EXIT_FAILURE;
    }
    const uint32_t width = r_pixel.data()->size();
    const uint32_t height = r_pixel.size();
    output << "P7\n" << "WIDTH " << width << "\n" << "HEIGHT " << height << "\n" << "DEPTH 3\n" << "MAXVAL 255\n" << "TUPLTYPE RGB\n" << "ENDHDR\n";
    for (uint32_t line = 0; line < height; line++) {
        for (uint32_t pixel = 0; pixel < width; pixel++) {
            output.write(&r_pixel[line][pixel][0], 1);
            output.write(&g_pixel[line][pixel][1], 1);
            output.write(&b_pixel[line][pixel][2], 1);
        }
    }
    return EXIT_SUCCESS;
}

int main(const int argc, const char *argv[]) {
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    std::ifstream r_input(argv[1], std::ios::binary);
    if (!r_input) {
        return EXIT_FAILURE;
    }
    std::ifstream g_input(argv[2], std::ios::binary);
    if (!g_input) {
        return EXIT_FAILURE;
    }
    std::ifstream b_input(argv[3], std::ios::binary);
    if (!b_input) {
        return EXIT_FAILURE;
    }
    return combine(r_input, g_input, b_input, argv[1]);
}
