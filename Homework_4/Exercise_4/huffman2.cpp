#include <algorithm>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>
#include <unordered_map>

#define LEFT    '1'
#define RIGHT   '0'

struct bit_reader {
    std::ifstream &input_;
    uint8_t buffer_, size_;

    explicit bit_reader(std::ifstream &input) : input_(input), buffer_(0), size_(0) {
    }

    uint32_t read_bit() {
        if (size_ == 0) {
            input_.read(reinterpret_cast<std::istream::char_type *>(&buffer_), 1);
            size_ = 8;
        }
        size_--;
        return (buffer_ >> size_) & 1;
    }

    uint32_t read(int &n_bits) {
        uint32_t value = 0;
        while (n_bits-- > 0) {
            value = (value << 1) | read_bit();
        }
        return value;
    }
};

struct bit_writer {
    std::ostream &output_;
    uint8_t buffer_, size_;
    explicit bit_writer(std::ostream &output) : output_(output), buffer_(0), size_(0) {}

    void write_bit(const uint8_t bit) {
        if (size_ == 8) {
            output_.write(reinterpret_cast<std::ostream::char_type *>(&buffer_), 1);
            size_ = 0;
        }
        buffer_ = (buffer_ << 1) | bit;
        size_++;
    }

    void write(const uint32_t value, const uint32_t n_bits) {
        for (uint32_t i = n_bits - 1; i < n_bits; i--) {
            write_bit(value >> i & 1);
        }
    }
};

void compress(std::istream &input, std::ostream &output) {
    std::vector<uint8_t> chars{
        std::istream_iterator<uint8_t>{input},
        std::istream_iterator<uint8_t>{}
    };
    auto frequencies = std::for_each(chars.begin(), chars.end(), [](const uint8_t c, std::unordered_map<uint8_t, uint32_t> &map){return map[c]++;});
}

void decompress(std::istream &input, std::ostream &output) {
}

int main(const int argc, const char *argv[]) {
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[2], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    std::ofstream output(argv[3], std::ios::binary);
    if (!output) {
        return EXIT_FAILURE;
    }
    if (std::string(argv[1]) == "c") {
        compress(input, output);
        return EXIT_SUCCESS;
    }
    if (std::string(argv[1]) == "d") {
        decompress(input, output);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
