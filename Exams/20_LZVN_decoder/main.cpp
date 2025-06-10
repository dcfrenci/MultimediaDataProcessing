//time 04:00:00

#include <cstdint>
#include <fstream>
#include <iostream>

uint32_t fit(const uint32_t value, const uint32_t min, const uint32_t max) {
    return value >= min and value <= max ? value : min;
}

int calculate(std::ifstream &input, const uint8_t &byte, std::string &literals, uint16_t &match_distance,
              uint16_t &match_length) {
    std::string string = "0000";
    uint8_t literals_length = 0;
    if (byte == 0b00011110 or byte == 0b00100110 or byte == 0b00101110 or byte == 0b00110110 or byte == 0b00111110) {
        //udef
        return EXIT_FAILURE;
    }
    if (byte == 0b00001110 or byte == 0b00010110) {
        //nop
        return EXIT_SUCCESS;
    }
    if (byte == 0b11100000) {
        //lrg_l
        input.read(reinterpret_cast<char *>(&literals_length), 1);
        literals_length += 16;
        string.resize(literals_length);
        input.read(string.data(), literals_length);
        literals += string;
        return 2;
    }
    if (byte == 0b00000110) {
        //eos
        uint8_t bytes[7];
        input.read(reinterpret_cast<char *>(bytes), 7);
        return -1;
    }
    if (byte == 0b11110000) {
        //lrg_m
        uint8_t byte2;
        input.read(reinterpret_cast<char *>(&byte2), 1);
        match_length = byte2 + 16;
    } else if ((byte & 0b11110000) == 0b01110000 or (byte & 0b11110000) == 0b11010000) {
        //udef
        return EXIT_FAILURE;
    } else if ((byte & 0b11110000) == 0b11110000) {
        //sml_m
        match_length = byte & 0b00001111;
    } else if ((byte & 0b11100000) == 0b11100000) {
        //sml_l
        literals_length = byte & 0b00001111;
        string.resize(literals_length);
        input.read(string.data(), literals_length);
        literals += string;
        return 2;
    } else if ((byte & 0b11100000) == 0b10100000) {
        //med_d
        uint8_t byte2, byte3;
        input.read(reinterpret_cast<char *>(&byte2), 1);
        input.read(reinterpret_cast<char *>(&byte3), 1);
        literals_length = byte >> 3 & 0b00000011;
        match_length = ((byte & 0b00000111) << 2 | byte2 & 0b00000011) + 3;
        match_distance = byte3 << 6 | byte2 >> 2;
    } else if ((byte & 0b00000111) == 0b00000111) {
        //lrg_d
        uint8_t byte2, byte3;
        input.read(reinterpret_cast<char *>(&byte2), 1);
        input.read(reinterpret_cast<char *>(&byte3), 1);
        literals_length = byte >> 6;
        match_length = (byte >> 3 & 0b00000111) + 3;
        match_distance = byte3 << 8 | byte2;
    } else if ((byte & 0b00000111) == 0b00000110) {
        //pre_d
        literals_length = byte >> 6;
        match_length = (byte >> 3 & 0b00000111) + 3;
    } else {
        //sml_d
        uint8_t byte2;
        input.read(reinterpret_cast<char *>(&byte2), 1);
        literals_length = byte >> 6;
        match_length = (byte >> 3 & 0b00000111) + 3;
        match_distance = (byte & 0b00000111) << 8 | byte2;
    }

    if (literals_length > 0) {
        string.resize(literals_length);
        input.read(string.data(), literals_length);
        literals += string;
    }
    return EXIT_SUCCESS;
}

int decode(std::ifstream &input, std::ofstream &output) {
    while (input.good()) {
        std::string magic_number = "code";
        input.read(magic_number.data(), 4);
        if (magic_number == "bvxn") {
            uint32_t byte_number, block_size;
            input.read(reinterpret_cast<char *>(&byte_number), 4);
            input.read(reinterpret_cast<char *>(&block_size), 4);
            const uint32_t block_start = input.tellg();
            std::string literals;
            uint16_t match_distance = 0, match_length = 0;
            while (static_cast<uint32_t>(input.tellg()) - block_start < block_size) {
                uint8_t byte;
                input.read(reinterpret_cast<char *>(&byte), 1);
                const int ret = calculate(input, byte, literals, match_distance, match_length);
                if (ret == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }
                if (ret == -1) {
                    break;
                }
                if (ret != 2) {
                    const uint32_t start = fit(literals.size() - match_distance, 0, literals.size() - 1);
                    std::string string = literals.substr(start, literals.size());
                    for (uint32_t i = 0; i < match_length;) {
                        for (const auto &c: string) {
                            if (i == match_length) {
                                break;
                            }
                            literals.push_back(c);
                            i++;
                        }
                    }
                }
            }
            output << literals;
        } else if (magic_number == "bvx$") {
            break;
        }
    }
    return EXIT_SUCCESS;
}

int main(const int argc, const char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    std::ofstream output(argv[2], std::ios::binary);
    // temp(input, output);
    if (!input or !output) {
        return EXIT_FAILURE;
    }
    return decode(input, output);
}
