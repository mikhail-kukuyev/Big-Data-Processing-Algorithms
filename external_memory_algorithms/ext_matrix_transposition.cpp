#include <iostream>
#include <memory>
using namespace std;

void print_matrix(const unsigned char *matrix, int n, int m) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cout << int(matrix[i*m + j]) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void transpose(const unsigned char *__restrict a, unsigned char *__restrict b, int n, int m) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            b[j*n + i] = a[i*m + j];
        }
    }
}

void block_transpose(const unsigned char *__restrict a, unsigned char *__restrict b, int n, int m) {
    const int BLOCK = 8;
    for (int i = 0; i < n; i+=BLOCK) {
        for (int j = 0; j < m; j+=BLOCK) {
            for (int ib = 0; ib < BLOCK && i + ib < n; ++ib) {
                for (int jb = 0; jb < BLOCK && j + jb < m; ++jb) {
                    b[(j+jb) * n + (i+ib)] = a[(i+ib) * m + (j+jb)];
                }
            }
        }
    }
}

void init_data() {
    FILE *f = NULL;

    f = fopen("input.bin", "w+b");
    if (f == NULL) {
        printf("Error opening file");
    }

    unsigned char a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0};

    int size1 = 6;
    int size2 = 2;
    fwrite(&size1, 4, 1, f);
    fwrite(&size2, 4, 1, f);
    fwrite(a, 1, 12, f);

    fclose(f);
}

void read_block(FILE *f, unsigned char *pBlock, int n, int m, int M) {
    if (m == M) {
        fread(pBlock, 1, n * m, f);
    }
    else {
        for (int i = 0; i < n; ++i) {
            fread(pBlock + i * m, 1, m, f);
            fseek(f, M - m, SEEK_CUR);
        }
    }
}

void write_block(FILE *f, unsigned char *pBlock, int n, int m, int M) {
    if (m == M) {
        fwrite(pBlock, 1, n * m, f);
    }
    else {
        for (int i = 0; i < n; ++i) {
            fwrite(pBlock + i * m, 1, m, f);
            fseek(f, M - m, SEEK_CUR);
        }
    }
}

int main() {
//    init_data();

    const int BLOCK_SIZE = 500;
    int block_n, block_m;
    int N, M;

    FILE *ifile = nullptr;
    FILE *ofile = nullptr;

    ifile = fopen("input.bin", "rb");
    ofile = fopen("output.bin", "w+b");
    if (ifile == nullptr) {
        printf("Error opening file");
        exit(1);
    }

    fread(&N, 4, 1, ifile);
    fread(&M, 4, 1, ifile);

    fwrite(&M, 4, 1, ofile);
    fwrite(&N, 4, 1, ofile);

    block_n = min(BLOCK_SIZE, N);
    block_m = min(M, BLOCK_SIZE * BLOCK_SIZE / block_n);
    block_n = min(N, BLOCK_SIZE * BLOCK_SIZE / block_m); //enlarge block_n if block_m < BLOCK_SIZE

    unsigned char a[block_n * block_m];
    unsigned char b[block_m * block_n];

    for (int i = 0; i < N; i += block_n) {
        int n = min(block_n, N - i);
        for (int j = 0; j < M; j += block_m) {
            int m = min(block_m, M - j);

            fseek(ifile, 8 + i * M + j, SEEK_SET);
            read_block(ifile, a, n, m, M);
//            print_matrix(a, n, m);
            transpose(a, b, n, m);
//            print_matrix(b, m, n);

            fseek(ofile, 8 + j * N + i, SEEK_SET);
            write_block(ofile, b, m, n, N);
        }
    }

    fclose(ifile);
    fclose(ofile);
}
