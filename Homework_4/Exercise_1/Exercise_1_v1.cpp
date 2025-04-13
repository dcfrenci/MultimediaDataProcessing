#include <fstream>
#include <iosfwd>
#include <string>

struct buffer {
    std::string buf_string_ = std::string();
    bool similar_ = false;
    std::ofstream &out_;
    explicit buffer(std::ofstream &out) : out_(out) {}

    void insert(const bool similar, const char c) {
        packBitPrintCompress(out_, buf_string_, similar_);
        buf_string_.clear();
        similar_ = similar;
        buf_string_.push_back(c);
    }

    static void packBitPrintCompress(std::ofstream &output, const std::string &characters, const bool similar) {
        const unsigned long long size = similar ? 257 - characters.size() : characters.size() - 1;
        const std::string string = similar ? std::string(1, characters.at(0)) : std::string().append(characters);
        output << static_cast<unsigned char>(size) << string;
    }
};

void packBitPrintDecompress(std::ofstream &output, const std::string &characters) {
    output << characters;
}

void compress(std::ifstream &input, std::ofstream &output) {
    buffer buf(output);
    char c;
    while (input.read(&c, 1)) {
        if (buf.buf_string_.size() < 128) {
            if (buf.buf_string_.back() == c) {
                if (buf.buf_string_.size() == 1) {
                    buf.similar_ = true;
                } else {
                    if (!buf.similar_) {
                        buf.buf_string_.pop_back();
                        buf.insert(true, c);
                    }
                }
                buf.buf_string_.push_back(c);
            } else {
                if (!buf.similar_) {
                    buf.buf_string_.push_back(c);
                } else {
                    buf.insert(false, c);
                }
            }
        } else {
            buf.insert(false, c);
        }
    }
    if (!buf.buf_string_.empty()) {
        buf.insert(true, c);
    }
    output << static_cast<unsigned char>(128);
}

void decompress(std::ifstream &input, std::ofstream &output) {
    while (input.good()) {
        char n;
        input.read(&n, 1);
        int number = static_cast<unsigned char>(n);
        if (number < 128) {
            std::string string(number + 1, '\0');
            input.read(string.data(), number + 1);
            packBitPrintDecompress(output, string);
        } else if (number > 128) {
            char c;
            input.read(&c, 1);
            packBitPrintDecompress(output, std::string(257 - number, c));
        } else {
            break;
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
