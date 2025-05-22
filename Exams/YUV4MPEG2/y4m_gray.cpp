#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "mat.h"
#include "pgm.h"
#include "types.h"
#include "utils.h"

bool y4m_extract_gray(const std::string &filename, std::vector<mat<uint8_t> > &frames) {
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        return false;
    }
    std::string head("YUV4MPEG2");
    input.read(head.data(), static_cast<long long>(head.size()));
    if (head != "YUV4MPEG2") {
        return false;
    }
    std::map<uint8_t, std::string> tag_field;
    std::string string;
    while (tag_field.size() != 7) {
        int8_t ch;
        input.read(reinterpret_cast<char *>(&ch), 1);
        if (ch == 0x20) {
            if (!string.empty()) {
                tag_field[string.at(0)] = string.substr(1);
            }
            string = "";
            continue;
        }
        if (ch == 0x0A) {
            tag_field[string.at(0)] = string.substr(1);
            break;
        }
        string.push_back(ch);
    }
    if (tag_field.find('C') != tag_field.end() and  tag_field['C'] != "420jpeg") {
        return false;
    }
    string = "";
    while (!input.eof()) {
        mat<uint8_t> y_frame(std::stoi(tag_field['H']), std::stoi(tag_field['W']));
        std::getline(input, string, '\n');
        if (string.find("FRAME") == std::string::npos) {
            break;
        }
        input.read(y_frame.rawdata(), y_frame.rawsize());
        frames.push_back(y_frame);
        input.seekg(y_frame.cols() / 4 * y_frame.rows() / 4, std::ios::cur);
    }
    return true;
}


int main(const int argc, const char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::vector<mat<uint8_t> > frames;
    y4m_extract_gray(argv[1], frames);
    uint32_t id = 0;
    const std::string dir = argv[2];
    for (auto &frame: frames) {
        std::string string = dir + "/frame_" + std::to_string(id) + ".pgm";
        save_pgm(string, frame, false);
        id++;
    }
}
