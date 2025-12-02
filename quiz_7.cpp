#include <vector> 
#include <iostream>
using namespace std;

int main() { 
   unsigned int i;
   vector Xvector; 
   Xvector.push_back(1); 
   Xvector.push_back(2); 
   Xvector.clear();  
   for (i = 0; i < Xvector.size(); ++i)
      cout << ' ' << Xvector.at(i);
   return 0; 
}
