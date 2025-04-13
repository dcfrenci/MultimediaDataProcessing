#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <crtdbg.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ranges>
#include <iterator>

// Comparison function for qsort

int compare(const void *a, const void *b) {
    if (*(int *) a < *(int *) b) {
        return -1;
    }
    if (*(int *) a > *(int *) b) {
        return 1;
    }
    return 0;
}

// bool compareReverse(const int a, const int b) {
//     return a > b;
// }
auto compareReverse(const int a, const int b) /*-> bool*/ {
    //This version allow to decide the return type after evaluating the
    return a > b; //arguments
}

//functor
struct comparator {
    int origin;

    bool operator()(const int a, const int b) const {
        return abs(static_cast<long long>(a) - static_cast<long long>(origin)) <
               abs((long long) b - (long long) origin);
    }
};


void print(std::ostream &os, const std::vector<int> &v) {
    for (const auto &it: v) {
        // fprintf(f, "%d\n", it);
        os << it << "\n";
    }
}

std::vector<int> read(std::istream &is) {
    using namespace std;
    vector<int> v;
    int num;
    //We use the extractor operator ">>"
    while (is >> num) {
        v.push_back(num);
    }
    return v;
}

int global_id = 0;


int main(const int argc, char *argv[]) { {
        using namespace std;

        if (argc != 3) {
            // Everytime we want to convert a variable in a text version (ASCII) we use function from the print function
            // fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);

            // We use the operator "<<" to insert the parameter into the stream. The inserting works from left to right
            // After each operation the return value is &valueInput so we can concatenate multiple insertion.
            std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
            return 1;
        }
        // std::ifstream input(argv[1]);                        //Reading in text mode
        std::ifstream input(argv[1], std::ios::binary);   //Reading in binary mode (necessary on windows)

        if (!input) {
            //The "!" is an operator and returns return input.fail();
            std::cerr << "Error opening output file\n";
            return 1;
        }
        std::ofstream output(argv[2]);
        if (!output) {
            std::cerr << "Error opening output file\n";
            return 1;
        }

        // std::copy(std::istream_iterator<int>(input)
        //           , std::istream_iterator<int>()
        //           , back_inserter(numbers));
        // std::istream_iterator<int> start(input);
        // std::istream_iterator<int> stop;
        // vector<int> numbers(start, stop);

        // The most vexing parse ---> doesn't work
        // vector<int> numbers(
        //     std::istream_iterator<int> (input),
        //     std::istream_iterator<int> ()
        // );
        //Fixed by changing "(" with "{"
        vector<int> numbers{
            std::istream_iterator<int>(input),
            std::istream_iterator<int>()
        };

        sort(numbers.begin(), numbers.end());
        print(output, numbers);
        print(std::cout, numbers);

        std::stringstream ss;
        print(ss, numbers);


        int arr[10];
        std::copy(begin(numbers), end(numbers),
                  std::ostream_iterator<int>(output, "\n")
        );
    }
    _CrtDumpMemoryLeaks();
    return 0;
}
