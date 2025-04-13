#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

template<typename T>
std::ostream &raw_read(std::istream &is, const T &value, const size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&value), size);
}

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

    std::vector<int> v;
    int32_t val;
    while (raw_read(input, val)) {
        v.push_back(val);
    }

    std::ranges::copy(v.begin(), v.end(), output);

    return EXIT_SUCCESS;
}
