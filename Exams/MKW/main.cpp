#include <algorithm>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iterator>

struct element {
    std::string name_;
    uint32_t id_;
    std::string type_;
    bool mandatory_;
    std::vector<uint8_t> data_;
    std::vector<uint32_t> children_;
    std::vector<uint32_t> necessary_;

    element(): id_(0), mandatory_(false) {
    }

    element(std::string name, const uint32_t id, std::string type, const bool mandatory): name_(std::move(name)),
        id_(id), type_(std::move(type)), mandatory_(mandatory) {
    }

    void save_data(std::ifstream &input, const uint64_t size) {
        std::vector<uint8_t> data(size);
        input.read(reinterpret_cast<char *>(data.data()), static_cast<long long>(size));
        data_.insert(data_.end(), data.begin(), data.end());
    }

    uint64_t get_int() {
        uint64_t value = 0;
        for (const auto &d: data_) {
            value = value << 8 | d;
        }
        return value;
    }

    [[nodiscard]] std::string get_string(const uint32_t level, const uint64_t position,
                                         const uint64_t content_size) const {
        std::ostringstream id;
        id << std::hex << id_;
        return std::string(level, '\t') + (type_ == "master" ? "+ " : "| ") + name_ + "(" + type_ +
               ")[position #" + std::to_string(position) + ", identifier 0x" + id.str() + ", size " +
               std::to_string(content_size) + "]\n";
    }

    [[nodiscard]] bool contains(const uint32_t id) {
        // if (name_ == "Cluster" and !necessary_.empty()) {
        //     if (id == necessary_.front()) {
        //         necessary_.erase(necessary_.begin());
        //         return true;
        //     }
        //     return false;
        // }
        return std::any_of(children_.begin(), children_.end(), [&id](const uint32_t d) { return d == id; });
    }
};

void mapping(std::ifstream &table, std::unordered_map<uint32_t, element> &table_map) {
    std::map<std::string, uint32_t> map;
    std::string line;
    while (table.good()) {
        std::getline(table, line);
        line = line.substr(std::min(line.find_first_not_of(' '), line.size()));
        if (line.find('<') != 0 or line.find("EBMLSchema") != std::string::npos or
            line.find("<!--") != std::string::npos or line.find("</element") != std::string::npos or
            line.find("<extension") != std::string::npos or line.find("<documentation") != std::string::npos or
            line.find("<enum") != std::string::npos or line.find("</enum") != std::string::npos or
            line.find("<restriction") != std::string::npos or line.find("</restriction") != std::string::npos or
            line.find("</documentation") != std::string::npos or line.find("<implementation") != std::string::npos) {
            continue;
        }
        uint32_t pos = line.find("name=") + 6;
        const std::string name = line.substr(pos, line.substr(pos).find_first_of('\"'));
        pos = line.find("type=") + 6;
        const std::string type = line.substr(pos, line.substr(pos).find_first_of('\"'));
        pos = line.find("path=") + 6;
        std::string path = line.substr(pos, line.substr(pos).find_first_of('\"'));
        pos = line.find("id=") + 4 + 2;
        std::string id = line.substr(pos, line.substr(pos).find_first_of('\"'));
        pos = line.find("minOccurs=") != std::string::npos ? 1 : 0;
        const element elem(name, std::stoi(id, nullptr, 16), type, pos == 1);
        table_map.insert(std::pair(elem.id_, elem));
        map.insert(std::pair(elem.name_, elem.id_));
        std::vector<std::string> names;
        while (!path.empty()) {
            const uint8_t index = path.find_last_of('\\');
            names.push_back(path.substr(index + 1));
            path = path.substr(0, index);
        }
        if (names.size() > 1) {
            table_map.at(map.at(names[1])).children_.push_back(elem.id_);
            if (elem.mandatory_) {
                table_map.at(map.at(names[1])).necessary_.push_back(elem.id_);
            }
        }
    }
}

uint8_t calc_size(const uint8_t byte) {
    const std::bitset<8> bits(byte);
    for (uint8_t i = 8 - 1; i < 8; i--) {
        if (bits.test(i)) {
            return 8 - i;
        }
    }
    return 0;
}

