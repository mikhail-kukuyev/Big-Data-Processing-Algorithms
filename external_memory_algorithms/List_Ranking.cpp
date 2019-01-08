#include <iostream>
#include <algorithm>

struct Triple_1 {
    unsigned i;
    unsigned p_i;
    unsigned w;

    bool operator < (Triple_1 t2) {
        return i < t2.i;
    }
};

struct Triple_2 {
    unsigned i;
    unsigned p_i;
    unsigned w;

    bool operator < (Triple_2 t2) {
        return p_i < t2.p_i;
    }
};

struct Triple_3 {
    unsigned i;
    unsigned p_i;
    unsigned w;

    bool operator < (Triple_3 t2) {
        return w > t2.w;
    }
};

struct Triple {
    union {
        Triple_1 triple1;
        Triple_2 triple2;
        Triple_3 triple3;
    };
};

static const unsigned RAM_SIZE = 100080;
//static const unsigned RAM_SIZE = 3000240;
static unsigned* memory = new unsigned[RAM_SIZE];
static unsigned series_sizes[3750000 / RAM_SIZE + 1];
static unsigned list_size;
static unsigned root;
static unsigned* a;
static unsigned* b;
static unsigned* c;
static unsigned block_size;
static unsigned n_read;

static FILE *tempfile, *mergefile1, *mergefile2, *sortedfile1, *sortedfile2, *joinedfile;

unsigned min(const unsigned& a, const unsigned& b) {
    return a < b ? a : b;
}

void open_files() {
    tempfile = fopen("tempfile.bin", "w+b");
    mergefile1 = fopen("mergefile1.bin", "w+b");
    mergefile2 = fopen("mergefile2.bin", "w+b");
    sortedfile1 = fopen("sortedfile1.bin", "w+b");
    sortedfile2 = fopen("sortedfile2.bin", "w+b");
    joinedfile = fopen("joinedfile.bin", "w+b");
}

void close_files() {
    fclose(tempfile);
    fclose(mergefile1);
    fclose(mergefile2);
    fclose(sortedfile1);
    fclose(sortedfile2);
    fclose(joinedfile);
}

