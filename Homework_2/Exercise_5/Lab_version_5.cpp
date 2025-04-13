#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <vector>

template<typename T>
std::ostream &raw_read(std::istream &is, const T &value, const size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&value), size);
}

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

    bitReader reader(input);
    std::vector<unsigned int> v;
    uint32_t value;
    while (reader.read(value, 11)) {
        int32_t realNumber = value;
        if (realNumber > 1023) {
            realNumber -= 2048;
        }
        v.push_back(realNumber);
    }
    // bitfields

    for (const auto &x: v) {
        output << x;
    }

    return EXIT_SUCCESS;
}
