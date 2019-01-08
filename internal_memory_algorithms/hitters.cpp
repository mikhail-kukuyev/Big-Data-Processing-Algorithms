#include <iostream>
#include <vector>
#include <map>

using namespace std;

int main() {
    int k;
    cin >> k;
    k--;

    map<int, int> hitters;
    map<int, int>::iterator it;

    int x = 1;
    while (x != -1) {
        scanf("%d", &x);
        if (x == 0) {
            for (auto el : hitters) {
                printf("%d ", el.first);
            }

            int value = hitters.begin()->first;
            for(int i = k - (int)hitters.size(); i > 0; --i) {
                printf("%d ", value);
            }
            printf("\n");
        }
        else {
            if ((it = hitters.find(x)) != hitters.end()) {
                it->second++;
            }
            else if (hitters.size() < k) {
                hitters[x] = 1;
            }
            else {
                bool saved = false;
                int saved_key;
                int saved_value;
                for (it = hitters.begin(); it != hitters.end();) {
                    it->second--;

                    if (!saved && it->second >= 0) {
                        saved = true;
                        saved_key = it->first;
                        saved_value = it->second;
                    }

                    if (it->second <= 0)
                        it = hitters.erase(it);
                    else
                        it++;
                }

                if (hitters.empty())
                    hitters[saved_key] = saved_value;
            }
        }
    }
}
