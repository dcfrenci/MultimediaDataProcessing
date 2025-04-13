#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

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

namespace mdp {
    template<typename T>
    struct vector {
        size_t size_, capacity_;
        T *data_;
        // Copy and swap idiom --> how to avoid writing two separate
        // assignment operators.
        vector() {
            // default constructor
            printf("default constructor\n");
            size_ = 0;
            capacity_ = 10;
            data_ = new T[capacity_]; //(T *) malloc(capacity_ * sizeof(T));
        }

        vector(const vector &other) {
            // copy constructor
            printf("copy constructor\n");
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new T[capacity_]; //(T *) malloc(capacity_ * sizeof(T));
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }

        vector(vector &&other) {
            // move constructor
            printf("move constructor\n");
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.data_ = nullptr;
        }

        //vector &operator=(const vector &other) {
        //    if (this == &other) {
        //        return *this;
        //    }
        //    free(data_);
        //    size_ = other.size_;
        //    capacity_ = other.capacity_;
        //    data_ = (int *)malloc(capacity_ * sizeof(int));
        //    for (size_t i = 0; i < size_; ++i) {
        //        data_[i] = other.data_[i];
        //    }
        //    return *this;
        //}
        vector &operator=(const vector &other) {
            // copy assignment
            printf("copy assignment\n");
            if (capacity_ < other.size_) {
                delete[] data_; //free(data_);
                capacity_ = other.capacity_;
                data_ = new T[capacity_]; //(T *) malloc(capacity_ * sizeof(T));
            }
            size_ = other.size_;
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
            return *this;
        }

        vector &operator=(vector &&other) {
            // move assignment
            printf("move assignment\n");
            size_ = other.size_;
            capacity_ = other.capacity_;
            std::swap(data_, other.data_);
            return *this;
        }

        ~vector() {
            // destructor
            printf("destructor\n");
            delete[] data_; //free(data_);
        }

        void push_back(const T &num) {
            if (size_ == capacity_) {
                capacity_ *= 2;
                // allocate new memory
                T *tmp = new T[capacity_];
                // copy old data in new memory
                for (size_t i = 0; i < size_; ++i) {
                    tmp[i] = data_[i];
                }
                // delete old memory
                delete[] data_;
                //data_ = (T *) realloc(data_, capacity_ * sizeof(T));
                data_ = tmp;
            }
            data_[size_] = num;
            size_++;
        }

        size_t size() const {
            return size_;
        }

        const T &at(size_t index) const {
            assert(index < size_);
            return data_[index];
        }

        const T &operator[](size_t index) const {
            //const vector<T> this, size_t index
            assert(index < size_); //     ||
            return data_[index]; //Have different parameters even if they look similar (const is missing)
        } //     ||
        T &operator[](size_t index) {
            //vector<T> this, size_t index
            assert(index < size_);
            return data_[index];
        }
    };
}

void print(FILE *f, const mdp::vector<int> &v) {
    for (size_t i = 0; i < v.size(); i++) {
        fprintf(f, "%d\n", v.at(i));
    }
}

mdp::vector<int> read(FILE *f) {
    using namespace mdp;
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
};

int main(const int argc, char *argv[]) { {
        // widget *p = static_cast<widget *>(malloc(sizeof(widget)));      // doesn't call constructor when allocating memory
        // free(p);
        // widget *q = new widget();                      //Allocate memory AND initialize the memory
        // delete q;
        // widget* w = new widget[3];                     //Allocate memory for 3 different widget
        // delete[] w;                                    //Usually compiler store before the pointer

        using namespace mdp;

        vector<widget> vec;
        vec.push_back(widget(7));
        vec.push_back(widget(25));
        vec.push_back(widget(123));
        vec.push_back(widget(7));
        vec.push_back(widget(25));
        vec.push_back(widget(123));
        vec.push_back(widget(7));
        vec.push_back(widget(25));
        vec.push_back(widget(123));
        vec.push_back(widget(7));
        vec.push_back(widget(25));
        vec.push_back(widget(123));

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

        vector<int> numbers;
        numbers = read(input);
        fclose(input);

        vector<double> v;
        v.push_back(2.1);
        v.push_back(1.2);


        qsort(numbers.data_, numbers.size(), sizeof(int), compare);

        print(output, numbers);

        fclose(output);
    }
    _CrtDumpMemoryLeaks();
    return 0;
}
