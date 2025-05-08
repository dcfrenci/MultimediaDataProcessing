#include <fstream>
#include <iterator>
#include <vector>

template<typename T>
std::ostream &raw_write(std::ostream &os, const T &value, const size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char *>(&value), size);
}

int main(const int argc, const char *const argv[]) {
    using namespace std;

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

    vector<int> v{
        istream_iterator<int>(input),
        istream_iterator<int>()
    };

    /*
    v1
    for (const auto& x : v) {
        output.put((x >> 0) & 0xff);
        output.put((x >> 8) & 0xff);
        output.put((x >> 16) & 0xff);
        output.put((x >> 24) & 0xff);
    }
    v2
    for (const auto &x: v) {
        output.write(reinterpret_cast<const char *>(&x), sizeof(int));
    }
    v3
    output.write(reinterpret_cast<const char *>(v.data()), v.size() * sizeof(int));
    */

    // v4
    for (const int& x: v) {
        raw_write(output, x);
    }
    return EXIT_SUCCESS;
}
