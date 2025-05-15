#include <complex>
#include <cstdint>
#include <fstream>
#include <vector>

struct bit_reader {
    uint8_t buffer_;
    uint8_t size_;
    std::ifstream &input_;

    explicit bit_reader(std::ifstream &input): buffer_(0), size_(0), input_(input) {
    }

    uint8_t read_bit() {
        if (size_ == 0) {
            input_.read(reinterpret_cast<char *>(&buffer_), 1);
            size_ = 8;
        }
        size_--;
        return buffer_ >> size_ & 1;
    }

    uint32_t read(uint16_t bits) {
        uint32_t out = 0;
        while (bits-- > 0) {
            out = out << 1 | read_bit();
        }
        return out;
    }
};

void write_pam(std::ofstream &output, const std::vector<std::vector<std::vector<uint8_t> > > &pixels, const std::vector<std::vector<uint8_t>> &colors, const uint32_t comp_method) {
    const uint32_t height = pixels.size();
    const uint32_t width = pixels[0].size();
    output << "P7\nWIDTH " << width << "\nHEIGHT " << height << "\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
    for (uint32_t r = 0; r < height; r++) {
        for (uint32_t c = 0; c < width; c++) {
            if (comp_method == 0) {
                output.write(reinterpret_cast<const std::ostream::char_type *>(&pixels[r][c][2]), 1);
                output.write(reinterpret_cast<const std::ostream::char_type *>(&pixels[r][c][1]), 1);
                output.write(reinterpret_cast<const std::ostream::char_type *>(&pixels[r][c][0]), 1);
            } else {
                output.write(reinterpret_cast<const std::ostream::char_type *>(&colors[pixels[r][c][0]][2]), 1);
                output.write(reinterpret_cast<const std::ostream::char_type *>(&colors[pixels[r][c][0]][1]), 1);
                output.write(reinterpret_cast<const std::ostream::char_type *>(&colors[pixels[r][c][0]][0]), 1);
            }
        }
    }
}

int compress(std::ifstream &input, std::ofstream &output) {
    //header file --> identifier, offset for pixels data
    uint8_t identify[2];
    input.read(reinterpret_cast<char *>(identify), 2);
    input.seekg(10, std::ios::beg);
    uint32_t offset;
    input.read(reinterpret_cast<char *>(&offset), 4);

    //header info --> width, height, n° bits for pixels
    input.seekg(18, std::ios::beg);
    uint32_t width, height;
    input.read(reinterpret_cast<char *>(&width), 4);
    input.read(reinterpret_cast<char *>(&height), 4);
    input.seekg(28, std::ios::beg);
    uint16_t bits_pixel;
    input.read(reinterpret_cast<char *>(&bits_pixel), 2);

    //color table --> quadruples of bytes B,G,R,0
    input.seekg(30, std::ios::beg);
    uint32_t comp_method;
    input.read(reinterpret_cast<char *>(&comp_method), 4);
    input.seekg(46, std::ios::beg);
    uint32_t n_colors;
    input.read(reinterpret_cast<char *>(&n_colors), 4);
    n_colors = n_colors == 0 ? static_cast<uint32_t>(std::pow(2, bits_pixel)) : n_colors;
    input.seekg(54, std::ios::beg);
    std::vector<std::vector<uint8_t>> colors(n_colors);
    if (comp_method != 0) {
        for (uint32_t i = 0; i < n_colors; i++) {
            for (uint8_t n = 0; n < 4; n++) {
                uint8_t value;
                input.read(reinterpret_cast<char *>(&value), 1);
                colors[i].push_back(value);
            }
        }
    }

    //pixel data --> bits
    input.seekg(offset, std::ios::beg);
    bit_reader reader(input);
    std::vector pixels(height, std::vector<std::vector<uint8_t> >(width));
    for (uint32_t r = height - 1; r < height; r--) {
        for (uint32_t c = 0; c < width; c++) {
            if (comp_method == 0) {
                pixels[r][c].push_back(reader.read(bits_pixel / 3));
                pixels[r][c].push_back(reader.read(bits_pixel / 3));
                pixels[r][c].push_back(reader.read(bits_pixel / 3));
            } else {
                pixels[r][c].push_back(reader.read(bits_pixel));
            }
        }
        reader.read(static_cast<uint16_t>(32 - (bits_pixel * width) % 32));
    }

    write_pam(output, pixels, colors, comp_method);
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
    return compress(input, output);
}
