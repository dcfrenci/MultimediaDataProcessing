#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct vector {
    size_t size, capacity;
    int *data;

    void constractor() {
        this->size = 0;
        this->capacity = 10;
        this->data = (int*) calloc(this->capacity, sizeof(int));
        if (this->data == NULL) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }
    }

    void destructor(const struct vector *this) {
        free(this->data);
    }

    void push_back(struct vector *this, const int num) {
        if (this->size == this->capacity) {
            this->capacity *= 2;
            this->data = realloc(this->data, this->capacity * sizeof(int));
        }
        this->data[this->size++] = num;
        this->size++;
    }

    size_t size(const struct vector *this) {
        return this->size;
    }

    int at(size_t index) const  {
        assert(index < this->size);             //Executed only during debugging
        return this->data[index];
    }
};



int compare(const void *a, const void *b) {
    if (*(int*)a > *(int*)b) return 1;
    if (*(int*)a < *(int*)b) return -1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
    }
    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("File input not found\n");
    }
    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        printf("File output not found\n");
    }

    struct vector numbers;
    constractor(&numbers);
    int num;
    while (fscanf(inputFile, "%d", &num) != EOF) {
        push_back(&numbers, num);
    }
    fclose(inputFile);

    qsort(numbers.data, numbers.size, sizeof(int), compare);

    for (size_t i = 0; i < size(&numbers); i++) {
        fprintf(outputFile, "%d\n", at(&numbers, i));
    }

    fclose(outputFile);
    destructor(&numbers);
    return 0;
}
