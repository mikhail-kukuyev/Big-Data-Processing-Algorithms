#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

/*void print_array(const unsigned long long* arr, int n) {
    for (int i = 0; i < n; ++i) {
        cout << arr[i] << " ";
    }
    cout << endl;
}*/

/*
void init_data() {
    FILE *f = nullptr;

    f = fopen("input.bin", "w+b");
    if (f == nullptr) {
        printf("Error opening file");
    }


    const unsigned long long SIZE = 1000000;
    auto * a = new unsigned long long[SIZE];
    for (int i = 0; i <SIZE; ++i) {
        a[i] = SIZE - i;
    }

    fwrite(&SIZE, 8, 1, f);
    fwrite(a, 8,SIZE, f);

    delete[] a;
    fclose(f);
}
*/

char* filename(unsigned int block_size, int i, char* str) {
    sprintf(str, "%d_%d.bin", block_size, i);
    return str;
}

char buf[20];

void merge_2_files(unsigned long long* a, unsigned long long* b, unsigned long long* c, int iter, unsigned int block_size, const unsigned int PART_SIZE) {
    FILE* f1 = fopen(filename(block_size, iter, buf), "rb");
    unsigned long long f1_size;
    fread(&f1_size, 8, 1, f1);

    FILE* f2 = fopen(filename(block_size, iter + 1, buf), "rb");
    unsigned long long f2_size;
    fread(&f2_size, 8, 1, f2);

    FILE* f3 = fopen(filename(2 * block_size, iter / 2, buf), "w+b");
    unsigned long long f3_size = f1_size + f2_size;
    fwrite(&f3_size, 8, 1, f3);

    unsigned int a_rest = 0, b_rest = 0;
    unsigned int f1_count = 0, f2_count = 0;
    unsigned int a_size = 0, b_size = 0;

    while (f1_count < f1_size || f2_count < f2_size || a_rest != 0 || b_rest != 0) {
        if (a_size == 0 ) {
            unsigned int f1_read = min(PART_SIZE, (unsigned int)(f1_size - f1_count));
            if (f1_read != 0) {
                fread(a, 8, f1_read, f1);
                f1_count += f1_read;
                a_size = f1_read;
            }
        }

        if (b_size == 0) {
            unsigned int f2_read = min(PART_SIZE, (unsigned int)(f2_size - f2_count));
            if (f2_read != 0) {
                fread(b, 8, f2_read, f2);
                f2_count += f2_read;
                b_size = f2_read;
            }
        }

        {
            int i = 0, j = 0, k = 0;

            if (a_size == 0) {
                copy(b, b + b_size, c);
                j = b_size;
            }
            else if (b_size == 0) {
                copy(a, a + a_size, c);
                i = a_size;
            }

            while (i < a_size && j < b_size)
                c[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];

            if (i < a_size) {
                copy(a + i, a + a_size, a);
                a_rest = a_size - i;
                b_rest = 0;
            }
            else if (j < b_size) {
                copy(b + j, b + b_size, b);
                b_rest = b_size - j;
                a_rest = 0;
            }
            else a_rest = b_rest = 0;
        }


        fwrite(c, 8, a_size + b_size - a_rest - b_rest, f3);

        a_size = a_rest;
        b_size = b_rest;
    }

    fclose(f1);
    fclose(f2);
    fclose(f3);
}

/*void read_file() {
    FILE *f = nullptr;

    f = fopen("output.bin", "r+b");
    if (f == nullptr) {
        printf("Error opening file");
    }

    unsigned long long N;
    fread(&N, 8, 1, f);
    cout << N << endl;

    auto * arr = new unsigned long long[N];

    fread(arr, 8, N, f);
    for (int i = 0; i < N; ++i) {
        cout << arr[i] << " ";
    }
    cout << endl;

    fclose(f);
    delete[] arr;
}*/

int main() {
//    init_data();
//    read_file();

    unsigned int PART_SIZE = 15000;
    unsigned long long N;

    FILE *ifile = nullptr;
    FILE *ofile = nullptr;

    ifile = fopen("input.bin", "rb");
    if (ifile == nullptr) {
        printf("Error opening file");
        exit(1);
    }

    fread(&N, 8, 1, ifile);
//    if (PART_SIZE > N) PART_SIZE = N;

    auto * arr = new unsigned long long[4 * PART_SIZE];
    unsigned long long* a = arr;
    unsigned long long* b = arr + PART_SIZE;
    unsigned long long* c = arr + 2 * PART_SIZE;

    unsigned int block_size = min(4 * PART_SIZE, (unsigned int)N);

    //split
    for (int i = 0; i < N; i += block_size) {
        unsigned long long n = min(block_size, (unsigned int)(N - i));
        fread(arr, 8, n, ifile);

        sort(arr, arr + n);

        ofile = fopen(filename(block_size, i / block_size, buf), "w+b");

        fwrite(&n, 8, 1, ofile);
        fwrite(arr, 8, n, ofile);

        fclose(ofile);
    }
    fclose(ifile);

    //merge

    while(block_size <= N) {
        unsigned int files_number = (unsigned int)ceil(N * 1.0 / block_size);

        if (files_number == 1) {
            rename(filename(block_size, 0, buf), "output.bin");
            break;
        }

        for (int i = 0; i < files_number; i += 2) {
            if (i+1 >= files_number) {
                char buf2[20];
                rename(filename(block_size, i, buf), filename(2 * block_size, i / 2, buf2));
                break;
            }

            merge_2_files(a, b, c, i, block_size, PART_SIZE);
        }

        if (2 * block_size > N) {
            rename(filename(2 * block_size, 0, buf), "output.bin");
            break;
        }

        block_size *= 2;
    }

    delete[] arr;
}
