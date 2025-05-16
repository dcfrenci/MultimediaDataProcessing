#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <bit>
#include <iostream>

struct bit_reader {
    uint8_t buffer_;
    uint8_t size_;
    std::ifstream &input;

    explicit bit_reader(std::ifstream &input) : buffer_(0), size_(0), input(input) {
    }

    uint8_t read_bit() {
        if (size_ == 0) {
            input.read(reinterpret_cast<char *>(&buffer_), 1);
            size_ = 8;
        }
        size_--;
        return buffer_ >> size_ & 1;
    }

    uint32_t operator ()(uint32_t n_bits) {
        uint32_t result = 0;
        while (n_bits-- > 0) {
            result = result << 1 | read_bit();
        }
        return result;
    }
};

int decoder(std::ifstream &input, std::ofstream &output) {
    bit_reader reader(input);
    bool eof = false;
    uint32_t l = 0, m = 0, d = 0;
    while (!eof) {
        const uint8_t f1_byte = reader(8);
        if (f1_byte >> 5 & 0xFF == 0x05) {
            // med_d
            std::cout << "med_d\n";
            const uint8_t f2_byte = reader(8);
            const uint8_t f3_byte = reader(8);
            l = f1_byte << 3 >> 3 & 0xFF;
            const uint16_t m1 = (f1_byte << 5 & 0xFF) >> 3;
            const uint16_t m2 = (f2_byte << 6 & 0xFF) >> 6;
            const uint16_t d1 = (f2_byte >> 2 & 0xFF) << 8;
            m = m1 | m2;
            d = d1 | f3_byte;

        } else if (f1_byte << 5 & 0xFF == 0xE0) {
            // lrg_d
            std::cout << "lrg_d\n";
        } else if (f1_byte << 5 & 0xFF == 0xC0) {
            // pre_d
            std::cout << "pre_d\n";
        } else if (f1_byte >> 4 & 0xFF == 0x0F) {
            // sml_m
            std::cout << "sml_m\n";
        } else if (f1_byte == 0xF0) {
            // lrg_m
            std::cout << "lrg_m\n";
        } else if (f1_byte >> 4 & 0xFF == 0x0E) {
            // sml_l
            std::cout << "sml_l\n";
        } else if (f1_byte == 0xE0) {
            // lrg_l
            std::cout << "lrg_l\n";
        } else if (f1_byte == 0x0E or f1_byte == 0x16) {
            // nop
            std::cout << "nop\n";
        } else if (f1_byte == 0x06) {
            // eof
            std::cout << "eof\n";
            eof = true;
        } else if (f1_byte >> 4 & 0xFF == 0x07 or f1_byte >> 4 & 0xFF == 0x0D or f1_byte == 0x1E or f1_byte == 0x26 or
                   f1_byte == 0x2E or f1_byte == 0x36 or f1_byte == 0x3E) {
            // udef
            std::cout << "udef\n";
            return EXIT_FAILURE;
        } else {
            // sml_d
            std::cout << "sml_d\n";
            const uint8_t f2_byte = reader(8);
            l = f1_byte >> 6 & 0xFF;
            m = (f1_byte << 2 & 0xFF) >> 5 & 0xFF;
            const uint16_t d1 = (f1_byte << 5 & 0xFF) << 3;
            d = d1 | f2_byte;
            auto temp = 0;
        }
    }
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
    return decoder(input, output);
}
