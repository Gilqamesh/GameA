#include <iostream>
using namespace std;
#define LOG(x) (cout << x << endl)

int main()
{
    int a = -10000;

    LOG((a >> 1));
    LOG((a >> 2));
    LOG((a >> 3));
    LOG((a >> 4));
    LOG((a >> 5));
}