void merge(FILE* from_file, FILE* to_file, const unsigned &SERIES_NUMBER, const unsigned &sort_type) {
    block_size = RAM_SIZE / 4;

    a = memory;
    b = memory + block_size;
    c = memory + 2 * block_size;

    Triple *p1;
    Triple *p2;
    Triple *p3;
    bool cmp;
    unsigned copy_size;

    unsigned total_read1, total_read2;
    unsigned series1_end, series2_end;
    unsigned a_size, b_size, c_size;
    unsigned i, j, k;

    c_size = 2 * block_size;

    fseek(to_file, 0, SEEK_SET);

    total_read1 = 0, total_read2 = 0;
    for (unsigned series_i = 0; series_i < SERIES_NUMBER; series_i += 2) {
        total_read1 = total_read2;
        series1_end = total_read1 + 3 * series_sizes[series_i];

        //if next series does not exist, just copy last to file
        if (series_i + 1 == SERIES_NUMBER) {
            fseek(from_file, 4 * total_read1, SEEK_SET);
            for (; total_read1 < 3*list_size; total_read1 += n_read) {
                n_read = (unsigned) fread(memory, 4, RAM_SIZE, from_file);
                fwrite(memory, 4, n_read, to_file);
            }
            break;
        }

        total_read2 = series1_end;
        series2_end = total_read2 + 3 * series_sizes[series_i + 1];

        i = j = a_size = b_size = block_size;
        k = 0;
        while (total_read1 != series1_end || total_read2 != series2_end || k != 0) {
            if (a_size != 0 && i == a_size) {
                if (total_read1 < series1_end) {
                    a_size = min(block_size, series1_end - total_read1);
                    fseek(from_file, 4 * total_read1, SEEK_SET);
                    fread(a, 4, a_size, from_file);
                    total_read1 += a_size;
                    i = 0;
                }
                else {
                    i = a_size = 0;
                }
            }
            if (b_size !=0 && j == b_size) {
                if (total_read2 < series2_end) {
                    b_size = min(block_size, series2_end - total_read2);
                    fseek(from_file, 4 * total_read2, SEEK_SET);
                    fread(b, 4, b_size, from_file);
                    total_read2 += b_size;
                    j = 0;
                }
                else {
                    j = b_size = 0;
                }
            }

            if (a_size == 0 && b_size == 0) {
                fwrite(c, 4, k, to_file);
                break;
            }
            if (k == c_size) {
                fwrite(c, 4, k, to_file);
                k = 0;
            }

            if (a_size == 0) {
                copy_size = min(b_size - j, c_size - k);
                std::copy(b + j, b + j + copy_size, c + k);
                k += copy_size;
                j += copy_size;
            }
            else if (b_size == 0) {
                copy_size = min(a_size - i, c_size - k);
                std::copy(a + i, a + i + copy_size, c + k);
                k += copy_size;
                i += copy_size;
            }
            else {
                while (i < a_size && j < b_size && k < c_size) {
//            c[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];

                    p1 = (Triple *) &a[i];
                    p2 = (Triple *) &b[j];
                    p3 = (Triple *) &c[k];

                    if (sort_type == 1) {
                        cmp = (p1->triple1 < p2->triple1);
                    } else if (sort_type == 2) {
                        cmp = (p1->triple2 < p2->triple2);
                    } else if (sort_type == 3) {
                        cmp = (p1->triple3 < p2->triple3);
                    }
                    else {
                        fprintf(stdout, "Undefined sort_type\n");
                        exit(1);
                    }

                    if (cmp) {
                        *p3 = *p1;
                        i += 3;
                    } else {
                        *p3 = *p2;
                        j += 3;
                    }

                    k += 3;
                }
            }
        }
    }
}

void merge_sort(FILE* ifile, FILE* ofile, const unsigned& sort_type) {
    fseek(ifile, 0, SEEK_SET);

    FILE* fileptr = (RAM_SIZE < list_size * 3) ? mergefile1 : ofile;
    fseek(fileptr, 0, SEEK_SET);

    //split
    unsigned counter = 0;
    for (unsigned i = 0; i < 3 * list_size; i += n_read) {
        n_read = (unsigned) fread(memory, 4, RAM_SIZE, ifile);

        if (sort_type == 1) {
            std::sort((Triple_1*)memory, (Triple_1*)memory + n_read / 3);
        }
        else if (sort_type == 2) {
            std::sort((Triple_2*)memory, (Triple_2*)memory + n_read / 3);
        }
        else if (sort_type == 3) {
            std::sort((Triple_3*)memory, (Triple_3*)memory + n_read / 3);
        }
        else {
            fprintf(stderr, "Undefined sort_type\n");
            exit(1);
        }

        fwrite(memory, 4, n_read, fileptr);
        series_sizes[counter] = n_read / 3;
        counter++;
    }

    //merge
    unsigned series_size = RAM_SIZE;
    unsigned series_number;
    unsigned iteration = 0;
    while (series_size < 3*list_size) {
        series_number = (3*list_size + series_size - 1) / series_size;

        if (series_number > 2) {
            if (iteration & 1) {
                merge(mergefile2, mergefile1, series_number, sort_type);
            }
            else {
                merge(mergefile1, mergefile2, series_number, sort_type);
            }
        }
        else {
            if (iteration & 1) {
                merge(mergefile2, ofile, series_number, sort_type);
            }
            else {
                merge(mergefile1, ofile, series_number, sort_type);
            }
        }

        for (unsigned l = 0; l < series_number; l += 2) {
            if (l == series_number - 1) {
                series_sizes[l/2] = series_sizes[l];
            }
            else {
                series_sizes[l/2] = series_sizes[l] + series_sizes[l + 1];
            }
        }

        series_size *= 2;
        iteration++;
    }
}

