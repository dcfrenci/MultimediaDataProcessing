#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

#define LEFT    '1'
#define RIGHT   '0'

struct frequency {
    std::string string;
    int freq;

    frequency(std::string string, const int freq): string(std::move(string)), freq(freq) {
    }

    bool operator()(frequency const &f1, frequency const &f2) const {
        return f1.freq > f2.freq;
    }
};

struct buffer {
    unsigned char buffer_ = 0;
    unsigned char size_ = 0;
    std::ofstream &out_;

    explicit buffer(std::ofstream &out): out_(out) {
    }

    void write(const unsigned int bit) {
        buffer_ = (buffer_ << 1) | (bit & 1);
        size_++;
        if (size_ == 8) {
            out_ << buffer_;
            size_ = 0;
        }
    }

    void write_string(std::string const &string) {
        for (const unsigned char bit: string) {
            write(bit == '0' ? 0 : 1);
        }
    }

    void write_bitset(const std::bitset<8> bitset, const int bits) {
        for (int i = bits - 1; i >= 0; i--) {
            write(bitset[i]);
        }
    }

    ~buffer() {
        while (size_ > 0) {
            write(0);
        }
    }
};

void compress(std::ifstream &input, std::ofstream &output) {
    std::string input_symbol;
    std::map<char, int> frequency_map;
    char c;
    while (input.read(&c, 1)) {
        frequency_map[c]++;
        input_symbol.push_back(c);
    }
    std::vector<frequency> frequencies;
    frequencies.reserve(frequency_map.size());
    for (auto [first, second]: frequency_map) {
        frequencies.emplace_back(std::string(1, first), second);
    }
    while (frequencies.size() > 1) {
        std::sort(frequencies.begin(), frequencies.end(), [](frequency const &f1, frequency const &f2) {
            return f1.freq > f2.freq;
        });
        auto first_last = frequencies.back();
        frequencies.pop_back();
        auto second_last = frequencies.back();
        frequencies.pop_back();
        frequencies.emplace_back(
            std::string("(").append(second_last.string).append("+").append(first_last.string).append(")"),
            first_last.freq + second_last.freq);
    }
    std::map<char, std::string> huffman;
    std::string string;
    bool first = true;
    bool close = false;
    for (const char ch: frequencies.back().string.substr(1, frequencies.back().string.size() - 2)) {
        if (ch == '(') {
            if (close or !first) {
                string += RIGHT;
                first = true;
                close = false;
            } else {
                string += LEFT;
            }
        } else if (ch == '+') {
            first = false;
        } else if (ch == ')') {
            string.pop_back();
            close = true;
        } else {
            if (first) {
                string += LEFT;
            } else {
                string += RIGHT;
            }
            huffman[ch] = string;
            string.pop_back();
        }
    }
    const unsigned char table_entry = huffman.size() == 256 ? 0 : huffman.size();
    output << "HUFFMAN1" << table_entry;
    buffer buffer(output);
    for (const auto &[first, second]: huffman) {
        buffer.write_bitset(std::bitset<8>(first), 8);
        buffer.write_bitset(std::bitset<8>(second.size()), 5);
        buffer.write_string(second);
    }
    buffer.write_string(std::bitset<32>(input_symbol.size()).to_string());
    for (const auto &symbol: input_symbol) {
        buffer.write_string(huffman[symbol]);
    }
    //auto flush
}

void decompress(std::ifstream &input, std::ofstream &output) {
    char magic_number[8];
    input.read(magic_number, sizeof(magic_number));
    char table_entries;
    input.read(&table_entries, sizeof(table_entries));
    std::map<std::string, char> huffman;
    std::vector<unsigned char> chars{
        std::istreambuf_iterator{input},
        std::istreambuf_iterator<char>{}
    };
    std::string binary;
    for (const auto &x: chars) {
        binary.append(std::bitset<8>(x).to_string());
    }
    long index = 0;
    const int entries = static_cast<unsigned char>(table_entries) == 0
                            ? 256
                            : static_cast<unsigned char>(table_entries);
    for (int i = 0; i < entries; i++) {
        const char sym = static_cast<char>(std::bitset<8>(binary.substr(index, 8)).to_ulong());
        index += 8;
        const unsigned char len = static_cast<unsigned char>(std::bitset<8>(
            std::string(3, '0') + binary.substr(index, 5)).to_ulong());
        index += 5;
        huffman[binary.substr(index, len)] = sym;
        index += len;
    }
    const unsigned int number_symbol = static_cast<unsigned int>(std::bitset<32>(binary.substr(index, 32)).to_ulong());
    index += 32;
    unsigned int number_read = 0;
    std::string string;
    for (const char ch: binary.substr(index, binary.size() - index - 1)) {
        if (number_read == number_symbol) {
            return;
        }
        string.push_back(ch);
        if (huffman.find(string) != huffman.end()) {
            output << huffman[string];
            string.clear();
            number_read++;
        }
    }
}

int main(const int argc, const char *argv[]) {
    if (argc < 3) {
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
