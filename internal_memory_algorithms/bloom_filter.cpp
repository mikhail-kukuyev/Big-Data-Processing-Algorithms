#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

int custom_hash(string s) {
    int hash = 0;
    for (char i : s) {
        hash += i;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

std::hash<std::string> hash_fn;

int compute_hash(const string &s, const vector<bool> &bits, int i) {
    return abs((int)((hash_fn(s) + (i * custom_hash(s))) % bits.size()));
}

void add(const string &s, int functions_count, vector<bool> &bits) {
    for (int i = 0; i < functions_count; i++) {
        bits[compute_hash(s, bits, i)] = true;
    }
}

bool contains(const string &s, int functions_count, const vector<bool> &bits) {
    for (int i = 0; i < functions_count; i++) {
        if (!bits[compute_hash(s, bits, i)])
            return false;
    }
    return true;
}

int main() {
    const int INPUT_SIZE = 50 * 1024;
    const int OUTPUT_SIZE = 50 * 1024;
    const int SIZE = 11200 * 1024;
    const int HASHES_NUM = 6;
    int queries_num, query_type;
    string str;
    bool exists;
    vector<bool> filter(SIZE, false);

    auto * input_buf = new char[INPUT_SIZE];
    auto * output_buf = new char[OUTPUT_SIZE];

    ifstream ifile("input.txt");
    ifile.rdbuf()->pubsetbuf(input_buf, SIZE);

    ofstream ofile("output.txt");
    ofile.rdbuf()->pubsetbuf(output_buf, SIZE);

    ifile >> queries_num;
    for (int i = 0; i < queries_num; ++i) {
        ifile >> query_type >> str;

        if(query_type == 1) {
            add(str, HASHES_NUM, filter);
        }
        else {
            exists = contains(str, HASHES_NUM, filter);
            ofile << exists;
        }
    }

    ifile.close();
    ofile.close();

    delete(input_buf);
    delete(output_buf);
}
