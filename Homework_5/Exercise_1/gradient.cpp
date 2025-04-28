#include <cstdint>
#include <fstream>
#include <array>

int gradient(std::ofstream &out) {
    //header
    out << "P7\n";
    out << "WIDTH 256\n";
    out << "HEIGHT 256\n";
    out << "DEPTH 1\n";
    out << "MAXVAL 255\n";
    out << "TUPLTYPE GRAYSCALE\n";
    out << "ENDHDR\n";

    //content
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
