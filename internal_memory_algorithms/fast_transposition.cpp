#include <iostream>
#include <vector>

using namespace std;

vector<int> generate_input(int n, int seed) {
    vector<int> d(n * n);
    for (size_t i = 0; i < d.size(); ++i) {
        d[i] = seed;
        seed = ((long long) seed * 197 + 2017) & 987654;
    }
    return d;
}

long long get_hash(const vector<int> &d) {
    const long long MOD = 987654321054321LL;
    const long long MUL = 179;

    long long result_value = 0;
    for (size_t i = 0; i < d.size(); ++i)
        result_value = (result_value * MUL + d[i]) & MOD;
    return result_value;
}

void transpose(vector<int> &m, int imin, int jmin, int side, int n) {
    const int BLOCK = 8;
    int tmp;
    for (int i = 0; i < side; i += BLOCK) {
        for (int j = i; j < side; j += BLOCK) {
            for (int ib = i; ib < i + BLOCK && ib < side; ++ib) {
                for (int jb = (j <= ib) ? ib + 1 : j; jb < j + BLOCK && jb < side; ++jb) {
                    /*tmp = m[(imin + ib) * n + (jmin + jb)];
                    m[(imin + ib) * n + (jmin + jb)] = m[(imin + jb) * n + (jmin + ib)];
                    m[(imin + jb) * n + (jmin + ib)] = tmp;*/
                    m[(imin + ib) * n + (jmin + jb)] ^= m[(imin + jb) * n + (jmin + ib)];
                    m[(imin + jb) * n + (jmin + ib)] ^= m[(imin + ib) * n + (jmin + jb)];
                    m[(imin + ib) * n + (jmin + jb)] ^= m[(imin + jb) * n + (jmin + ib)];
                }
            }
        }
    }
}

void print(vector<int> &m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cout << m[i * n + j] << "\t";
        }
        cout << endl;
    }
    cout << endl;
}

int main() {
    int n, k, imin, jmin, side;
    int seed;
    cin >> n;
    cin >> seed;
    cin >> k;

    vector<int> m = generate_input(n, seed);

    for (int i = 0; i < k; ++i) {
        cin >> imin;
        cin >> jmin;
        cin >> side;
        transpose(m, imin, jmin, side, n);
    }

    cout << get_hash(m);
    return 0;
}