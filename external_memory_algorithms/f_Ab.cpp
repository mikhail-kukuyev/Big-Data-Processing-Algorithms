#include <iostream>
#include <memory>
using namespace std;

void multiply(const unsigned char *__restrict a, const unsigned char *__restrict b, unsigned char *__restrict c,
              int n, int m, bool initC) {
    for (int i = 0; i < n; ++i) {
        if (initC) {
            c[i] = 0;
        }
        for (int j = 0; j < m; ++j) {
            c[i] += a[i*m + j] * b[j] % 256;
        }
    }
}

void init_data() {
    FILE *f = fopen("input.bin", "w+b");
    unsigned char a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0};
    unsigned char b[4] = {1, 2, 3, 4};

    int size1 = 3;
    int size2 = 4;
    fwrite(&size1, 4, 1, f);
    fwrite(&size2, 4, 1, f);
    fwrite(a, 1, 12, f);
    fwrite(b, 1, 4, f);

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

int main() {
//    init_data();

    const int BLOCK_SIZE = 500;
    int block_n, block_m;
    int N, M;

    FILE *ifile = nullptr;
    FILE *ofile = nullptr;

    ifile = fopen("input.bin", "rb");
    ofile = fopen("output.bin", "w+b");

    fread(&N, 4, 1, ifile);
    fread(&M, 4, 1, ifile);

    block_n = min(BLOCK_SIZE, N);
    block_m = min(M, BLOCK_SIZE * BLOCK_SIZE / block_n);
    block_n = min(N, BLOCK_SIZE * BLOCK_SIZE / block_m); //enlarge block_n if block_m < BLOCK_SIZE

    unsigned char a[block_n * block_m];
    unsigned char b[block_m];
    unsigned char c[block_n];

    for (int i = 0; i < N; i += block_n) {
        int n = min(block_n, N - i);
        for (int j = 0; j < M; j += block_m) {
            int m = min(block_m, M - j);

            fseek(ifile, 8 + i * M + j, SEEK_SET);
            read_block(ifile, a, n, m, M);

            fseek(ifile, 8 + M * N + j, SEEK_SET);
            fread(b, 1, m, ifile);

            multiply(a, b, c, n, m, j==0);
        }
        fseek(ofile, i, SEEK_SET);
        fwrite(c, 1, n, ofile);
    }

    fclose(ifile);
    fclose(ofile);
}
