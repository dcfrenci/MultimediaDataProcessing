#include <complex>
#include <cstdint>
#include <fstream>
#include <vector>
#include <map>

int64_t fit(const int64_t index) {
    if (index < 0) {
        return fit(85 + index);
    }
    if (index >= 85) {
        return fit(index - 85);
    }
    return index;
}

uint32_t rotation(const int32_t remainder, const uint32_t n, const uint32_t cycle) {
    const auto pos = static_cast<int32_t>(remainder - n * cycle);
    return pos >= 0 ? pos : 85 + pos;
}

uint32_t rotation_decode(const uint32_t index, const uint32_t n, const uint32_t cycle) {
    const uint32_t pos = cycle * n;
    return fit(index + pos);
    // return index + pos < 85 ? index + pos : index + pos - 85;
}

int encode(std::ifstream &input, std::ofstream &output, const uint32_t n) {
    uint32_t width = 0, height = 0, max_val = 0;
    std::vector<std::string> header;
    while (max_val == 0) {
        std::string line;
        std::getline(input, line);
        if (header.empty()) {
            header.push_back(line);
        } else {
            if (line.front() != '#') {
                if (width == 0) {
                    width = std::stoi(line.substr(0, line.find_first_of(' ')));
                    height = std::stoi(line.substr(line.find_first_of(' ') + 1));
                } else {
                    max_val = std::stoi(line);
                }
            }
        }
    }

    constexpr int8_t symbol[85] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/',
        '*', '?', '&', '<', '>', '(', ')', '[', ']', '{',
        '}', '@', '%', '$', '#'
    };

    //output
    output << width << "," << height << ",";
    uint32_t n_rotation = 0;
    while (input.peek() != EOF) {
        uint32_t value = 0;
        for (uint8_t b = 0; b < 4; b++) {
            uint8_t bits = 0;
            if (!input.eof()) {
                input.read(reinterpret_cast<char *>(&bits), 1);
            }
            value = value << 8 | bits;
        }
        std::vector<uint8_t> components;
        for (uint8_t i = 0; i < 5; i++) {
            const uint8_t remainder = value % 85;
            components.push_back(remainder);
            value /= 85;
        }
        for (uint8_t i = 4; i < 5; i--) {
            output << symbol[rotation(components[i], n, n_rotation)];
            n_rotation++;
        }
    }
    return EXIT_SUCCESS;
}

int decode(std::ifstream &input, std::ofstream &output, const uint32_t n) {
    uint32_t height = 0, width = 0, max_val = 0;
    std::string string;
    while (height == 0) {
        int8_t ch;
        input.read(reinterpret_cast<char *>(&ch), 1);
        if (ch == ',') {
            if (width == 0) {
                width = std::stoi(string);
                string = "";
            } else {
                height = std::stoi(string);
            }
        } else {
            string.push_back(ch);
        }
    }

    constexpr int8_t symbol[85] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/',
        '*', '?', '&', '<', '>', '(', ')', '[', ']', '{',
        '}', '@', '%', '$', '#'
    };
    std::map<uint8_t, uint8_t> map_symbol;
    for (uint8_t i = 0; i < 85; i++) {
        map_symbol[symbol[i]] = i;
    }

    const uint32_t dimension = width * height * 3 % 4 == 0 ? width * height * 3 : width * height * 3 + (4 - width * height * 3 % 4);
    std::vector<uint32_t> components(dimension);
    for (uint32_t i = 0; i < dimension; i++) {
        uint8_t ch;
        input.read(reinterpret_cast<char *>(&ch), 1);
        components[i] = rotation_decode(map_symbol[ch], n, i);
    }
    uint32_t value = 0;
    std::vector<uint8_t> pix;
    // for (uint32_t i = 0; i < dimension; i++) {
    //     // value = value * 85 + components[i];
    //     value += static_cast<uint32_t>(std::pow(85, i % 4)) + components[i];
    //     if (i % 4 == 0 & i != 0) {
    //         pix.push_back(value & 0xFF000000);
    //         pix.push_back(value & 0x00FF0000);
    //         pix.push_back(value & 0x0000FF00);
    //         pix.push_back(value & 0x000000FF);
    //
    //         value = 0;
    //     }
    // }
    for (uint32_t p = 0; p < dimension; p += 5) {
        for (uint32_t i = 4; i < 5; i--) {
            value += static_cast<uint32_t>(std::pow(85, 4 - i)) * components[i];
        }
        pix.push_back(value & 0xFF000000);
        pix.push_back(value & 0x00FF0000);
        pix.push_back(value & 0x0000FF00);
        pix.push_back(value & 0x000000FF);
        value = 0;
    }

    output << "P6\n" << width << " " << height << "\n" << max_val << "\n";

    return EXIT_SUCCESS;
}

int main(const int argc, const char *argv[]) {
    if (argc != 5) {
        return EXIT_FAILURE;
    }
    if (argv[1][0] == 'c') {
        uint32_t n = std::stoi(argv[2]);
        std::ifstream input(argv[3], std::ios::binary);
        std::ofstream output(argv[4], std::ios::binary);
        if (!input or !output) {
            return EXIT_FAILURE;
        }
        return encode(input, output, n);
    }
    if (argv[1][0] == 'd') {
        uint32_t n = std::stoi(argv[2]);
        std::ifstream input(argv[3], std::ios::binary);
        std::ofstream output(argv[4], std::ios::binary);
        if (!input or !output) {
            return EXIT_FAILURE;
        }
        return decode(input, output, n);
    }
}
