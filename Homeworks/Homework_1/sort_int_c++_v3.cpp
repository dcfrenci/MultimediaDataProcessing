#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <crtdbg.h>
#include <vector>
#include <algorithm>
#include <ranges>

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


void print(FILE *f, const std::vector<int> &v) {
    //v1
    // for (size_t i = 0; i < v.size(); i++) {
    //     fprintf(f, "%d\n", v.at(i));
    // }

    //v2
    // const int *start = &v[0];
    // const int *stop = start + v.size();
    // const int *p = start;
    // while (p != stop) {
    //     fprintf(f, "%d\n", *p);
    //     p++;
    // }

    //v3
    // std::vector<int>::const_iterator start = v.begin();
    // std::vector<int>::const_iterator end = v.end();
    // std::vector<int>::const_iterator it = start;
    // while (it != end) {
    //     const int i = *it;
    //     fprintf(f, "%d\n", i);
    //     ++it;
    // }

    //v4
    // const auto start = v.begin();
    // const auto end = v.end();
    // auto it = start;
    // while (it != end) {
    //     const auto &i = *it;            //If we don't use "&" we have to call the copy constructor every time we
    //     fprintf(f, "%d\n", i);             //want to read the value contained
    //     ++it;
    // }

    //v5
    // for (auto it = begin(v), stop = end(v); it != stop; ++it) {
    //     fprintf(f, "%d\n", *it);
    // }

    //v6
    for (const auto &it: v) {
        fprintf(f, "%d\n", it);
    }
}

std::vector<int> read(FILE *f) {
    using namespace std;
    if (f == nullptr) {
        return vector<int>();
    }

    vector<int> v;
    int num;
    while (fscanf(f, "%d", &num) == 1) {
        v.push_back(num);
    }
    return v;
}

int global_id = 0;

struct widget {
    int id;
    int x;

    widget() {
        id = global_id++;
        x = 5;
    }

    widget(int value) {
        id = global_id++;
        x = value;
    }

    widget(const widget &other) {
        id = other.id;
        x = other.x;
    }

    widget &operator=(const widget &other) {
        x = other.x;
        return *this;
    }

    ~widget() {
    }

    bool operator<(const widget &rhs) const {
        return x < rhs.x;
    }
};


int main(const int argc, char *argv[]) { {
        // widget *p = static_cast<widget *>(malloc(sizeof(widget)));      // doesn't call constructor when allocating memory
        // free(p);
        // widget *q = new widget();                      //Allocate memory AND initialize the memory
        // delete q;
        // widget* w = new widget[3];                     //Allocate memory for 3 different widget
        // delete[] w;                                    //Usually compiler store before the pointer

        using namespace std;

        vector<widget> vec;

        if (argc != 3) {
            fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
            return 1;
        }
        FILE *input = fopen(argv[1], "r");
        if (!input) {
            perror("Error opening input file");
            return 1;
        }
        FILE *output = fopen(argv[2], "w");
        if (!output) {
            perror("Error opening output file");
            fclose(input);
            return 1;
        }

        vector<int> numbers = read(input);
        fclose(input);

        vector<double> v;
        v.push_back(2.1);
        v.push_back(1.2);


        //std::sort(begin(numbers), end(numbers));
        //RAIter = Random Access Iterator   //Pr = Predicate: let you choose how to sort
        // sort(begin(numbers), end(numbers));
        //ADL = Argument Dependant Lookup
        //Since the vector numbers are defined in std, then we can write sort without specifying the library (std::)
        // sort(begin(numbers), end(numbers), compareReverse);//We use different function to sort (compareReverse)
        // const comparator comp;
        // sort(begin(numbers), end(numbers), comp); //If we need to have context to sort
        // sort(begin(numbers), end(numbers),
        // [](const int a, const int b) { return a > b; }); //lamda function using [] instead (type)(fun name)

        // const int origin = 2'000'000'001;
        // const comparator comp{origin}; //Uniform initialization: call the constructor if there otherwise initialize with elm
        // sort(begin(numbers), end(numbers), comp);

        const long long origin = 2'000'000'001;
        sort(begin(numbers), end(numbers),
             [&/*&origin*/](const int a, const int b) {
                 //the & operator take what variable need from the contex
                 //the context of lambda is called a closure
                 return abs(a - origin) < abs(b - origin);
             });

        std::ranges::sort(numbers);
        print(output, numbers);
        fclose(output);
    }
    _CrtDumpMemoryLeaks();
    return 0;
}
