#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <vector>

struct ifd {
    uint8_t bytes_[12]{};

    explicit ifd() = default;

    void operator()(std::ifstream &input) {
        for (auto &b: bytes_) {
            input.read(reinterpret_cast<char *>(&b), sizeof(b));
        }
    }

    [[nodiscard]] uint16_t tag() const {
        return bytes_[1] << 8 | bytes_[0];
    }

    [[nodiscard]] uint16_t type() const {
        return bytes_[3] << 8 | bytes_[2];
    }

    [[nodiscard]] uint32_t count() const {
        return bytes_[7] << 24 | bytes_[6] << 16 | bytes_[5] << 8 | bytes_[4];
    }

    [[nodiscard]] uint32_t offset() const {
        return bytes_[11] << 24 | bytes_[10] << 16 | bytes_[9] << 8 | bytes_[8];
    }
};

void write_pam(std::ofstream &output, std::vector<uint8_t> &pixel, const uint32_t width, const uint32_t height) {
    output << "P7\nWIDTH " << width << "\nHEIGHT " << height << "\nDEPTH 1\nMAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n";
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            output.write(reinterpret_cast<char *>(&pixel[r * width + c]), 1);
        }
    }
}

int decode(std::ifstream &input, std::ofstream &output) {
    uint16_t encoding = 0;
    input.read(reinterpret_cast<char *>(&encoding), sizeof(uint16_t));
    if (encoding != 0x4949) {
        return EXIT_FAILURE;
    }
    ifd ifd;
    uint16_t compression = 0, photometric = 0;
    uint32_t width = 0, height = 0, row_strip = 0, strip_byte = 0, strip_off = 0;

    input.seekg(4, std::ios::beg);
    uint8_t of[4];
    input.read(reinterpret_cast<char *>(of), sizeof(of));
    uint32_t ifd_position = of[3] << 24 | of[2] << 16 | of[1] << 8 | of[0];

    while (ifd_position != 0) {
        input.seekg(ifd_position, std::ios::beg);
        uint16_t n_dir;
        input.read(reinterpret_cast<char *>(&n_dir), sizeof(n_dir));

        for (uint16_t i = 0; i < n_dir; i++) {
            input.seekg(ifd_position + 2 + i * 12, std::ios::beg);
            ifd(input);
            const uint16_t tag = ifd.tag();
            if (tag == 262) {
                photometric = ifd.offset();
            }
            else if (tag == 259) {
                compression = ifd.offset();
            }
            else if (tag == 257) {
                height = ifd.offset();
            }
            else if (tag == 256) {
                width = ifd.offset();
            }
            else if (tag == 278) {
                row_strip = ifd.offset();
            }
            else if (tag == 279) {
                strip_byte = ifd.offset();
            }
            else if (tag == 273) {
                strip_off = ifd.offset();
            }
        }
        uint8_t nof[4];
        input.read(reinterpret_cast<char *>(nof), sizeof(nof));
        ifd_position = nof[3] << 24 | nof[2] << 16 | nof[1] << 8 | nof[0];
    }

    if (compression != 1 or photometric != 1 or row_strip != height or strip_byte != width * height) {
        return EXIT_FAILURE;
    }

    input.seekg(strip_off, std::ios::beg);
    std::vector<uint8_t> pixel(width * height);
    input.read(reinterpret_cast<char *>(pixel.data()), width * height);

    write_pam(output, pixel, width, height);
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
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
