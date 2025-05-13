#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void write_pam(std::ofstream &output, float min, float max,
               const std::vector<std::vector<std::vector<float>>> &pixels) {
    const uint32_t height = pixels.size();
    const uint32_t width = pixels[0].size();

    output << "P7\nWIDTH " << width << "\nHEIGHT " << height
           << "\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            std::vector<uint8_t> rgb(3);
            for (int c = 0; c < 3; ++c) {
                float val = pixels[y][x][c];
                float mapped = std::pow((val - min) / (max - min), 0.45f);
                rgb[c] = static_cast<uint8_t>(std::clamp(mapped * 255.0f, 0.0f, 255.0f));
            }
            output.write(reinterpret_cast<char *>(rgb.data()), 3);
        }
    }
}

bool parse_resolution(const std::string &line, uint32_t &width, uint32_t &height) {
    if (line.rfind("-Y", 0) == 0) {
        std::istringstream iss(line);
        std::string y_token, x_token;
        iss >> y_token >> height >> x_token >> width;
        return true;
    }
    return false;
}

int decode(std::ifstream &input, std::ofstream &output) {
    std::string line;
    bool valid_format = false;

    // Header parsing
    while (std::getline(input, line)) {
        if (line == "") break; // End of header
        if (line == "#?RADIANCE") continue;
        if (line.rfind("FORMAT=", 0) == 0) {
            if (line.substr(7) != "32-bit_rle_rgbe") return EXIT_FAILURE;
            valid_format = true;
        }
    }

    if (!valid_format) return EXIT_FAILURE;

    // Parse resolution string
    std::getline(input, line);
    uint32_t width = 0, height = 0;
    if (!parse_resolution(line, width, height)) return EXIT_FAILURE;

    std::vector<std::vector<std::vector<float>>> pixels(height, std::vector<std::vector<float>>(width, std::vector<float>(3)));
    float global_min = std::numeric_limits<float>::max();
    float global_max = std::numeric_limits<float>::lowest();

    for (uint32_t y = 0; y < height; ++y) {
        // Read scanline header
        uint8_t magic[4];
        input.read(reinterpret_cast<char *>(magic), 4);
        if (magic[0] != 2 || magic[1] != 2) return EXIT_FAILURE;
        uint32_t scanline_width = (magic[2] << 8) | magic[3];
        if (scanline_width != width) return EXIT_FAILURE;

        std::vector<uint8_t> channel_data[4];
        for (int i = 0; i < 4; ++i) channel_data[i].reserve(width);

        for (int channel = 0; channel < 4; ++channel) {
            size_t x = 0;
            while (x < width) {
                uint8_t count;
                input.read(reinterpret_cast<char *>(&count), 1);
                if (count <= 127) {
                    for (int i = 0; i < count; ++i) {
                        char val;
                        input.read(&val, 1);
                        channel_data[channel].push_back(static_cast<uint8_t>(val));
                        ++x;
                    }
                } else {
                    uint8_t val;
                    input.read(reinterpret_cast<char *>(&val), 1);
                    for (int i = 0; i < count - 128; ++i) {
                        channel_data[channel].push_back(val);
                        ++x;
                    }
                }
            }
        }

        for (uint32_t x = 0; x < width; ++x) {
            uint8_t r = channel_data[0][x];
            uint8_t g = channel_data[1][x];
            uint8_t b = channel_data[2][x];
            uint8_t e = channel_data[3][x];

            if (e != 0) {
                float f = std::ldexp(1.0f, e - 128) / 256.0f; // 2^(E - 128) / 256
                float rf = (r + 0.5f) * f;
                float gf = (g + 0.5f) * f;
                float bf = (b + 0.5f) * f;

                pixels[y][x][0] = rf;
                pixels[y][x][1] = gf;
                pixels[y][x][2] = bf;

                global_min = std::min({global_min, rf, gf, bf});
                global_max = std::max({global_max, rf, gf, bf});
            } else {
                pixels[y][x] = {0.0f, 0.0f, 0.0f};
            }
        }
    }

    write_pam(output, global_min, global_max, pixels);
    return EXIT_SUCCESS;
}

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: hdr_decode <input.hdr> <output.pam>\n";
        return EXIT_FAILURE;
    }

    std::ifstream input(argv[2], std::ios::binary);
    std::ofstream output(argv[3], std::ios::binary);

    if (!input) {
        std::cerr << "Error opening input file\n";
        return EXIT_FAILURE;
    }
    if (!output) {
        std::cerr << "Error opening output file\n";
        return EXIT_FAILURE;
    }

    return decode(input, output);
}