uint64_t calc_id(std::ifstream &input, const uint8_t byte, const uint8_t size, const bool is_size) {
    uint64_t id = byte;
    if (is_size) {
        id = byte & (0xFF >> size);
    }
    for (uint8_t i = 0; i < size - 1; i++) {
        uint8_t b;
        input.read(reinterpret_cast<char *>(&b), 1);
        id = id << 8 | b;
    }
    return id;
}

void update_master(std::vector<uint64_t> &m_size, std::vector<element> &m_elements, const uint64_t byte) {
    std::for_each(m_size.begin(), m_size.end(), [byte](uint64_t &m) { m = m - byte < m ? m - byte : 0; });
    const uint8_t remove = std::count_if(m_size.begin(), m_size.end(), [](const uint64_t m) { return m == 0; });
    for (uint8_t i = 0; i < remove; i++) {
        m_size.pop_back();
        m_elements.pop_back();
    }
}

void analyze(std::ifstream &input, std::ofstream &output, std::unordered_map<uint32_t, element> &table_map,
             std::vector<uint64_t> &m_size, std::vector<element> &m_elements) {
    if (!input.good()) { return; }
    const uint64_t position = input.tellg();
    uint8_t byte;
    input.read(reinterpret_cast<char *>(&byte), 1);
    uint8_t h_size = calc_size(byte);
    const uint32_t id = calc_id(input, byte, calc_size(byte), false);
    input.read(reinterpret_cast<char *>(&byte), 1);
    h_size += calc_size(byte);
    const uint64_t c_size = calc_id(input, byte, calc_size(byte), true);
    if (table_map.find(id) == table_map.end() or (
            !m_elements.empty() != 0 and !m_elements.back().contains(id))) {
        input.seekg(static_cast<int64_t>(position) + 1, std::ios::beg);
    } else {
        element elem = table_map.at(id);
        output << elem.get_string(m_size.size(), position, c_size + h_size);
        if (elem.type_ == "master") {
            m_size.push_back(c_size + h_size);
            m_elements.push_back(elem);
            update_master(m_size, m_elements, h_size);
            analyze(input, output, table_map, m_size, m_elements);
        } else {
            elem.save_data(input, c_size);
        }
    }
    const uint64_t byte_read = static_cast<uint64_t>(input.tellg()) - position;
    update_master(m_size, m_elements, byte_read);
    analyze(input, output, table_map, m_size, m_elements);
}

int decoder(std::ifstream &input, std::ofstream &output, std::ifstream &table) {
    std::unordered_map<uint32_t, element> table_map;
    mapping(table, table_map);
    std::vector<uint64_t> m_size;
    std::vector<uint32_t> m_ids;
    std::vector<element> m_elements;
    analyze(input, output, table_map, m_size, m_elements);
    return EXIT_SUCCESS;
}

int main(const int argc, const char **argv) {
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    std::ifstream table(argv[2], std::ios::binary);
    std::ofstream output(argv[3], std::ios::binary);
    if (!input or !table or !output) {
        return EXIT_FAILURE;
    }
    return decoder(input, output, table);
}

// void analyze(std::ifstream &input, std::ofstream &output, std::unordered_map<uint32_t, element> &table_map, std::vector<uint64_t> &m_size,
//              const uint64_t byte_read, uint32_t father_id) {
//     if (!input.good()) { return; }
//
//     const uint64_t position = input.tellg();
//     uint8_t byte;
//     input.read(reinterpret_cast<char *>(&byte), 1);
//     uint8_t h_size = calc_size(byte);
//     const uint32_t id = calc_id(input, byte, calc_size(byte), false);
//
//     // if (!m_size.empty() and byte_read + input.tellg() - position > m_size.back()) {
//     //     m_size.pop_back();
//     //     father_id = 0;
//     // }
//     if (table_map.find(id) == table_map.end() or (father_id != 0 and !table_map.find(father_id)->second.contains(id))) {
//         input.seekg(static_cast<int64_t>(position) + 1, std::ios::beg);
//         if (!m_size.empty() and byte_read + input.tellg() - position > m_size.back()) {
//             m_size.pop_back();
//             father_id = 0;
//         }
//         analyze(input, output, table_map, m_size, byte_read + input.tellg() - position, father_id);
//     } else {
//         input.read(reinterpret_cast<char *>(&byte), 1);
//         h_size += calc_size(byte);
//         const uint64_t c_size = calc_id(input, byte, calc_size(byte), true);
//
//         element elem = table_map.find(id)->second;
//         output << elem.get_string(m_size.size(), position, c_size);
//         if (!m_size.empty() and byte_read + input.tellg() - position > m_size.back()) {
//             m_size.pop_back();
//             father_id = 0;
//         }
//         if (elem.type_ == "master") {
//             m_size.push_back(c_size);
//             analyze(input, output, table_map, m_size, h_size, elem.id_);
//         }
//         elem.save_data(input, c_size);
//         analyze(input, output, table_map, m_size, byte_read + input.tellg() - position, father_id);
//     }
// }