bool join() {
    block_size = RAM_SIZE / 3;

    a = memory;
    b = memory + block_size;
    c = memory + block_size * 2;

    fseek(sortedfile2, 0, SEEK_SET);
    fseek(sortedfile1, 0, SEEK_SET);
    fseek(joinedfile, 0, SEEK_SET);

    bool last_join = true;
    unsigned b_i, b_n;
    b_i = b_n = (unsigned)fread(b, 4, block_size, sortedfile1);
    unsigned k = 0;

    for (unsigned i = 0; i < 3 * list_size; i += n_read) {
        n_read = (unsigned)fread(a, 4, block_size, sortedfile2);

        for (unsigned j = 0; j < n_read; j += 3) {
            while (a[j + 1] != b[k]) {
                k += 3;
                if (k == b_n) {
                    b_n = (unsigned)fread(b, 4, block_size, sortedfile1);
                    b_i += b_n;
                    k = 0;
                    if (b_n == 0 || b_i > 3*list_size) {
                        fprintf(stderr, "Infinite cycle\n i=%d j=%d a[j+1]=%d block_size=%d", i, j, a[j+1], block_size);
                        exit(2);
                    }
                }
            }
            c[j] = a[j];
            c[j + 1] = b[k + 1];
            c[j + 2] = a[j + 2] + b[k + 2];

            if (last_join && c[j + 1] != root) {
//                printf("%d w=%d root=%d\n", c[j+1], c[j+2], root);
                last_join = false;
            }
        }
        fwrite(c, 4, n_read, joinedfile);
    }

    return last_join;
}

inline void cyclic_join() {
    while (!join()) {
        merge_sort(joinedfile, sortedfile2, 2);
        merge_sort(joinedfile, sortedfile1, 1);
    }
}

void create_weighted(const char* ifilename) {
    block_size = RAM_SIZE / 5 * 2;
    a = memory;
    b = memory + block_size;

    FILE* ifile = fopen(ifilename, "rb");
    fread(&list_size, 4, 1, ifile);

    fseek(tempfile, 0, SEEK_SET);

    for (unsigned i = 0; i < 2*list_size; i += block_size) {
        n_read = (unsigned) fread(a, 4, block_size, ifile);
        for (unsigned j = 0; j < n_read / 2; ++j) {
            b[3*j] = a[2*j];
            b[3*j+1] = a[2*j+1];
            b[3*j+2] = 1;
        }
        fwrite(b, 4, n_read / 2 * 3, tempfile);
    }
    fclose(ifile);
}

void change_root_weight(FILE* file) {
    fseek(file, 0, SEEK_SET);
    fread(&root, 4, 1, file);

    unsigned p_w[2];
    p_w[0] = root;
    p_w[1] = 0;
    fseek(file, 0, SEEK_CUR);
    fwrite(p_w, 4, 2, file);
}

void print_results(const char* ofilename) {
    block_size = RAM_SIZE / 4;

    a = memory;
    b = memory + block_size * 3;

    fseek(tempfile, 0, SEEK_SET);

    FILE* ofile = fopen(ofilename, "wb");

    for (unsigned i = 0; i < 3*list_size; i += 3*block_size) {
        n_read = (unsigned) fread(a, 4, 3*block_size, tempfile);
        for (unsigned j = 0; j < n_read / 3; ++j) {
            b[j] = a[3*j];
        }
        fwrite(b, 4, n_read / 3, ofile);
    }
    fclose(ofile);
}

void ranking() {
    open_files();

    create_weighted("input.bin");

    merge_sort(tempfile, sortedfile2, 2);
    change_root_weight(sortedfile2);
    merge_sort(sortedfile2, sortedfile1, 1);
    merge_sort(sortedfile1, sortedfile2, 2);

    cyclic_join();
    merge_sort(joinedfile, tempfile, 3);
    print_results("output.bin");

    close_files();
}

int main() {
    ranking();
    delete[] memory;
}
