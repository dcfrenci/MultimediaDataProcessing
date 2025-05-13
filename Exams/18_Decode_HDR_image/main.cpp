#include <algorithm>
#include <complex>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

void write_pam(std::ofstream &output, const float_t min, const float_t max,
               const std::vector<std::vector<std::vector<float_t> > > &pixels) {
    const uint32_t column = pixels[0].size();
    const uint32_t row = pixels.size();
    output << "P7\nWIDTH " << column << "\nHEIGHT " << row << "\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
    std::vector<uint8_t> pixels_cast(3);
    for (uint32_t r = 0; r < row; r++) {
        for (uint32_t c = 0; c < column; c++) {
            auto pixel = pixels[r][c];
            pixels_cast[0] = static_cast<uint8_t>(255 * std::pow((pixel[0] - min) / (max - min), 0.45));
            pixels_cast[1] = static_cast<uint8_t>(255 * std::pow((pixel[1] - min) / (max - min), 0.45));
            pixels_cast[2] = static_cast<uint8_t>(255 * std::pow((pixel[2] - min) / (max - min), 0.45));
            output.write(reinterpret_cast<const std::ostream::char_type *>(pixels_cast.data()), 3);
        }
    }
}

int decode(std::ifstream &input, std::ofstream &output) {
    uint32_t height = 0, width = 0;
    float_t max = 0, min = 255;
    std::vector<std::string> header;
    std::string line;
    while (header.empty() or !header.back().empty()) {
        std::getline(input, line);
        header.push_back(line);
        if (line.find("FORMAT=") != std::string::npos and line.substr(7) != "32-bit_rle_rgbe") {
            return EXIT_FAILURE;
        }
    }
    if (header[0] != "#?RADIANCE") {
        return EXIT_FAILURE;
    }
    std::getline(input, line);
    height = std::stoi(line.substr(2, line.find('+')));
    width = std::stoi(line.substr(line.find('+') + 2));

    std::vector<std::vector<std::vector<uint8_t> > > pixels_encoded;

    for (uint32_t r = 0; r < height; r++) {
        uint32_t head;
        input.read(reinterpret_cast<char *>(&head), sizeof(uint32_t));
        std::vector row(4, std::vector<uint8_t>());
        for (uint8_t color = 0; color < 4; color++) {
            while (row[color].size() < width) {
                uint8_t l = 0;
                input.read(reinterpret_cast<std::istream::char_type *>(&l), 1);
                if (l <= 127) {
                    for (uint8_t i = 0; i < l; i++) {
                        char t;
                        input.read(&t, 1);
                        row[color].push_back(t);
                    }
                } else {
                    char t;
                    input.read(&t, 1);
                    for (uint32_t i = 0; i < l - 128; i++) {
                        row[color].push_back(t);
                    }
                }
            }
        }
        pixels_encoded.push_back(row);
    }
    std::vector pixels(height, std::vector<std::vector<float_t> >(width));
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            std::vector<float_t> pix(3);
            const auto e = pixels_encoded[r][3][c];
            pix[0] = static_cast<float_t>((pixels_encoded[r][0][c] + 0.5) / 256 * std::pow(2, e - 128));
            pix[1] = static_cast<float_t>((pixels_encoded[r][1][c] + 0.5) / 256 * std::pow(2, e - 128));
            pix[2] = static_cast<float_t>((pixels_encoded[r][2][c] + 0.5) / 256 * std::pow(2, e - 128));
            pixels[r][c] = pix;
            for (auto &p: pix) {
                min = std::min(min, p);
                max = std::max(max, p);
            }
        }
    }
    write_pam(output, min, max, pixels);
    return EXIT_SUCCESS;
}

int main(const int argc, const char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    std::ofstream output(argv[2], std::ios::binary);
    if (!input or !output) {
        return EXIT_FAILURE;
    }
    return decode(input, output);
}
