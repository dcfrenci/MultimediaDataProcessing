#include <cstdint>
#include <fstream>
#include <array>

int gradient(std::ofstream &out) {
    out << "P7\n" << "WIDTH 256\n" << "HEIGHT 256\n" << "DEPTH 1\n" << "MAXVAL 255\n" << "TUPLTYPE GRAYSCALE\n" << "ENDHDR\n";
    std::array<char, 256> v{};
    for (uint32_t i = 0; i < 256; i++) {
        v.fill(static_cast<char>(i));
        out.write(v.data(), v.size());
    }
    return 0;
}

int main(const int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
    std::ofstream out(argv[1], std::ios::binary);
    if (!out) {
        return 1;
    }
    return gradient(out);
}
