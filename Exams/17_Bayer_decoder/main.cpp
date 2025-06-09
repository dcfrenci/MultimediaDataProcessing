//Time 03:47:07

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

uint8_t get_pixel(const uint32_t row, const uint32_t column, const std::vector<std::vector<std::array<uint8_t, 3> > > &data,
                  const bool green) {
    if (row < data.size() and column < data[0].size()) {
        const std::array<uint8_t, 3> pixel = data[row][column];
        if (green) {
            return pixel[1];
        }
        if (pixel[0] != 0 or pixel[2] != 0) {
            return pixel[0] != 0 ? pixel[0] : pixel[2];
        }
        return pixel[1];
    }
    return 0;
}

uint8_t fit(const int32_t value, const uint8_t min, const uint8_t max) {
    return value >= min and value <= max ? static_cast<uint8_t>(value) : value < min ? min : max;
}

uint8_t calculate(const uint8_t v[]) {

}

int decode(std::ifstream &input, const std::string &out_prefix) {
    std::string line;
    std::getline(input, line);
    if (line != "P5") {
        return EXIT_FAILURE;
    }
    std::getline(input, line);
    const uint32_t width = std::stoi(line.substr(0, line.find(' ')));
    const uint32_t height = std::stoi(line.substr(line.find(' ') + 1));
    std::getline(input, line);
    const uint32_t max_val = std::stoi(line);
    std::ofstream output_gray(out_prefix + "_gray.pgm", std::ios::binary);
    std::ofstream output_bayer(out_prefix + "_bayer.ppm", std::ios::binary);
    std::ofstream output_green(out_prefix + "_green.ppm", std::ios::binary);
    std::ofstream output_interp(out_prefix + "_interp.ppm", std::ios::binary);
    if (!output_gray or !output_bayer or !output_green or !output_interp) {
        return EXIT_FAILURE;
    }
    output_gray     << "P5\n" << width << " " << height << "\n255\n";
    output_bayer    << "P6\n" << width << " " << height << "\n255\n";
    output_green    << "P6\n" << width << " " << height << "\n255\n";
    output_interp   << "P6\n" << width << " " << height << "\n255\n";

    std::vector data(height, std::vector<std::array<uint8_t, 3> >(width));
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            uint16_t value = input.get();
            value = value << 8 | input.get();
            uint8_t final_value = fit(value / 256, 0, 255);
            std::array<uint8_t, 3> pixel{{0, 0, 0}};
            if (r % 2 == 0) {
                if (c % 2 == 0) {
                    pixel[0] = final_value;
                } else {
                    pixel[1] = final_value;
                }
            } else {
                if (c % 2 == 0) {
                    pixel[1] = final_value;
                } else {
                    pixel[2] = final_value;
                }
            }
            output_gray << final_value;
            output_bayer << pixel[0] << pixel[1] << pixel[2];
            data[r][c] = pixel;
        }
    }
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            std::array<uint8_t, 3> &pixel = data[r][c];
            if ((r % 2 == 0 and c % 2 != 0) or (r % 2 != 0 and c % 2 == 0)) {
                output_green << pixel[0] << pixel[1] << pixel[2];
                continue;
            }
            const uint8_t x5 = get_pixel(r, c, data, false);
            const uint8_t x1 = get_pixel(r - 2, c, data, false);
            const uint8_t g2 = get_pixel(r - 1, c, data, true);
            const uint8_t g8 = get_pixel(r + 1, c, data, true);
            const uint8_t x9 = get_pixel(r + 2, c, data, false);
            const uint8_t x3 = get_pixel(r, c - 2, data, false);
            const uint8_t g4 = get_pixel(r, c - 1, data, true);
            const uint8_t g6 = get_pixel(r, c + 1, data, true);
            const uint8_t x7 = get_pixel(r, c + 2, data, false);
            const uint32_t delta_h = std::abs(g4 - g6) + std::abs(x5 - x3 + x5 - x7);
            const uint32_t delta_v = std::abs(g2 - g8) + std::abs(x5 - x1 + x5 - x9);
            uint8_t g5;
            if (delta_h < delta_v) {
                g5 = fit((g4 + g6) / 2 + (x5 - x3 + x5 - x7) / 4, 0, 255);
            } else if (delta_h > delta_v) {
                g5 = fit((g2 + g8) / 2 + (x5 - x7 + x5 - x9) / 4, 0, 255);
            } else {
                g5 = fit((g2 + g4 + g6 + g8) / 4 + (x5 - x3 + x5 - x7 + x5 - x7 + x5 - x9) / 8, 0, 255);
            }
            pixel[1] = g5;
            output_green << pixel[0] << pixel[1] << pixel[2];
        }
    }
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            std::array<uint8_t, 3> &pixel = data[r][c];
            if ((r % 2 == 0 and c % 2 != 0) or (r % 2 != 0 and c % 2 == 0)) {
                output_interp << pixel[0] << pixel[1] << pixel[2];
                continue;
            }
            const uint8_t g5 = get_pixel(r, c, data, true);
            const uint8_t x1 = get_pixel(r - 1, c - 1, data, false);
            const uint8_t x9 = get_pixel(r + 1, c + 1, data, false);
            const uint8_t g1 = get_pixel(r - 1, c - 1, data, true);
            const uint8_t g9 = get_pixel(r + 1, c + 1, data, true);
            const uint8_t x3 = get_pixel(r - 1, c + 1, data, false);
            const uint8_t x7 = get_pixel(r + 1, c - 1, data, false);
            const uint8_t g3 = get_pixel(r - 1, c + 1, data, true);
            const uint8_t g7 = get_pixel(r + 1, c - 1, data, true);
            const uint32_t lamda_n = std::abs(x1 - x9) + std::abs(g5 - g1 + g5 - g9);
            const uint32_t lamda_p = std::abs(x3 - x7) + std::abs(g5 - g3 + g5 - g7);
            uint16_t x5;
            if (lamda_n < lamda_p) {
                x5 = fit((x1 + x9) / 2 + (g5 - g1 + g5 - g9) / 4, 0, 255);
            } else if (lamda_n > lamda_p) {
                x5 = fit((x3 + x7) / 2 + (g5 - g3 + g5 - g7) / 4, 0, 255);
            } else {
                x5 = fit((x1 + x9 + x3 + x7) / 4 + (g5 - g1 + g5 - g9 + g5 - g3 + g5 - g7) / 8, 0, 255);
            }
            if (r % 2 != 0) {
                pixel[0] = static_cast<uint8_t>(x5);
            } else {
                pixel[2] = static_cast<uint8_t>(x5);
            }
            output_interp << pixel[0] << pixel[1] << pixel[2];
        }
    }
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    return decode(input, argv[2]);
}
