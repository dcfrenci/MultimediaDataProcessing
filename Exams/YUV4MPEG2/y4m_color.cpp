#include <fstream>
#include "mat.h"
#include "pgm.h"
#include "types.h"
#include "utils.h"
#include <map>
#include <vector>

uint8_t fit(const uint8_t value, const uint8_t min, const uint8_t max) {
    return value >= min and value <= max ? value : value < min ? min : max;
}

bool y4m_extract_color(const std::string &filename, std::vector<mat<vec3b> > &frames) {
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
    if (tag_field.find('C') != tag_field.end() and tag_field['C'] != "420jpeg") {
        return false;
    }
    string = "";
    const int32_t height = std::stoi(tag_field['H']);
    const int32_t width = std::stoi(tag_field['W']);
    while (!input.eof()) {
        std::getline(input, string, '\n');
        if (string.find("FRAME") == std::string::npos) {
            break;
        }
        mat<uint8_t> y_frame(height, width);
        input.read(y_frame.rawdata(), y_frame.rawsize());
        mat<uint8_t> cb_frame(height / 2, width / 2);
        input.read(cb_frame.rawdata(), cb_frame.rawsize());
        mat<uint8_t> cr_frame(height / 2, width / 2);
        input.read(cr_frame.rawdata(), cr_frame.rawsize());

        mat<vec3b> frame(height, width);
        int32_t c_index = 0;
        for (int32_t i = 0; i < y_frame.size(); i++) {
            uint8_t y = fit(y_frame(i / width, i % width), 16, 235);
            uint8_t cb = fit(cb_frame(c_index / (width / 2), c_index % (width / 2)), 16, 240);
            uint8_t cr = fit(cr_frame(c_index / (width / 2), c_index % (width / 2)), 16, 240);
            if (i % width == 0) {
                c_index = i / width % 2 == 0 ? c_index : c_index - width / 2;
            } else if (i % 2 != 0) {
                c_index++;
            }
            vec3b pixel;
            pixel[0] = fit(static_cast<uint8_t>(1.164 * (y - 16) + 0     * (cb - 128) + 1.596 * (cr - 128)), 0, 255);
            pixel[1] = fit(static_cast<uint8_t>(1.164 * (y - 16) - 0.392 * (cb - 128) - 0.813 * (cr - 128)), 0, 255);
            pixel[2] = fit(static_cast<uint8_t>(1.164 * (y - 16) + 2.017 * (cb - 128) + 0     * (cr - 128)), 0, 255);
            frame(i / width, i % width) = pixel;
        }
        frames.push_back(frame);
    }
    return true;
}

// void write_pam(const char **argv, std::vector<mat<vec3b> > &frames) {
//     uint32_t id = 0;
//     const std::string dir = argv[2];
//     for (auto &frame: frames) {
//         std::string string = dir + "/frame_" + std::to_string(id) + ".pam";
//         std::ofstream output(string, std::ios::binary);
//         if (!output) {
//             return;
//         }
//         output << "P7\nHEIGHT " << frame.rows() << "\nWIDTH " << frame.cols() <<
//                 "\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
//         for (int32_t r = 0; r < frame.rows(); r++) {
//             for (int32_t c = 0; c < frame.cols(); c++) {
//                 for (auto &x: frame(r, c)) {
//                     output.write(reinterpret_cast<char *>(&x), 1);
//                 }
//             }
//         }
//         id++;
//     }
// }
//
// int main(const int argc, const char *argv[]) {
//     if (argc != 3) {
//         return EXIT_FAILURE;
//     }
//     std::vector<mat<vec3b> > frames;
//     y4m_extract_color(argv[1], frames);
//     write_pam(argv, frames);
//     return EXIT_SUCCESS;
// }
