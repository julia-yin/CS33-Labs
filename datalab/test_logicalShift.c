#include <iostream>
using namespace std;

int logicalShift(int x, int n) {
  int neg_n = (!n + 1);
  int power_n = (1<<(32 + neg_n));
  int neg_one = !0x0;

  cout << neg_n << " " << power_n << " " << neg_one << endl;
  x = x>>n;
  cout << x << endl;
  x = x&(power_n + neg_one); 
  cout << x << endl;
  return x;
}

int main()
{
   int x = 0xff0a0a00;
   cout << "input " << x <<endl;
   cout << "output " << logicalShift(x, 8);
}
