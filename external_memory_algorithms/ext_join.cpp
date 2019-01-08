#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;

void print_array(const int* arr, int n) {
    for (int i = 0; i < n; ++i) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

char* filename(int block_size, int i, char* str) {
    sprintf(str, "%d_%d.bin", block_size, i);
    return str;
}

char buf[20];

struct Pair {
    int first;
    int second;
};

struct Triple {
    int first;
    int second;
    int third;
};

bool cmp_pair(const Pair& l, const Pair& r, bool sortBySecond) {
    return sortBySecond ? l.second <= r.second : l.first <= r.first;
}

void merge_2_files(int* memory, const char* filename1, const char* filename2, const char* outfilename,
                   int tuple_size, bool sort_by_second, const int PART_SIZE) {
    int* a = memory;
    int* b = memory + PART_SIZE;
    int* c = memory + 2 * PART_SIZE;

    FILE* f1 = nullptr;
    int f1_size = 0;
    if (filename1 != nullptr) {
        f1 = fopen(filename1, "rb");
        fread(&f1_size, 4, 1, f1);
        f1_size *= tuple_size;
    }

    FILE* f2 = nullptr;
    int f2_size = 0;
    if (filename2 != nullptr) {
        f2 = fopen(filename2, "rb");
        fread(&f2_size, 4, 1, f2);
        f2_size *= tuple_size;
    }

    FILE* f3 = fopen(outfilename, "wb");
    int f3_size = (f1_size + f2_size) / tuple_size;
    fwrite(&f3_size, 4, 1, f3);

    int a_rest = 0, b_rest = 0;
    int f1_count = 0, f2_count = 0;
    int a_size = 0, b_size = 0;

    while (f1_count < f1_size || f2_count < f2_size || a_rest != 0 || b_rest != 0) {
        if (a_size == 0 ) {
            int f1_read = min(PART_SIZE, f1_size - f1_count);
            if (f1_read > 0) {
                fread(a, 4, f1_read, f1);
                f1_count += f1_read;
                a_size = f1_read;
            }
        }

        if (b_size == 0) {
            int f2_read = min(PART_SIZE, f2_size - f2_count);
            if (f2_read > 0) {
                fread(b, 4, f2_read, f2);
                f2_count += f2_read;
                b_size = f2_read;
            }
        }

        int i = 0, j = 0, k = 0;

        if (a_size == 0) {
            copy(b, b + b_size, c);
            j = b_size;
        }
        else if (b_size == 0) {
            copy(a, a + a_size, c);
            i = a_size;
        }

        while (i < a_size && j < b_size) {
//            c[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];
            if (tuple_size == 2) {
                auto * p1 = (Pair*)&a[i];
                auto * p2 = (Pair*)&b[j];
                auto * p3 = (Pair*)&c[k];
                if (cmp_pair(*p1, *p2, sort_by_second)) {
                    *p3 = *p1;
                    i += tuple_size;
                }
                else {
                    *p3 = *p2;
                    j += tuple_size;
                }
            }
            else if (tuple_size == 3) {
                auto * p1 = (Triple*)&a[i];
                auto * p2 = (Triple*)&b[j];
                auto * p3 = (Triple*)&c[k];
                if (p1->first <= p2->first) {
                    *p3 = *p1;
                    i += tuple_size;
                }
                else {
                    *p3 = *p2;
                    j += tuple_size;
                }
            }

            k += tuple_size;
        }

        if (i < a_size && j >= b_size) {
            copy(a + i, a + a_size, a);
            a_rest = a_size - i;
            b_rest = 0;
        }
        else if (j < b_size && i >= a_size) {
            copy(b + j, b + b_size, b);
            b_rest = b_size - j;
            a_rest = 0;
        }
        else a_rest = b_rest = 0;


        fwrite(c, 4, a_size + b_size - a_rest - b_rest, f3);

        a_size = a_rest;
        b_size = b_rest;
    }

    if (f1 != nullptr)
        fclose(f1);
    if (f2 != nullptr)
        fclose(f2);
    fclose(f3);
}

void merge_sort(int* memory, const char* inputFilename, const char* outputFilename, int tuple_size, bool sortBySecond, const int PART_SIZE) {
    FILE *ifile = nullptr;
    FILE *ofile = nullptr;
    int N;

    ifile = fopen(inputFilename, "rb");

    fread(&N, 4, 1, ifile);
    N *= tuple_size;

    int block_size = min(4 * PART_SIZE, N);

    //split
    for (int i = 0; i < N; i += block_size) {
        int n = min(block_size, N - i);
        fread(memory, 4, n, ifile);

        if(tuple_size == 2) {
            auto * pairs = (Pair*)memory;
            if (sortBySecond) {
                sort(pairs, pairs + n / tuple_size, [] (const Pair& l, const Pair& r) {
                    return l.second < r.second;
                });
            }
            else {
                sort(pairs, pairs + n / tuple_size, [](const Pair& l, const Pair& r) {
                    return l.first < r.first;
                });
            }
        }
        else if (tuple_size == 3) {
            auto * triples = (Triple*)memory;
            sort(triples, triples + n / tuple_size, [] (const Triple& l, const Triple& r) {
                return l.first < r.first;
            });
        }

        ofile = fopen(filename(block_size, i / block_size, buf), "wb");

        int output_size = n / tuple_size;
        fwrite(&output_size, 4, 1, ofile);
        fwrite(memory, 4, n, ofile);

        fclose(ofile);
    }
    fclose(ifile);

    //merge
    while(block_size <= N) {
        auto files_number = ceil(N * 1.0 / block_size);

        if (files_number == 1) {
            merge_2_files(memory, filename(block_size, 0, buf), NULL, outputFilename,tuple_size, sortBySecond, PART_SIZE);
            break;
        }

        for (int i = 0; i < files_number; i += 2) {
            char filename1[20];
            filename(block_size, i, filename1);

            char* filename2 = NULL;
            if (i+1 < files_number)
                filename2 = filename(block_size, i + 1, buf);

            char outfilename[20];
            filename(2 * block_size, i / 2, outfilename);

            merge_2_files(memory, filename1, filename2, outfilename, tuple_size, sortBySecond, PART_SIZE);
        }

        if (2 * block_size > N) {
            merge_2_files(memory, filename(2 * block_size, 0, buf), NULL, outputFilename,tuple_size, sortBySecond, PART_SIZE);
            break;
        }

        block_size *= 2;
    }
}

void join(int *memory, const char *outputFilename, const int PART_SIZE) {
    int* a = memory;
    int* b = memory + PART_SIZE;
    int* c = memory + 2 * PART_SIZE;

    FILE* f1 = fopen("output1.bin", "rb");
    int f1_size;
    fread(&f1_size, 4, 1, f1);
    f1_size *= 2;

    FILE* f2 = fopen("output2.bin", "rb");
    int f2_size;
    fread(&f2_size, 4, 1, f2);
    f2_size *= 2;

    FILE* f3 = fopen(outputFilename, "wb");
    int size_output = f1_size / 2;
    fwrite(&size_output, 4, 1, f3);

    for (int i = 0; i < f1_size; i += PART_SIZE) {
        int n = min(PART_SIZE, f1_size - i);
        fread(a, 4, n, f1);
        fread(b, 4, n, f2);

        int k = 0;
        for (int j = 0; j < n; j+=2) {
            c[k] = a[j];
            c[k+1] = a[j+1];
            c[k+2] = b[j+1];
            k += 3;
        }

        fwrite(c, 4, k, f3);
    }
    fclose(f3);
}

void copy_to_result_file(int *memory, const char *inputFilename, const char *outputFilename, const int PART_SIZE) {
    FILE* ifile = fopen(inputFilename, "rb");
    int size;
    fread(&size, 4, 1, ifile);
    size *= 3;

    FILE* ofile = fopen(outputFilename, "wb");

    int block_size = min(4 * PART_SIZE, size);

    for (int i = 0; i < size; i += block_size) {
        int n = min(block_size, size - i);
        fread(memory, 4, n, ifile);
        fwrite(memory, 4, n, ofile);
    }
    fclose(ifile);
    fclose(ofile);
}

void ext_join(const int PART_SIZE) {
    auto* memory = new int[4 * PART_SIZE];

    merge_sort(memory, "input.bin", "output1.bin", 2, true, PART_SIZE);
    merge_sort(memory, "input.bin", "output2.bin", 2, false, PART_SIZE);

    join(memory, "output3.bin", PART_SIZE);

    merge_sort(memory, "output3.bin", "output4.bin", 3, false, PART_SIZE);

    copy_to_result_file(memory, "output4.bin", "output.bin", PART_SIZE);

    delete[] memory;
}

void init_data(const int SIZE) {
    FILE *f = nullptr;

    f = fopen("input.bin", "wb");
    if (f == nullptr) {
        printf("Error opening file");
    }

    int* arr = new int[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        arr[i] = i + 1;
    }
    long seed = chrono::system_clock::now().time_since_epoch().count();
//    shuffle (arr, arr + SIZE, default_random_engine(seed));

    int* a = new int[2*SIZE];
    for (int i = 0; i < SIZE - 1; i++) {
        a[2*i] = arr[i];
        a[2*i+1] = arr[i+1];
    }
    a[2*(SIZE-1)] = arr[SIZE-1];
    a[2*(SIZE-1)+1] = arr[0];

    fwrite(&SIZE, 4, 1, f);
    fwrite(a, 4, 2 * SIZE, f);

    fclose(f);
    delete[] arr;
    delete[] a;
}


int* read_file(const char* filename, const int size) {
    FILE* f = fopen(filename, "rb");

    auto* arr = new int[size];
    fread(arr, 4, size, f);
    return arr;
}

int main() {
//    init_data(size);

    ext_join(30000);
}
