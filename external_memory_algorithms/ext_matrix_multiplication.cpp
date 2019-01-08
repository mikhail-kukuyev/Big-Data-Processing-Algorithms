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

void MultSwappedLoops(const unsigned char *__restrict a, const unsigned char *__restrict b, unsigned char *__restrict c,
                      int n, int m, int l, bool initC) {
    for (int i = 0; i < n; ++i) {
        if (initC) {
            for (int j = 0; j < l; ++j) {
                c[i * l + j] = 0;
            }
        }

        for (int k = 0; k < m; ++k) {
            for (int j = 0; j < l; ++j) {
                c[i * l + j] = (c[i * l + j] + a[i * m + k] * b[k * l + j]) % 256;
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

    unsigned char a[9] = {2, 0, 0, 0, 1, 0, 0, 0, 1};
    unsigned char b[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    int size = 3;
    fwrite(&size, 4, 1, f);
    fwrite(&size, 4, 1, f);
    fwrite(a, 1, 9, f);

    fwrite(&size, 4, 1, f);
    fwrite(&size, 4, 1, f);
    fwrite(b, 1, 9, f);

    fclose(f);
}

void read_block(FILE *f, unsigned char *pBlock, int n, int m, int N) {
    for (int i = 0; i < n; ++i) {
        fread(pBlock + i * m, 1, m, f);
        fseek(f, N - m, SEEK_CUR);
    }
}

void write_block(FILE *f, unsigned char *pBlock, int n, int m, int N) {
    for (int i = 0; i < n; ++i) {
        fwrite(pBlock + i * m, 1, m, f);
        fseek(f, N - m, SEEK_CUR);
    }
}

int main() {
    const int s = 400;
    int N, N1, N2, M1, M2;
    unsigned char a[s*s];
    unsigned char b[s*s];
    unsigned char c[s*s];

    FILE *ifile = NULL;
    FILE *ofile = NULL;

    ifile = fopen("input.bin", "rb");
    ofile = fopen("output.bin", "w+b");
    if (ifile == NULL) {
        printf("Error opening file");
        exit(1);
    }

    fread(&N1, 4, 1, ifile);
    fread(&M1, 4, 1, ifile);

    if (N1 != M1) {
        printf("Not square matrix");
        exit(1);
    }

    N = N1;
    fwrite(&N, 4, 1, ofile);
    fwrite(&N, 4, 1, ofile);

    for (int i = 0; i < N; i += s) {
        int n = min(s, N - i);
        for (int j = 0; j < N; j += n) {
            int l = min(s, N - j);
            for (int k = 0; k < N; k += s) {
                int m = min(s, N - k);

                fseek(ifile, 8 + i * N + k, SEEK_SET);
                read_block(ifile, a, n, m, N);
                fseek(ifile, 16 + N * N + k * N + j, SEEK_SET);
                read_block(ifile, b, m, l, N);
                MultSwappedLoops(a, b, c, n, m, l, k == 0);
            }
            fseek(ofile, 8 + i * N + j, SEEK_SET);
            write_block(ofile, c, n, l, N);
        }
    }

    fclose(ifile);
    fclose(ofile);
}
