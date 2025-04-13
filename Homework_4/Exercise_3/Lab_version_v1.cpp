#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <string>
#include <bit>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <format>

template<typename T>
std::ostream &raw_write(std::ostream &os, const T &val,
                        const size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char *>(&val), size);
}

template<typename T>
std::istream &raw_read(std::istream &is, T &val,
                       size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&val), size);
}

class bitwriter {
    std::ostream &os_;
    uint8_t buffer_ = 0;
    size_t n_ = 0;

    void writebit(uint32_t bit) {
        buffer_ = (buffer_ << 1) | (bit & 1);
        ++n_;
        if (n_ == 8) {
            raw_write(os_, buffer_);
            n_ = 0;
        }
    }

public:
    bitwriter(std::ostream &os) : os_(os) {
    }

    ~bitwriter() {
        flush();
    }

    // Write the n least significant bits of u from MSB to LSB
    std::ostream &operator()(uint32_t u, size_t n) {
        while (n-- > 0) {
            writebit(u >> n);
        }
        return os_;
    }

    std::ostream &flush(uint32_t bit = 0) {
        while (n_ > 0) {
            writebit(bit);
        }
        return os_;
    }
};

class bitreader {
    std::istream &is_;
    uint8_t buffer_ = 0;
    size_t n_ = 0;

    uint32_t readbit() {
        if (n_ == 0) {
            raw_read(is_, buffer_);
            n_ = 8;
        }
        --n_;
        return (buffer_ >> n_) & 1;
    }

public:
    bitreader(std::istream &is) : is_(is) {
    }

    // Read n bits into u from MSB to LSB
    std::istream &operator()(uint32_t &u, size_t n) {
        u = 0;
        while (n-- > 0) {
            u = (u << 1) | readbit();
        }
        return is_;
    }

    operator bool() const {
        return is_.good();
    }
};

template<typename T>
struct frequency {
    std::unordered_map<T, uint32_t> counter_;

    void operator()(const T &val) {
        ++counter_[val];
    }
};

template<typename T>
struct node {
    T sym_;
    uint32_t freq_;
    uint32_t code_{}, len_{};
    node *left_ = nullptr;
    node *right_ = nullptr;

    node(T sym, const uint32_t freq) : sym_(std::move(sym)), freq_(freq) {
    }

    node(node *left, node *right) : freq_(left->freq_ + right->freq_),
                                    left_(left), right_(right) {
    }
};

template<typename T>
void make_codes(node<T> *n, uint32_t code, uint32_t len,
                std::unordered_map<T, node<T> *> &map) {
    if (n->left_ == nullptr) {
        n->code_ = code;
        n->len_ = len;
        map[n->sym_] = n;
    } else {
        make_codes(n->left_, code << 1, len + 1, map);
        make_codes(n->right_, (code << 1) | 1, len + 1, map);
    }
}

void compress(const std::string &infile, const std::string &outfile) {
    using namespace std;
    using namespace std::literals;

    ifstream is(infile, std::ios::binary);
    if (!is) {
        exit(EXIT_FAILURE);
    }

    is.seekg(0, ios::end);
    auto filesize = is.tellg();
    is.seekg(0);
    vector<uint8_t> v(filesize);
    raw_read(is, v[0], filesize);

    /*frequency<uint8_t> f;
    for (const auto& x : v) {
        f(x);
    }*/

    auto f = for_each(begin(v), end(v), frequency<uint8_t>{});

    vector<node<uint8_t> *> nodes;
    for (const auto &[sym, freq]: f.counter_) {
        auto n = new node(sym, freq);
        nodes.push_back(n);
    }
    auto pred = [](node<uint8_t> *a, node<uint8_t> *b) {
        return a->freq_ > b->freq_;
    };
    sort(begin(nodes), end(nodes), pred);
    while (nodes.size() > 1) {
        auto n1 = nodes.back();
        nodes.pop_back();
        auto n2 = nodes.back();
        nodes.pop_back();
        auto n = new node(n1, n2);

        auto it = lower_bound(begin(nodes), end(nodes), n, pred);
        nodes.insert(it, n);
    }
    auto root = nodes.back();
    nodes.pop_back();

    unordered_map<uint8_t, node<uint8_t> *> map;
    make_codes(root, 0, 0, map);

    ofstream os(outfile, std::ios::binary);
    if (!os) {
        exit(EXIT_FAILURE);
    }
    os << "HUFFMAN1";
    uint8_t table_size = static_cast<uint8_t>(map.size());
    os.put(table_size);
    bitwriter bw(os);
    for (const auto &[sym, n]: map) {
        bw(sym, 8);
        bw(n->len_, 5);
        bw(n->code_, n->len_);
    }
    bw(static_cast<uint32_t>(v.size()), 32);
    for (const auto &x: v) {
        auto n = map[x];
        bw(n->code_, n->len_);
    }
}

void decompress(const std::string &infile, const std::string &outfile) {
    using namespace std;

    ifstream is(infile, std::ios::binary);
    if (!is) {
        exit(EXIT_FAILURE);
    }

    string header(8, ' ');
    // WRONG!!! -> is.read(reinterpret_cast<char*>(&header), 8);
    // WRONG!!! -> raw_read(is, header, 8);
    // is.read(&header[0], 8); // OK
    // is.read(header.data(), 8); // OK
    raw_read(is, header[0], 8); // OK
    size_t table_len = is.get();
    if (table_len == 0) {
        table_len = 256;
    }
    //typedef tuple<uint8_t, uint32_t, uint32_t> table_entry;
    using table_entry = tuple<uint8_t, uint32_t, uint32_t>; // sym, code, len
    vector<table_entry> table;
    bitreader br(is);
    for (size_t i = 0; i < table_len; ++i) {
        uint32_t sym, code, len;
        br(sym, 8);
        br(len, 5);
        br(code, len);
        table.emplace_back(sym, code, len);
    }
    uint32_t n;
    br(n, 32);

    sort(begin(table), end(table),
         [](const table_entry &a, const table_entry &b) {
             return get<2>(a) < get<2>(b);
         });

    ofstream os(outfile, std::ios::binary);
    if (!os) {
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < n; ++i) {
        uint32_t code = 0;
        uint32_t len = 0;
        bool found = false;
        size_t pos;
        for (pos = 0; pos < table_len; ++pos) {
            while (len < get<2>(table[pos])) {
                uint32_t bit;
                br(bit, 1);
                code = (code << 1) | bit;
                ++len;
            }
            if (code == get<1>(table[pos])) {
                found = true;
                break;
            }
        }
        if (!found) {
            exit(EXIT_FAILURE);
        }
        os.put(get<0>(table[pos]));
    }
}

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace std::literals;

    if (argc != 4) {
        return EXIT_FAILURE;
    }
    if (argv[1] == "c"s) {
        compress(argv[2], argv[3]);
    } else if (argv[1] == "d"s) {
        decompress(argv[2], argv[3]);
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
