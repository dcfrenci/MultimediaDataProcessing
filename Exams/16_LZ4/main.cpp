#include <fstream>
#include <cstdint>
#include <vector>
#include <numeric>

struct Lz4 {
    uint32_t literal_length_;
    uint32_t match_length_;
    std::vector<uint8_t> v_literal_length_;
    std::vector<uint8_t> v_match_length_;
    std::string literals_;
    uint16_t offset_;
    std::ifstream &input_;

    explicit Lz4(std::ifstream &input) : literal_length_(0), match_length_(0), offset_(0), input_(input) {
        uint8_t token;
        input_.read(reinterpret_cast<char *>(&token), sizeof(token));
        v_literal_length_.push_back(token >> 4);
        read_additional(v_literal_length_);
        literal_length_ = std::accumulate(v_literal_length_.begin(), v_literal_length_.end(), 0);
        literals_.resize(literal_length_);
        input_.read(literals_.data(), literal_length_);
        v_match_length_.push_back(token & 0x0F);
    }

    void read_match_copy() {
        input_.read(reinterpret_cast<char *>(&offset_), 2);
        read_additional(v_match_length_);
        match_length_ = std::accumulate(v_match_length_.begin(), v_match_length_.end(), 0) + 4;
    }

    void read_additional(std::vector<uint8_t> &vector) const {
        if (vector.back() != 15) {
            return;
        }
        while (vector.back() == 255 or vector.size() == 1) {
            uint8_t byte;
            input_.read(reinterpret_cast<char *>(&byte), sizeof(byte));
            vector.push_back(byte);
        }
    }

    void read_copy(std::string &old_string)  {
        const std::string new_string = old_string.substr(old_string.size() - offset_);
        if (!new_string.empty()) {
            while (match_length_ > 0) {
                for (const auto &c : new_string) {
                    if (match_length_ == 0) {
                        return;
                    }
                    old_string.push_back(c);
                    match_length_--;
                }
            }
        }
    }
};

int decode(std::ifstream &input, std::ofstream &output) {
    uint8_t byte[4];
    input.read(reinterpret_cast<char *>(byte), 4);
    const uint32_t magic_number = byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3];
    input.read(reinterpret_cast<char *>(byte), 4);
    const uint32_t file_size = byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3];
    input.read(reinterpret_cast<char *>(byte), 4);
    const uint32_t constant = byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3];
    if (magic_number != 0x03214C18 or constant != 0x0000004D) {
        return EXIT_FAILURE;
    }
    if (file_size < 13) {
        uint8_t literals[file_size];
        input.read(reinterpret_cast<char *>(literals), file_size);
        output.write(reinterpret_cast<char *>(literals), file_size);
        return EXIT_SUCCESS;
    }
    std::string output_string;
    while (input.good()) {
        uint32_t block_length;
        input.read(reinterpret_cast<char *>(&block_length), 4);
        const uint32_t block_start = input.tellg();
        while (static_cast<uint32_t>(input.tellg()) - block_start < block_length and input.good()) {
            Lz4 element(input);
            output_string += element.literals_;
            if (static_cast<uint32_t>(input.tellg()) - block_start < block_length) {
                element.read_match_copy();
                element.read_copy(output_string);
            }
        }
    }
    output << output_string;
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