// v2
// void analyze(std::ifstream &input, std::ofstream &output, std::unordered_map<uint32_t, element> &table_map,
//              std::vector<uint64_t> &m_size,
//              uint64_t byte_read, uint32_t father_id) {
//     if (!input.good()) { return; }
//
//     const uint64_t position = input.tellg();
//
//     uint8_t byte;
//     input.read(reinterpret_cast<char *>(&byte), 1);
//     uint8_t h_size = calc_size(byte);
//     const uint32_t id = calc_id(input, byte, calc_size(byte), false);
//
//     input.read(reinterpret_cast<char *>(&byte), 1);
//     h_size += calc_size(byte);
//     const uint64_t c_size = calc_id(input, byte, calc_size(byte), true);
//
//     element elem = table_map.find(id) != table_map.end() ? table_map.at(id) : element();
//
//     if (table_map.find(id) == table_map.end() or (father_id != 0 and !table_map.find(father_id)->second.contains(id))) {
//         input.seekg(static_cast<int64_t>(position) + 1, std::ios::beg);
//     } else {
//         output << elem.get_string(m_size.size(), position, c_size + h_size);
//         if (elem.type_ == "master") {
//             m_size.push_back(c_size + h_size);
//             analyze(input, output, table_map, m_size, h_size, elem.id_);
//         } else {
//             elem.save_data(input, c_size);
//         }
//     }
//     byte_read = byte_read + input.tellg() - position;
//     if (!m_size.empty() and byte_read >= m_size.back()) {
//         m_size.pop_back();
//         if (m_size.empty()) {
//             byte_read = 0;
//         }
//         father_id = 0;
//     }
//     analyze(input, output, table_map, m_size, byte_read, father_id);
// }


//v3
// void analyze(std::ifstream &input, std::ofstream &output, std::unordered_map<uint32_t, element> &table_map,
//              std::vector<uint64_t> &m_size, std::vector<uint32_t> &m_ids) {
//     if (!input.good()) { return; }
//
//     const uint64_t position = input.tellg();
//
//     uint8_t byte;
//     input.read(reinterpret_cast<char *>(&byte), 1);
//     uint8_t h_size = calc_size(byte);
//     const uint32_t id = calc_id(input, byte, calc_size(byte), false);
//
//     input.read(reinterpret_cast<char *>(&byte), 1);
//     h_size += calc_size(byte);
//     const uint64_t c_size = calc_id(input, byte, calc_size(byte), true);
//
//     element elem = table_map.find(id) != table_map.end() ? table_map.at(id) : element();
//
//     if (table_map.find(id) == table_map.end() or (
//             !m_ids.empty() != 0 and !table_map.find(m_ids.back())->second.contains(id))) {
//         input.seekg(static_cast<int64_t>(position) + 1, std::ios::beg);
//             } else {
//                 output << elem.get_string(m_size.size(), position, c_size + h_size);
//                 if (elem.type_ == "master") {
//                     m_size.push_back(c_size + h_size);
//                     m_ids.push_back(elem.id_);
//                     update_master(m_size, m_ids, h_size);
//                     analyze(input, output, table_map, m_size, m_ids);
//                 } else {
//                     elem.save_data(input, c_size);
//                 }
//             }
//     const uint64_t byte_read = static_cast<uint64_t>(input.tellg()) - position;
//     update_master(m_size, m_ids, byte_read);
//     analyze(input, output, table_map, m_size, m_ids);
// }

//v4
