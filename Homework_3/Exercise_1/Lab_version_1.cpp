#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <vector>

template<typename T>
std::ostream &raw_read(std::istream &is, const T &value, const size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&value), size);
}

template<typename T>
std::ostream &raw_write(std::ostream &os, const T &value, const size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char *>(&value), size);
}

class bitWriter {
    // public: --> Makes the whole class public == Struct
    std::ofstream &out_;
    uint8_t buffer_ = 0;
    size_t n_ = 0;

    void writeBit(const uint32_t bit) {
        buffer_ = (buffer_ << 1) | (bit & 1);
        ++n_;
        if (n_ == 8) {
            // out_ << buffer_; //??
            raw_write(out_, buffer_);
            n_ = 0;
        }
    }

public:
    bitWriter(std::ofstream &out) : out_(out) {
        // When we initialize and specify the out_ and other parameter we should put them in the same order
        // they are declare in the class (struct)
    }

    ~bitWriter() {
        flush();
    }

    // Write the least significant bits of u from MSB to LSB
    std::ostream &write(const uint32_t u, size_t n) {
        // Never create count down with size_t because can't be < 0
        while (n-- > 0) {
            writeBit(u >> n);
        }
        return out_;
    }

    std::ostream &flush(const uint32_t bit = 0) {
        while (n_ > 0) {
            // The n_ will be changed by writeBit and set to zero when reach 8
            writeBit(bit);
        }
        return out_;
    }
};

class bitReader {
    std::istream &input_;
    uint8_t buffer_ = 0;
    size_t n_ = 0;

    uint32_t readBit() {
        if (n_ == 0) {
            raw_read(input_, buffer_);
            n_ = 8;
        }
        --n_;
        return buffer_ >> n_ & 1;
    }

public:
    bitReader(std::istream &input) : input_(input) {
        // When we initialize and specify the out_ and other parameter we should put them in the same order
        // they are declare in the class (struct)
    }

    // Read n bits into u from MSB to LSB
    std::istream &read(uint32_t &u, size_t n) {
        while (n-- > 0) {
            u = (u >> 1) | readBit();
        }
        return input_;
    }

    operator bool() const {
        return input_.good();
    }
};

uint32_t map(const int x) {
    if (x < 0) {
        return static_cast<uint32_t>(-x * 2);
    }
    return static_cast<uint32_t>(x * 2 + 1);
}

int32_t unmap(const uint32_t x) {
    if (x % 2 == 0) {
        return - static_cast<int32_t>(x / 2);
    }
    return static_cast<int32_t>((x - 1) / 2);
}

void compress(const std::string &infile, const std::string &outfile) {
    std::ifstream input(infile);
    if (!input) {
        return;
    }
    std::ofstream output(outfile, std::ios::binary);
    if (!output) {
        return;
    }
    std::vector<int> v{
        std::istream_iterator<int>(input),
        std::istream_iterator<int>()
    };
    bitWriter bw(output);
    for (const auto &x : v) {
        auto val = map(x);
        bw.write(val, std::bit_width(val) * 2);
    }
}

void decompress(const std::string &infile, const std::string &outfile) {
    std::ifstream input(infile, std::ios::binary);
    if (!input) {
        return;
    }
    std::ofstream output(outfile);
    if (!output) {
        return;
    }
    bitReader br(input);
    while (true) {
        int nbits = 0;
        uint32_t u;
        while (br.read(u, 1)) {
            if (u == 1) {
                break;
            }
            ++nbits;
        }
        if (!br) {
            break;
        }
        br.read(u, nbits);
        uint32_t val = (1 << nbits) | u;
        std::println(output, "{}", unmap(val));
    }
}

int main(const int argc, const char *argv[]) {
    if (argc < 3) {
        return EXIT_FAILURE;
    }
    if (std::string(argv[1]) == "c") {
        compress(argv[2], argv[3]);
        return EXIT_SUCCESS;
    } else if (std::string(argv[1]) == "d") {
        decompress(argv[2], argv[3]);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
