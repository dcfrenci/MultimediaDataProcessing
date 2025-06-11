//time > 04:00:00

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

int compress(std::ifstream &input, std::ofstream &output) {
    std::string head;
    uint32_t height = 0, width = 0, max_val = 0;
    std::string str;
    while (max_val == 0) {
        uint8_t ch;
        input.read(reinterpret_cast<char *>(&ch), 1);
        if (ch == '#') {
            std::getline(input, str);
            str.clear();
        } else if (ch == '\n' or ch == '\r' or ch == ' ') {
            if (head.empty()) {
                head = str;
            } else if (width == 0) {
                width = std::stoi(str);
            } else if (height == 0) {
                height = std::stoi(str);
            } else {
                max_val = std::stoi(str);
            }
            str.clear();
        } else {
            str.push_back(static_cast<char>(ch));
        }
    }
    if (head != "P5") {
        return EXIT_FAILURE;
    }
    if (max_val > 255) {
        std::cout << "Max val > 255" << std::endl;
        return EXIT_FAILURE;
    }
    std::vector pixels(height, std::vector<uint8_t>(width));
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            uint8_t pixel;
            input.read(reinterpret_cast<char *>(&pixel), 1);
            pixels[r][c] = pixel;
        }
    }
    std::vector<std::vector<uint8_t> > level(7);
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            if (r % 8 == 0 and c % 8 == 0) {
                level[0].push_back(pixels[r][c]);
            } else if (r % 8 == 0 and c % 8 == 4) {
                level[1].push_back(pixels[r][c]);
            } else if (r % 8 == 4 and (c % 8 == 0 or c % 8 == 4)) {
                level[2].push_back(pixels[r][c]);
            } else if ((r % 8 == 0 or r % 8 == 4) and (c % 8 == 2 or c % 8 == 6)) {
                level[3].push_back(pixels[r][c]);
            } else if ((r % 8 == 2 or r % 8 == 6) and c % 2 == 0) {
                level[4].push_back(pixels[r][c]);
            } else if (c % 2 != 0 and (r % 8 == 0 or r % 8 == 4 or r % 8 == 2 or r % 8 == 6)) {
                level[5].push_back(pixels[r][c]);
            } else if (r % 2 != 0) {
                level[6].push_back(pixels[r][c]);
            }
        }
    }
    output << "MULTIRES";
    output.write(reinterpret_cast<char *>(&width), 4);
    output.write(reinterpret_cast<char *>(&height), 4);
    for (const auto &l: level) {
        for (const auto &pixel: l) {
            output << pixel;
        }
    }
    return EXIT_SUCCESS;
}

void fill_block(std::vector<std::vector<uint8_t> > &pixels, const uint32_t r_cur, const uint32_t c_cur,
                const uint32_t r_max, const uint32_t c_max, const uint8_t value) {
    for (uint32_t r = r_cur; r < std::min(r_max, static_cast<uint32_t>(pixels.size())); r++) {
        for (uint32_t c = c_cur; c < std::min(c_max, static_cast<uint32_t>(pixels[0].size())); c++) {
            pixels[r][c] = value;
        }
    }
}

