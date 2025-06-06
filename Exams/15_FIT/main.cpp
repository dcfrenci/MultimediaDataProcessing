//Time: 03:33:57

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>

using namespace std;

void FitCRC_Get16(uint16_t &crc, uint8_t byte) {
    static const uint16_t crc_table[16] =
    {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
    };
    uint16_t tmp;
    // compute checksum of lower four bits of byte
    tmp = crc_table[crc & 0xF];
    crc = (crc >> 4) & 0x0FFF;
    crc = crc ^ tmp ^ crc_table[byte & 0xF];
    // now compute checksum of upper four bits of byte
    tmp = crc_table[crc & 0xF];
    crc = (crc >> 4) & 0x0FFF;
    crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}

uint16_t check_crc(std::ifstream &input, const uint32_t end) {
    const uint32_t position = input.tellg();
    input.seekg(0, std::ios::beg);
    uint16_t crc = 0;
    for (uint32_t i = 0; i < end; i++) {
        FitCRC_Get16(crc, input.get());
    }
    input.seekg(position, std::ios::beg);
    return crc;
}

int decode(std::ifstream &input) {
    //header
    uint8_t header_size, protocol_version;
    input.read(reinterpret_cast<char *>(&header_size), 1);
    input.read(reinterpret_cast<char *>(&protocol_version), 1);
    uint8_t profile[2];
    input.read(reinterpret_cast<char *>(profile), 2);
    uint16_t profile_version = profile[1] << 8 | profile[0];
    uint32_t length_data_records;
    input.read(reinterpret_cast<char *>(&length_data_records), sizeof(uint32_t));
    std::string header_data_type = ".FIT";
    input.read(header_data_type.data(), 4);
    if (header_data_type != ".FIT") {
        return EXIT_FAILURE;
    }

    uint16_t header_crc;
    input.read(reinterpret_cast<char *>(&header_crc), sizeof(uint16_t));
    if (check_crc(input, static_cast<uint32_t>(input.tellg()) - 2) != header_crc) {
        return EXIT_FAILURE;
    }
    std::cout << "Header CRC ok\n";

    //data records
    std::map<uint8_t, std::vector<std::vector<uint8_t>> > data_field_def;
    std::map<uint8_t, uint16_t> data_field_global;
    const uint32_t start_data_records = input.tellg();
    while (static_cast<size_t>(input.tellg()) - start_data_records < length_data_records) {
        uint8_t data_header;
        input.read(reinterpret_cast<char *>(&data_header), sizeof(uint8_t));
        if (data_header >> 4 == 4) {
            //definition
            uint8_t data_reserved, data_architecture, data_num_fields;
            uint16_t data_global_number;
            input.read(reinterpret_cast<char *>(&data_reserved), sizeof(uint8_t));
            input.read(reinterpret_cast<char *>(&data_architecture), sizeof(uint8_t));
            input.read(reinterpret_cast<char *>(&data_global_number), sizeof(uint16_t));
            input.read(reinterpret_cast<char *>(&data_num_fields), sizeof(uint8_t));

            std::vector<std::vector<uint8_t>> v;
            for (uint8_t i = 0; i < data_num_fields; i++) {
                std::vector<uint8_t> data_field(3);
                input.read(reinterpret_cast<char *>(data_field.data()), 3);
                v.push_back(data_field);
            }
            //data message
            input.read(reinterpret_cast<char *>(&data_header), sizeof(uint8_t));
            data_field_def.insert(std::pair(data_header, v));
            data_field_global.insert(std::pair(data_header, data_global_number));
        }
        std::vector<vector<uint8_t>> data_content;
        for (auto &v : data_field_def[data_header]) {
            std::vector<uint8_t> data(v[1]);
            input.read(reinterpret_cast<char *>(data.data()), v[1]);
            data_content.push_back(data);
        }
        if (const uint16_t data_global_number = data_field_global[data_header]; data_global_number == 0) {
            const std::vector<uint8_t>& data = data_content.at(1);
            const uint32_t time = data[3] << 24 | data.at(2) << 16 | data.at(1) << 8 | data.at(0);
            std::cout << "time_created = " << time << "\n";
        } else if (data_global_number == 19) {
            const std::vector<uint8_t>& data = data_content.at(17);
            double speed = data[1] << 8 | data[0];
            speed /= 1000;
            speed /= 1000;
            speed *= 60;
            speed *= 60;
            std::printf("avg_speed = %.3f km/h\n", speed);
        }
    }
    //crc
    uint16_t final_crc;
    input.read(reinterpret_cast<char *>(&final_crc), sizeof(uint16_t));
    if (check_crc(input, static_cast<uint32_t>(input.tellg()) - 2) != final_crc) {
        return EXIT_FAILURE;
    }
    std::cout << "File CRC ok\n";
    return EXIT_SUCCESS;
}

int main(const int argc, const char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    return decode(input);
}
