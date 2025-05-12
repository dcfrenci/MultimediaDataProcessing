#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <vector>

struct bit_writer {
    std::ofstream &input_;
    uint8_t buffer_;
    uint8_t size_;

    explicit bit_writer(std::ofstream &input) : input_(input), buffer_(0), size_(0) {
    }

    void write_bit(const uint8_t bit) {
        if (size_ == 8) {
            input_.write(reinterpret_cast<const std::ostream::char_type *>(&buffer_), 1);
            size_ = 0;
        }
        buffer_ = (buffer_ << 1) | bit;
        size_++;
    }

    void write(const uint32_t value, const uint32_t n_bits, const bool big_endian) {
        if (big_endian) {
            for (uint32_t i = n_bits - 1; i < n_bits; i--) {
                write_bit(value >> i & 1);
            }
        } else {
            for (uint32_t i = 0; i < n_bits; i++) {
                write_bit(value >> i & 1);
            }
        }
    }

    ~bit_writer() {
        while (size_ > 0) {
            write_bit(0);
        }
    }
};

struct bit_reader {
};

struct node {
    node *right_node_;
    node *left_node_;
    uint32_t code_;
    uint32_t frequency_;
    uint8_t symbol_;

    explicit node(const uint32_t frequency, const uint8_t symbol) : right_node_(nullptr), left_node_(nullptr), code_(0),
                                                                    frequency_(frequency), symbol_(symbol) {
    }

    explicit node(node *right_node, node *left_node) : right_node_(right_node), left_node_(left_node), code_(0),
                                                       frequency_(right_node->frequency_ + left_node->frequency_),
                                                       symbol_() {
    }

    bool operator()(node const &n1, node const &n2) const {
        return n1.frequency_ > n2.frequency_;
    }
};

int compress(std::ifstream &input, std::ofstream &output) {
    std::vector<std::string> header;
    uint32_t width = 0, height = 0, depth = 0;
    while (header.empty() or header.back().find("ENDHDR") == std::string::npos) {
        std::string string;
        std::getline(input, string);
        header.push_back(string);
        if (string.find("WIDTH") != std::string::npos)
            width = std::stoi(string.substr(6));
        if (string.find("HEIGHT") != std::string::npos)
            height = std::stoi(string.substr(7));
        if (string.find("DEPTH") != std::string::npos)
            depth = std::stoi(string.substr(6));
    }
    std::vector<std::vector<int16_t> > lines;
    std::unordered_map<int16_t, uint32_t> pixels;
    for (uint32_t line = 0; line < height; line++) {
        std::vector<int16_t> row;
        for (uint32_t pixel = 0; pixel < width; pixel++) {
            uint8_t bit;
            input.read(reinterpret_cast<std::istream::char_type *>(&bit), 1);
            if (pixel != 0) {
                row.push_back(static_cast<int16_t>(bit - row.at(pixel - 1)));
            } else if (line != 0) {
                row.push_back(static_cast<int16_t>(bit - lines[line - 1][0]));
            } else {
                row.push_back(bit);
            }
            pixels[row[pixel]]++;
        }
        lines.push_back(row);
    }

    for (auto &element: header) {
        output << element << "\n";
    }
    for (uint32_t line = 0; line < height; line++) {
        for (uint32_t column = 0; column < width; column++) {
            uint8_t pixel = static_cast<uint8_t>(std::floor(lines.at(line).at(column) / 2)) + 128;
            output.write(reinterpret_cast<const std::ostream::char_type *>(&pixel), 1);
        }
    }

    std::vector<node> nodes;
    nodes.reserve(pixels.size());
    for (auto &[first, second]: pixels) {
        nodes.emplace_back(second, first);
    }

    while (nodes.size() != 1) {
        std::sort(nodes.begin(), nodes.end(), [](node const &n1, node const &n2) {
            return n1.frequency_ > n2.frequency_;
        });
        auto n1 = nodes.back();
        nodes.pop_back();
        auto n2 = nodes.back();
        nodes.pop_back();
        nodes.emplace_back(&n1, &n2);
    }

    bit_writer writer(output);
    // output << "HUFFDIFF";
    // writer.write(width, 32, true);
    // writer.write(height, 32, true);

    return EXIT_SUCCESS;
}

int decompress(std::ifstream &input, std::ofstream &output) {
    return EXIT_SUCCESS;
}

int main(const int argc, const char **argv) {
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[2], std::ios::binary);
    std::ofstream output(argv[3], std::ios::binary);
    if (!input or !output) {
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "c") == 0) {
        return compress(input, output);
    }
    if (strcmp(argv[1], "d") == 0) {
        return decompress(input, output);
    }
    return EXIT_FAILURE;
}
