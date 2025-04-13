#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <vector>

template<typename T>
std::ostream &raw_write(std::ostream &os, const T &value, const size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char *>(&value), size);
}

template<typename T>
std::ostream &raw_read(std::istream &is, const T &value, const size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&value), size);
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

int main(const int argc, const char *const argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    std::ofstream output(argv[2], std::ios::binary);
    if (!output) {
        return EXIT_FAILURE;
    }

    std::vector<int> v {
        std::istream_iterator<int> (input),
        std::istream_iterator<int> ()
    };

    bitWriter writer(output);

    for (const auto &x : v) {
        writer.write(x, 11);
    }

    // writer.write(0xFFA, 12);
    // writer.write(0xBC, 8);
    // output --> FF AB --> No C because we don't have multiple of 8 bit

    // The objects destructors will be called in the opposite order of their creation
    return EXIT_SUCCESS;
}