void fill(std::vector<std::vector<uint8_t> > &pixel, const uint8_t value, const std::vector<uint8_t> &level) {
    const uint32_t height = pixel.size();
    const uint32_t width = pixel[0].size();
    uint32_t index = 0;
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            if (value == 1 and r % 8 == 0 and c % 8 == 0) {
                fill_block(pixel, r, c, r + 8, c + 8, level[index]);
                index++;
            } else if (value == 2 and r % 8 == 0 and c % 8 == 4) {
                fill_block(pixel, r, c, r + 8, c + 4, level[index]);
                index++;
            } else if (value == 3 and r % 8 == 4 and (c % 8 == 0 or c % 8 == 4)) {
                fill_block(pixel, r, c, r + 4, c + 4, level[index]);
                index++;
            } else if (value == 4 and (r % 8 == 0 or r % 8 == 4) and (c % 8 == 2 or c % 8 == 6)) {
                fill_block(pixel, r, c, r + 4, c + 2, level[index]);
                index++;
            } else if (value == 5 and (r % 8 == 2 or r % 8 == 6) and c % 2 == 0) {
                fill_block(pixel, r, c, r + 2, c + 2, level[index]);
                index++;
            } else if (value == 6 and c % 2 != 0 and (r % 8 == 0 or r % 8 == 4 or r % 8 == 2 or r % 8 == 6)) {
                fill_block(pixel, r, c, r + 2, c + 1, level[index]);
                index++;
            } else if (value == 7 and r % 2 != 0) {
                pixel[r][c] = level[index];
                index++;
            }
        }
    }
}

uint32_t calculate_matrix(const std::vector<std::vector<uint8_t> > &matrix, const uint8_t level, const uint32_t r_max,
                          const uint32_t c_max) {
    uint32_t result = 0;
    for (uint32_t r = 0; r < r_max; r++) {
        for (uint32_t c = 0; c < c_max; c++) {
            if (matrix[r][c] == level) {
                result++;
            }
        }
    }
    return result;
}

uint32_t calculate(const uint8_t level, const uint32_t width, const uint32_t height) {
    const std::vector<uint8_t> matrix_complete = {1, 1, 2, 4, 8, 16, 32};
    const std::vector<std::vector<uint8_t> > matrix = {
        {1, 6, 4, 6, 2, 6, 4, 6},
        {7, 7, 7, 7, 7, 7, 7, 7},
        {5, 6, 5, 6, 5, 6, 5, 6},
        {7, 7, 7, 7, 7, 7, 7, 7},
        {3, 6, 4, 6, 3, 6, 4, 6},
        {7, 7, 7, 7, 7, 7, 7, 7},
        {5, 6, 5, 6, 5, 6, 5, 6},
        {7, 7, 7, 7, 7, 7, 7, 7}
    };
    uint32_t output = 0;
    if (height > 7 and width > 7) {
        output = width / 8 * (height / 8) * matrix_complete.at(level - 1);
        output += calculate_matrix(matrix, level, 8, width % 8) * (height / 8);
        output += calculate_matrix(matrix, level, height % 8, 8) * (width / 8);
    }
    return output + calculate_matrix(matrix, level, height % 8, width % 8);
}

int decompress(std::ifstream &input, const std::string &prefix) {
    std::string magic_number = "MULTIRES";
    input.read(magic_number.data(), 8);
    uint32_t width, height;
    input.read(reinterpret_cast<char *>(&width), 4);
    input.read(reinterpret_cast<char *>(&height), 4);
    std::vector<std::vector<uint8_t> > level(7);
    for (uint8_t i = 0; i < 7; i++) {
        const uint32_t level_size = calculate(i + 1, width, height);
        level[i].resize(level_size);
        input.read(reinterpret_cast<char *>(level[i].data()), level_size);
    }
    std::vector pixels(height, std::vector<uint8_t>(width));
    for (uint8_t i = 0; i < 7; i++) {
        fill(pixels, i + 1, level[i]);
        std::ofstream output(prefix + "_" + std::to_string(i + 1) + ".pgm", std::ios::binary);
        if (!output) {
            return EXIT_FAILURE;
        }
        output << "P5 " << width << ' ' << height << " 255\n";
        for (uint32_t r = 0; r < height; r++) {
            for (uint32_t c = 0; c < width; c++) {
                output << pixels[r][c];
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[2], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    if (argv[1][0] == 'c') {
        std::ofstream output(argv[3], std::ios::binary);
        if (!output) {
            return EXIT_FAILURE;
        }
        return compress(input, output);
    }
    if (argv[1][0] == 'd') {
        return decompress(input, argv[3]);
    }
    return EXIT_FAILURE;
}
