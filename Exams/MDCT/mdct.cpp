#include <cmath>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <map>
#include <vector>

void calc_error(const std::vector<int16_t> &original, const std::vector<int16_t> &reconstructed, std::vector<int16_t> &error) {
    for (uint32_t i = 0; i < original.size(); i++) {
        error[i] = static_cast<int16_t>(original[i] - reconstructed[i]);
    }
}

void de_quantize(std::vector<int16_t> &samples, const int32_t quantization) {
    for (auto &sample: samples) {
        sample = static_cast<int16_t>(std::round(static_cast<double_t>(sample) * quantization));
    }
}

void quantize(std::vector<int16_t> &samples, const int32_t quantization) {
    for (auto &sample: samples) {
        sample = static_cast<int16_t>(std::round(static_cast<double_t>(sample) / quantization));
    }
}

double entropy(const std::vector<int16_t> &samples) {
    std::map<int16_t, double_t> freq;
    for (auto &sample: samples) {
        freq[sample]++;
    }
    double entropy = 0;
    for (const auto &[first, second]: freq) {
        const double_t p = second / static_cast<double_t>(samples.size());
        entropy += p * log2(p);
    }
    return -entropy;
}

void load_input(std::ifstream &input, std::vector<int16_t> &samples) {
    input.seekg(0, std::ios::end);
    const long long file_size = input.tellg();
    input.seekg(0, std::ios::beg);
    samples.resize(file_size / 2);
    input.read(reinterpret_cast<char *>(samples.data()), file_size);
}

void load_output(const std::string &file, const std::vector<int16_t> &samples) {
    std::ofstream output(file, std::ios::binary);
    if (!output) {
        return;
    }
    output.write(reinterpret_cast<const char *>(samples.data()), static_cast<long long>(samples.size() * sizeof(int16_t)));
}

void quant_force(std::ifstream &input) {
    std::vector<int16_t> samples;
    load_input(input, samples);
    std::cout << "Entropy:\t" << entropy(samples) << "\n";
    std::vector quantized{samples};
    quantize(quantized, 2600);
    std::cout << "Entropy quantized:\t" << entropy(quantized) << "\n";
    std::vector de_quantized{quantized};
    de_quantize(de_quantized, 2600);
    std::cout << "Entropy de_quantized:\t" << entropy(de_quantized) << "\n";
    std::vector<int16_t> error(samples.size());
    calc_error(samples, de_quantized, error);
    load_output("output_qt.raw", de_quantized);
    load_output("error_qt.raw", error);
}

void mdct_force(std::ifstream &input) {
    std::vector<int16_t> samples;
    load_input(input, samples);

    std::vector<int16_t> beg(1024);
    std::vector<int16_t> window(2048);
    for (uint32_t i = 0; i < samples.size() / 1024; i++) {
        std::vector<int16_t> end(1024);

        input.read(reinterpret_cast<char *>(end.data()), 1024);
        window.emplace_back(beg.begin(), beg.end());
        window.emplace_back(end.begin(), end.end());

        window.clear();
        beg = end;
    }
    std::fill(window.begin(), window.end(), 0);
    window.insert(window.begin(), beg.begin(), beg.end());
}

int main(const int argc, char **argv) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        return EXIT_FAILURE;
    }
    // quant_force(input);
    mdct_force(input);

    return EXIT_SUCCESS;
}
